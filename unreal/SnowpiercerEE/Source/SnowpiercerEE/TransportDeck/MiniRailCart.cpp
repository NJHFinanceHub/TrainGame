// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "MiniRailCart.h"
#include "TransportDeckSubsystem.h"
#include "EngineUtils.h"

AMiniRailCart::AMiniRailCart()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMiniRailCart::BeginPlay()
{
	Super::BeginPlay();

	// Register with subsystem
	UWorld* World = GetWorld();
	if (World && !CartID.IsNone())
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub)
		{
			DeckSub->RegisterCart(CartID, this);
		}
	}
}

void AMiniRailCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CartState == EMiniRailCartState::Moving)
	{
		UpdateMovement(DeltaTime);
		UpdateCarTracking();
		GenerateMovementNoise();
	}

	NoiseCooldown = FMath::Max(0.0f, NoiseCooldown - DeltaTime);
}

void AMiniRailCart::StartMoving()
{
	if (!CanMove())
	{
		return;
	}

	CartState = EMiniRailCartState::Moving;
}

void AMiniRailCart::ApplyBrakes()
{
	if (CartState == EMiniRailCartState::Moving)
	{
		// Decelerate to stop
		CurrentSpeed = FMath::Max(0.0f, CurrentSpeed - BrakeDeceleration);
		if (CurrentSpeed <= 0.0f)
		{
			CartState = EMiniRailCartState::Idle;
			CurrentSpeed = 0.0f;
		}
	}
}

void AMiniRailCart::EmergencyStop()
{
	CurrentSpeed = 0.0f;
	CartState = EMiniRailCartState::Idle;
}

bool AMiniRailCart::LoadCargo(const FCargoManifest& Cargo)
{
	if (CurrentCargoWeight + Cargo.Weight > CargoCapacity)
	{
		return false;
	}

	LoadedCargo.Add(Cargo);
	CurrentCargoWeight += Cargo.Weight;
	return true;
}

bool AMiniRailCart::UnloadCargo(FName CargoID)
{
	for (int32 i = LoadedCargo.Num() - 1; i >= 0; --i)
	{
		if (LoadedCargo[i].CargoID == CargoID)
		{
			CurrentCargoWeight -= LoadedCargo[i].Weight;
			LoadedCargo.RemoveAt(i);
			return true;
		}
	}
	return false;
}

void AMiniRailCart::SetRoute(const TArray<FName>& NewRoute)
{
	Route = NewRoute;
	RouteIndex = 0;
	SegmentProgress = 0.0f;

	if (Route.Num() > 0)
	{
		CurrentSegmentID = Route[0];
	}
}

void AMiniRailCart::OnPlayerBoard()
{
	bPlayerOnBoard = true;
}

void AMiniRailCart::OnPlayerDismount()
{
	bPlayerOnBoard = false;
}

float AMiniRailCart::GetCurrentNoiseRadius() const
{
	if (CartState != EMiniRailCartState::Moving || CurrentSpeed <= 0.0f)
	{
		return IdleNoiseRadius;
	}

	float SpeedRatio = CurrentSpeed / MaxSpeed;
	return FMath::Lerp(IdleNoiseRadius, MaxSpeedNoiseRadius, SpeedRatio);
}

bool AMiniRailCart::CanMove() const
{
	if (CartState == EMiniRailCartState::Derailed || CartState == EMiniRailCartState::Locked)
	{
		return false;
	}

	if (Route.Num() == 0)
	{
		return false;
	}

	// Check lockdown
	UWorld* World = GetWorld();
	if (World)
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub && DeckSub->GetAlertLevel() == EDeckAlertLevel::Lockdown)
		{
			return false;
		}
	}

	return true;
}

void AMiniRailCart::UpdateMovement(float DeltaTime)
{
	if (Route.Num() == 0 || RouteIndex >= Route.Num())
	{
		CartState = EMiniRailCartState::Idle;
		CurrentSpeed = 0.0f;
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
	if (!DeckSub)
	{
		return;
	}

	FTrackSegment CurrentSeg;
	if (!DeckSub->GetTrackSegment(Route[RouteIndex], CurrentSeg))
	{
		CartState = EMiniRailCartState::Derailed;
		CurrentSpeed = 0.0f;
		return;
	}

	// Accelerate toward segment max speed (or cart max, whichever lower)
	float TargetSpeed = FMath::Min(MaxSpeed, CurrentSeg.MaxSpeed);

	// Heavier cargo = slower acceleration
	float WeightFactor = 1.0f - (CurrentCargoWeight / (CargoCapacity * 2.0f));
	WeightFactor = FMath::Clamp(WeightFactor, 0.3f, 1.0f);

	if (CurrentSpeed < TargetSpeed)
	{
		CurrentSpeed = FMath::Min(TargetSpeed, CurrentSpeed + Acceleration * WeightFactor * DeltaTime);
	}

	// Advance along segment
	float SegmentLength = FVector::Dist(CurrentSeg.StartPoint, CurrentSeg.EndPoint);
	if (SegmentLength > 0.0f)
	{
		// Speed is in m/s, segment length in cm — convert
		float SpeedCm = CurrentSpeed * 100.0f;
		SegmentProgress += (SpeedCm * DeltaTime) / SegmentLength;

		// Interpolate position
		FVector NewLoc = FMath::Lerp(CurrentSeg.StartPoint, CurrentSeg.EndPoint, SegmentProgress);
		SetActorLocation(NewLoc);
	}

	// Advance to next segment if done
	if (SegmentProgress >= 1.0f)
	{
		SegmentProgress = 0.0f;
		RouteIndex++;

		if (RouteIndex < Route.Num())
		{
			CurrentSegmentID = Route[RouteIndex];
		}
		else
		{
			// Reached end of route
			CartState = EMiniRailCartState::Idle;
			CurrentSpeed = 0.0f;
		}
	}
}

void AMiniRailCart::UpdateCarTracking()
{
	float XPos = GetActorLocation().X;
	int32 NewCarIndex = FMath::FloorToInt32(XPos / 10000.0f);
	NewCarIndex = FMath::Clamp(NewCarIndex, 15, 82);
	CurrentCarIndex = NewCarIndex;
}

void AMiniRailCart::GenerateMovementNoise()
{
	if (NoiseCooldown > 0.0f)
	{
		return;
	}

	float NoiseRadius = GetCurrentNoiseRadius();
	if (NoiseRadius <= 0.0f)
	{
		return;
	}

	// Emit noise events every 0.5 seconds while moving
	NoiseCooldown = 0.5f;

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* NPC = *It;
		float Dist = FVector::Dist(GetActorLocation(), NPC->GetActorLocation());
		if (Dist <= NoiseRadius * 100.0f)
		{
			// Detection system handles alert propagation
		}
	}
}
