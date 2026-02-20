// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "TransportDeckSubsystem.h"

void UTransportDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UTransportDeckSubsystem::Deinitialize()
{
	TrackNetwork.Empty();
	RegisteredCarts.Empty();
	CargoManifests.Empty();
	AccessPoints.Empty();
	JunctionStates.Empty();
	CurrentAlertLevel = EDeckAlertLevel::Clear;
	bLockdownActive = false;
	Super::Deinitialize();
}

// --- Track Network ---

void UTransportDeckSubsystem::RegisterTrackSegment(const FTrackSegment& Segment)
{
	if (Segment.SegmentID.IsNone())
	{
		return;
	}

	TrackNetwork.Add(Segment.SegmentID, Segment);

	// Auto-register junctions with default branch
	if (Segment.SegmentType == ETrackSegmentType::Junction && Segment.ConnectedSegments.Num() > 0)
	{
		if (!JunctionStates.Contains(Segment.SegmentID))
		{
			JunctionStates.Add(Segment.SegmentID, Segment.ConnectedSegments[0]);
		}
	}
}

bool UTransportDeckSubsystem::GetTrackSegment(FName SegmentID, FTrackSegment& OutSegment) const
{
	if (const FTrackSegment* Found = TrackNetwork.Find(SegmentID))
	{
		OutSegment = *Found;
		return true;
	}
	return false;
}

bool UTransportDeckSubsystem::SwitchJunction(FName JunctionID, FName TargetSegmentID)
{
	FTrackSegment* Junction = TrackNetwork.Find(JunctionID);
	if (!Junction || Junction->SegmentType != ETrackSegmentType::Junction)
	{
		return false;
	}

	if (!Junction->ConnectedSegments.Contains(TargetSegmentID))
	{
		return false;
	}

	JunctionStates.Add(JunctionID, TargetSegmentID);
	OnTrackSwitched.Broadcast(JunctionID);
	return true;
}

TArray<FTrackSegment> UTransportDeckSubsystem::GetSegmentsForCar(int32 CarIndex) const
{
	TArray<FTrackSegment> Result;
	for (const auto& Pair : TrackNetwork)
	{
		if (Pair.Value.CarIndex == CarIndex)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// --- Cart Management ---

void UTransportDeckSubsystem::RegisterCart(FName CartID, AActor* CartActor)
{
	if (!CartID.IsNone() && CartActor)
	{
		RegisteredCarts.Add(CartID, CartActor);
	}
}

void UTransportDeckSubsystem::UnregisterCart(FName CartID)
{
	RegisteredCarts.Remove(CartID);
}

bool UTransportDeckSubsystem::DispatchCart(FName CartID, const TArray<FName>& Route)
{
	if (bLockdownActive)
	{
		return false;
	}

	AActor** CartActor = RegisteredCarts.Find(CartID);
	if (!CartActor || !*CartActor)
	{
		return false;
	}

	// Validate route segments exist
	for (const FName& SegmentID : Route)
	{
		if (!TrackNetwork.Contains(SegmentID))
		{
			return false;
		}
	}

	// Route is valid — cart movement is driven by MiniRailCart actor tick
	return true;
}

TArray<FName> UTransportDeckSubsystem::GetAllCartIDs() const
{
	TArray<FName> IDs;
	RegisteredCarts.GetKeys(IDs);
	return IDs;
}

// --- Cargo ---

void UTransportDeckSubsystem::RegisterCargo(const FCargoManifest& Manifest)
{
	CargoManifests.Add(Manifest);
}

bool UTransportDeckSubsystem::RemoveCargo(FName CargoID)
{
	for (int32 i = CargoManifests.Num() - 1; i >= 0; --i)
	{
		if (CargoManifests[i].CargoID == CargoID)
		{
			ECargoType Type = CargoManifests[i].CargoType;
			CargoManifests.RemoveAt(i);
			OnCargoLooted.Broadcast(CargoID, Type);
			return true;
		}
	}
	return false;
}

TArray<FCargoManifest> UTransportDeckSubsystem::GetCargoAtCar(int32 CarIndex) const
{
	TArray<FCargoManifest> Result;
	for (const FCargoManifest& Manifest : CargoManifests)
	{
		if (Manifest.SourceCar == CarIndex || Manifest.DestinationCar == CarIndex)
		{
			Result.Add(Manifest);
		}
	}
	return Result;
}

TArray<FCargoManifest> UTransportDeckSubsystem::GetContrabandCargo() const
{
	TArray<FCargoManifest> Result;
	for (const FCargoManifest& Manifest : CargoManifests)
	{
		if (Manifest.bIsContraband)
		{
			Result.Add(Manifest);
		}
	}
	return Result;
}

// --- Access Points ---

void UTransportDeckSubsystem::RegisterAccessPoint(const FDeckAccessConfig& Config)
{
	AccessPoints.Add(Config);
}

TArray<FDeckAccessConfig> UTransportDeckSubsystem::GetAccessPointsForCar(int32 CarIndex) const
{
	TArray<FDeckAccessConfig> Result;
	for (const FDeckAccessConfig& AP : AccessPoints)
	{
		if (AP.CarIndex == CarIndex)
		{
			Result.Add(AP);
		}
	}
	return Result;
}

bool UTransportDeckSubsystem::HasUnlockedAccess(int32 CarIndex) const
{
	for (const FDeckAccessConfig& AP : AccessPoints)
	{
		if (AP.CarIndex == CarIndex && !AP.bLocked)
		{
			return true;
		}
	}
	return false;
}

// --- Security ---

void UTransportDeckSubsystem::RaiseAlert(EDeckAlertLevel Level)
{
	if (Level > CurrentAlertLevel)
	{
		CurrentAlertLevel = Level;
		OnAlertChanged.Broadcast(CurrentAlertLevel);
	}
}

void UTransportDeckSubsystem::ReduceAlert()
{
	if (CurrentAlertLevel == EDeckAlertLevel::Clear)
	{
		return;
	}

	CurrentAlertLevel = static_cast<EDeckAlertLevel>(
		FMath::Max(0, static_cast<int32>(CurrentAlertLevel) - 1)
	);
	OnAlertChanged.Broadcast(CurrentAlertLevel);
}

void UTransportDeckSubsystem::TriggerLockdown()
{
	bLockdownActive = true;
	CurrentAlertLevel = EDeckAlertLevel::Lockdown;
	OnAlertChanged.Broadcast(CurrentAlertLevel);
}

void UTransportDeckSubsystem::ClearLockdown()
{
	bLockdownActive = false;
	CurrentAlertLevel = EDeckAlertLevel::Clear;
	OnAlertChanged.Broadcast(CurrentAlertLevel);
}

// --- Queries ---

ESEETrainZone UTransportDeckSubsystem::GetZoneForCar(int32 CarIndex)
{
	if (CarIndex <= 15) return ESEETrainZone::Tail;
	if (CarIndex <= 35) return ESEETrainZone::ThirdClass;
	if (CarIndex <= 55) return ESEETrainZone::SecondClass;
	if (CarIndex <= 70) return ESEETrainZone::WorkingSpine;
	if (CarIndex <= 90) return ESEETrainZone::FirstClass;
	if (CarIndex <= 98) return ESEETrainZone::Sanctum;
	return ESEETrainZone::Engine;
}
