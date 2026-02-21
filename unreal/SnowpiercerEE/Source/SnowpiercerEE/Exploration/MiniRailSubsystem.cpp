#include "MiniRailSubsystem.h"

void UMiniRailSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMiniRailSubsystem::Deinitialize()
{
	Segments.Empty();
	Carts.Empty();
	DisabledSecurity.Empty();
	Super::Deinitialize();
}

// --- Track Management ---

void UMiniRailSubsystem::RegisterSegment(const FTransportDeckSegment& Segment)
{
	Segments.Add(Segment.CarIndex, Segment);
}

FTransportDeckSegment UMiniRailSubsystem::GetSegment(int32 CarIndex) const
{
	if (const FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		return *Seg;
	}
	return FTransportDeckSegment();
}

ERailJunction UMiniRailSubsystem::GetJunctionAt(int32 CarIndex) const
{
	if (const FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		return Seg->JunctionType;
	}
	return ERailJunction::Straight;
}

void UMiniRailSubsystem::SetJunction(int32 CarIndex, ERailJunction Direction)
{
	if (FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		Seg->JunctionType = Direction;
	}
}

// --- Cart Management ---

FName UMiniRailSubsystem::SpawnCart(int32 CarIndex)
{
	FName CartID = GenerateCartID();

	FMiniRailCartData Cart;
	Cart.CartID = CartID;
	Cart.CurrentCarIndex = CarIndex;
	Cart.State = ECartState::Stationary;
	Cart.Speed = 0.0f;

	Carts.Add(CartID, Cart);
	return CartID;
}

TArray<FMiniRailCartData> UMiniRailSubsystem::GetAllCarts() const
{
	TArray<FMiniRailCartData> Result;
	for (const auto& Pair : Carts)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

FMiniRailCartData UMiniRailSubsystem::GetCart(FName CartID) const
{
	if (const FMiniRailCartData* Cart = Carts.Find(CartID))
	{
		return *Cart;
	}
	return FMiniRailCartData();
}

void UMiniRailSubsystem::StartCart(FName CartID, float Speed)
{
	if (FMiniRailCartData* Cart = Carts.Find(CartID))
	{
		Cart->Speed = FMath::Clamp(Speed, -Cart->MaxSpeed, Cart->MaxSpeed);
		Cart->State = ECartState::Moving;
	}
}

void UMiniRailSubsystem::StopCart(FName CartID)
{
	if (FMiniRailCartData* Cart = Carts.Find(CartID))
	{
		Cart->Speed = 0.0f;
		Cart->State = ECartState::Stationary;
	}
}

bool UMiniRailSubsystem::LoadCargo(FName CartID, const FCargoManifest& Cargo)
{
	FMiniRailCartData* Cart = Carts.Find(CartID);
	if (!Cart || Cart->State == ECartState::Moving)
	{
		return false;
	}

	Cart->LoadedCargo.Add(Cargo);
	Cart->State = ECartState::Loading;
	return true;
}

bool UMiniRailSubsystem::UnloadCargo(FName CartID, FName CargoID)
{
	FMiniRailCartData* Cart = Carts.Find(CartID);
	if (!Cart)
	{
		return false;
	}

	for (int32 i = Cart->LoadedCargo.Num() - 1; i >= 0; --i)
	{
		if (Cart->LoadedCargo[i].CargoID == CargoID)
		{
			Cart->LoadedCargo.RemoveAt(i);
			return true;
		}
	}
	return false;
}

TArray<FName> UMiniRailSubsystem::GetCartsNearPosition(FVector WorldPosition, float Radius) const
{
	TArray<FName> NearCarts;
	// Cart world positions are computed from CarIndex * car span.
	// This is a simplified distance check; full implementation uses
	// actor overlaps or spatial queries.
	float RadiusCm = Radius * 100.0f;
	for (const auto& Pair : Carts)
	{
		float CartX = Pair.Value.CurrentCarIndex * 13000.0f; // car span in cm
		float Dist = FMath::Abs(WorldPosition.X - CartX);
		if (Dist <= RadiusCm)
		{
			NearCarts.Add(Pair.Key);
		}
	}
	return NearCarts;
}

// --- Security ---

TArray<EDeckSecurityType> UMiniRailSubsystem::GetSecurityAt(int32 CarIndex) const
{
	// Return segment's security minus any disabled devices
	if (const FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		TArray<EDeckSecurityType> Active = Seg->SecurityDevices;
		if (const TArray<EDeckSecurityType>* Disabled = DisabledSecurity.Find(CarIndex))
		{
			for (EDeckSecurityType D : *Disabled)
			{
				Active.Remove(D);
			}
		}
		return Active;
	}
	return TArray<EDeckSecurityType>();
}

void UMiniRailSubsystem::DisableSecurity(int32 CarIndex, EDeckSecurityType Device)
{
	DisabledSecurity.FindOrAdd(CarIndex).AddUnique(Device);
}

void UMiniRailSubsystem::ResetSecurity(int32 CarIndex)
{
	DisabledSecurity.Remove(CarIndex);
}

// --- Queries ---

ESEEFaction UMiniRailSubsystem::GetControllingFaction(int32 CarIndex) const
{
	if (const FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		return Seg->ControllingFaction;
	}
	return ESEEFaction::Neutral;
}

bool UMiniRailSubsystem::IsSmuggleRoute(int32 CarIndex) const
{
	if (const FTransportDeckSegment* Seg = Segments.Find(CarIndex))
	{
		return Seg->bKronoleSmuggling;
	}
	return false;
}

// --- Internal ---

void UMiniRailSubsystem::UpdateCarts(float DeltaTime)
{
	for (auto& Pair : Carts)
	{
		if (Pair.Value.State == ECartState::Moving)
		{
			AdvanceCart(Pair.Value, DeltaTime);
		}
	}
}

void UMiniRailSubsystem::AdvanceCart(FMiniRailCartData& Cart, float DeltaTime)
{
	// Move cart along track. Car index changes when crossing segment boundaries.
	// Speed is in m/s, convert to car-index delta based on car span (130m at 10x).
	float DistanceM = Cart.Speed * DeltaTime;
	float CarSpanM = 130.0f; // 120m car + 10m gap

	// Simplified: accumulate fractional car index progress
	float CarDelta = DistanceM / CarSpanM;
	int32 PrevCar = Cart.CurrentCarIndex;
	int32 NewCar = PrevCar + FMath::FloorToInt32(CarDelta);

	// Check for terminus
	if (const FTransportDeckSegment* Seg = Segments.Find(NewCar))
	{
		if (Seg->JunctionType == ERailJunction::Terminus)
		{
			Cart.Speed = 0.0f;
			Cart.State = ECartState::Stationary;
			OnCartArrived.Broadcast(Cart.CartID, NewCar);
			return;
		}
	}

	// Check bounds (deck range: cars 15-82)
	NewCar = FMath::Clamp(NewCar, 15, 82);
	if (NewCar != PrevCar)
	{
		Cart.CurrentCarIndex = NewCar;
		OnCartArrived.Broadcast(Cart.CartID, NewCar);
	}
}

FName UMiniRailSubsystem::GenerateCartID()
{
	FString ID = FString::Printf(TEXT("Cart_%03d"), NextCartIndex++);
	return FName(*ID);
}
