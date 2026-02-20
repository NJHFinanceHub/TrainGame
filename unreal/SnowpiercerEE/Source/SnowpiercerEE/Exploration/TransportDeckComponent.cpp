#include "TransportDeckComponent.h"
#include "EngineUtils.h"

UTransportDeckComponent::UTransportDeckComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTransportDeckComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bOnTransportDeck)
	{
		UpdateCarTracking();
		UpdateAlertDecay(DeltaTime);
	}
}

// --- Access ---

void UTransportDeckComponent::EnterTransportDeck(int32 CarIndex, EDeckAccessType AccessType)
{
	if (!HasTransportDeck(CarIndex))
	{
		return;
	}

	bOnTransportDeck = true;
	CurrentCarIndex = CarIndex;
	OnDeckEntered.Broadcast(CarIndex);
}

void UTransportDeckComponent::ExitTransportDeck()
{
	if (bRidingCart)
	{
		DisembarkCart();
	}

	bOnTransportDeck = false;
	OnDeckExited.Broadcast();
}

bool UTransportDeckComponent::HasTransportDeck(int32 CarIndex) const
{
	return CarIndex >= DeckStartCar && CarIndex <= DeckEndCar;
}

bool UTransportDeckComponent::HasAccessPoint(int32 CarIndex, EDeckAccessType AccessType) const
{
	// Access points are data-driven via FTransportDeckSegment DataTable rows.
	// This stub returns true for valid deck range; full implementation queries
	// the DataTable at runtime.
	return HasTransportDeck(CarIndex);
}

// --- Mini-Rail ---

bool UTransportDeckComponent::BoardCart(FName CartID)
{
	if (bRidingCart || !bOnTransportDeck)
	{
		return false;
	}

	bRidingCart = true;
	CurrentCartID = CartID;
	OnCartBoarded.Broadcast(CartID, CurrentCarIndex);
	return true;
}

void UTransportDeckComponent::DisembarkCart()
{
	if (!bRidingCart)
	{
		return;
	}

	FName PrevCartID = CurrentCartID;
	bRidingCart = false;
	CurrentCartID = NAME_None;
	OnCartExited.Broadcast(PrevCartID);
}

void UTransportDeckComponent::SetCartSpeed(float SpeedMs)
{
	if (!bRidingCart)
	{
		return;
	}

	// Cart speed is clamped to MaxSpeed; negative reverses direction.
	// Actual movement is handled by MiniRailSubsystem.
}

bool UTransportDeckComponent::SwitchJunction(ERailJunction NewDirection)
{
	if (!bRidingCart && !bOnTransportDeck)
	{
		return false;
	}

	// Junction switching generates noise
	GenerateNoise(BaseNoiseRadius * 1.5f);
	return true;
}

FMiniRailCart UTransportDeckComponent::GetCurrentCart() const
{
	FMiniRailCart Cart;
	Cart.CartID = CurrentCartID;
	Cart.CurrentCarIndex = CurrentCarIndex;
	Cart.bPlayerRiding = bRidingCart;
	return Cart;
}

bool UTransportDeckComponent::IsNearCart() const
{
	// Proximity check delegated to MiniRailSubsystem overlap queries
	return false;
}

// --- Cargo ---

TArray<FCargoManifest> UTransportDeckComponent::InspectCargo() const
{
	// Returns cargo manifest for current segment/cart.
	// Full implementation queries MiniRailSubsystem for cart cargo
	// and FTransportDeckSegment for stationary cargo.
	TArray<FCargoManifest> Manifests;
	return Manifests;
}

bool UTransportDeckComponent::TakeContraband(FName CargoID, int32 CunningScore)
{
	// Cunning 6+ to successfully take without triggering alert
	if (CunningScore < 6)
	{
		TriggerSecurity(EDeckSecurityType::MotionSensor);
		return false;
	}

	OnCargoInteraction.Broadcast(CargoID, ECargoType::Contraband);
	return true;
}

bool UTransportDeckComponent::PlantContraband(FName CargoID, FName CartID)
{
	if (!bOnTransportDeck)
	{
		return false;
	}

	// Planting contraband on a cart for Kronole Network smuggling missions
	OnCargoInteraction.Broadcast(CargoID, ECargoType::Contraband);
	return true;
}

// --- Security ---

TArray<EDeckSecurityType> UTransportDeckComponent::GetActiveSecurityDevices() const
{
	// Returns active security devices in current segment.
	// Full implementation queries FTransportDeckSegment DataTable.
	TArray<EDeckSecurityType> Devices;
	return Devices;
}

bool UTransportDeckComponent::DisableSecurityDevice(EDeckSecurityType Device, int32 CunningScore)
{
	int32 RequiredSkill = 0;
	switch (Device)
	{
	case EDeckSecurityType::LaserTripwire: RequiredSkill = 4; break;
	case EDeckSecurityType::MotionSensor: RequiredSkill = 5; break;
	case EDeckSecurityType::PatrolDrone: RequiredSkill = 7; break;
	case EDeckSecurityType::PressurePlate: RequiredSkill = 3; break;
	case EDeckSecurityType::SecurityCamera: RequiredSkill = 6; break;
	default: return true;
	}

	if (CunningScore >= RequiredSkill)
	{
		return true;
	}

	// Failed — triggers alert
	TriggerSecurity(Device);
	return false;
}

float UTransportDeckComponent::GetCurrentNoiseRadius() const
{
	float Multiplier = 1.0f;
	if (bCrouching) Multiplier = 0.4f;
	else if (bSprinting) Multiplier = 2.5f;

	if (bRidingCart) Multiplier = 3.0f; // Carts are noisy

	return FMath::Max(0.0f, BaseNoiseRadius * Multiplier - MachineryMasking);
}

// --- Internal ---

void UTransportDeckComponent::UpdateCarTracking()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	float XPos = Owner->GetActorLocation().X;
	// Each car span at 10x scale = 12000cm (120m) + 1000cm gap = 13000cm
	int32 NewCarIndex = FMath::FloorToInt32(XPos / 13000.0f);
	NewCarIndex = FMath::Clamp(NewCarIndex, DeckStartCar, DeckEndCar);

	if (NewCarIndex != CurrentCarIndex)
	{
		CurrentCarIndex = NewCarIndex;
	}
}

void UTransportDeckComponent::UpdateAlertDecay(float DeltaTime)
{
	if (AlertLevel == EDeckAlertLevel::Clear)
	{
		return;
	}

	AlertDecayTimer += DeltaTime;
	if (AlertDecayTimer >= AlertDecayTime)
	{
		AlertDecayTimer = 0.0f;

		// Decay one step
		uint8 Current = static_cast<uint8>(AlertLevel);
		if (Current > 0)
		{
			SetAlertLevel(static_cast<EDeckAlertLevel>(Current - 1));
		}
	}
}

void UTransportDeckComponent::GenerateNoise(float Radius)
{
	if (Radius <= 0.0f) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Alert nearby NPCs within noise radius (same pattern as CrawlspaceComponent)
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* NPC = *It;
		if (NPC == Owner) continue;

		float Dist = FVector::Dist(Owner->GetActorLocation(), NPC->GetActorLocation());
		if (Dist <= Radius * 100.0f) // meters to cm
		{
			// NPC AI detection system handles the alert via hearing
		}
	}
}

void UTransportDeckComponent::TriggerSecurity(EDeckSecurityType Device)
{
	OnSecurityTriggered.Broadcast(Device);

	// Escalate alert level
	switch (Device)
	{
	case EDeckSecurityType::PressurePlate:
		if (AlertLevel < EDeckAlertLevel::Suspicious)
			SetAlertLevel(EDeckAlertLevel::Suspicious);
		break;
	case EDeckSecurityType::LaserTripwire:
	case EDeckSecurityType::MotionSensor:
	case EDeckSecurityType::SecurityCamera:
		if (AlertLevel < EDeckAlertLevel::Alert)
			SetAlertLevel(EDeckAlertLevel::Alert);
		break;
	case EDeckSecurityType::PatrolDrone:
		SetAlertLevel(EDeckAlertLevel::Lockdown);
		break;
	default:
		break;
	}
}

void UTransportDeckComponent::SetAlertLevel(EDeckAlertLevel NewLevel)
{
	if (AlertLevel != NewLevel)
	{
		AlertLevel = NewLevel;
		AlertDecayTimer = 0.0f;
		OnAlertLevelChanged.Broadcast(NewLevel);
	}
}
