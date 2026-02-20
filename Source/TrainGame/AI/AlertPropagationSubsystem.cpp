// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "AlertPropagationSubsystem.h"

void UAlertPropagationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize all zones to Green
	ZoneAlerts.Add(ETrainZone::Tail, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::ThirdClass, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::SecondClass, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::FirstClass, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::EngineSection, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::Security, FZoneAlertState());
	ZoneAlerts.Add(ETrainZone::Medical, FZoneAlertState());
}

void UAlertPropagationSubsystem::Deinitialize()
{
	ZoneAlerts.Empty();
	Super::Deinitialize();
}

EAlertLevel UAlertPropagationSubsystem::GetZoneAlertLevel(ETrainZone Zone) const
{
	if (const FZoneAlertState* State = ZoneAlerts.Find(Zone))
	{
		return State->Level;
	}
	return EAlertLevel::Green;
}

void UAlertPropagationSubsystem::EscalateZoneAlert(ETrainZone Zone, EAlertLevel Level, FVector ThreatLocation)
{
	FZoneAlertState& State = ZoneAlerts.FindOrAdd(Zone);

	if (Level <= State.Level) return; // Only escalate

	EAlertLevel OldLevel = State.Level;
	State.Level = Level;
	State.ThreatLocation = ThreatLocation;
	State.DeescalationTimer = -1.f; // Cancel any de-escalation

	OnZoneAlertChanged.Broadcast(Zone, Level);

	// Auto-propagate Red alerts to adjacent zones
	if (Level == EAlertLevel::Red)
	{
		PropagateToAdjacentZones(Zone);
	}
}

void UAlertPropagationSubsystem::SetZoneAlertLevel(ETrainZone Zone, EAlertLevel Level)
{
	FZoneAlertState& State = ZoneAlerts.FindOrAdd(Zone);

	if (Level == State.Level) return;

	State.Level = Level;
	if (Level == EAlertLevel::Green)
	{
		State.DeescalationTimer = -1.f;
	}

	OnZoneAlertChanged.Broadcast(Zone, Level);
}

void UAlertPropagationSubsystem::ResetAllAlerts()
{
	for (auto& Pair : ZoneAlerts)
	{
		if (Pair.Value.Level != EAlertLevel::Green)
		{
			Pair.Value.Level = EAlertLevel::Green;
			Pair.Value.DeescalationTimer = -1.f;
			OnZoneAlertChanged.Broadcast(Pair.Key, EAlertLevel::Green);
		}
	}
}

FVector UAlertPropagationSubsystem::GetZoneThreatLocation(ETrainZone Zone) const
{
	if (const FZoneAlertState* State = ZoneAlerts.Find(Zone))
	{
		return State->ThreatLocation;
	}
	return FVector::ZeroVector;
}

void UAlertPropagationSubsystem::PropagateToAdjacentZones(ETrainZone SourceZone)
{
	const FZoneAlertState* SourceState = ZoneAlerts.Find(SourceZone);
	if (!SourceState || SourceState->Level == EAlertLevel::Green) return;

	EAlertLevel ReducedLevel = ReduceAlertLevel(SourceState->Level);
	if (ReducedLevel == EAlertLevel::Green) return;

	TArray<ETrainZone> Adjacent = GetAdjacentZones(SourceZone);
	for (ETrainZone AdjacentZone : Adjacent)
	{
		EscalateZoneAlert(AdjacentZone, ReducedLevel, SourceState->ThreatLocation);
	}
}

void UAlertPropagationSubsystem::BeginDeescalation(ETrainZone Zone)
{
	if (FZoneAlertState* State = ZoneAlerts.Find(Zone))
	{
		State->DeescalationTimer = DeescalationTime;
	}
}

TArray<ETrainZone> UAlertPropagationSubsystem::GetAdjacentZones(ETrainZone Zone) const
{
	TArray<ETrainZone> Adjacent;

	// Train car adjacency — linear layout with some branches
	switch (Zone)
	{
	case ETrainZone::Tail:
		Adjacent.Add(ETrainZone::ThirdClass);
		break;
	case ETrainZone::ThirdClass:
		Adjacent.Add(ETrainZone::Tail);
		Adjacent.Add(ETrainZone::SecondClass);
		break;
	case ETrainZone::SecondClass:
		Adjacent.Add(ETrainZone::ThirdClass);
		Adjacent.Add(ETrainZone::FirstClass);
		Adjacent.Add(ETrainZone::Medical);
		break;
	case ETrainZone::FirstClass:
		Adjacent.Add(ETrainZone::SecondClass);
		Adjacent.Add(ETrainZone::EngineSection);
		break;
	case ETrainZone::EngineSection:
		Adjacent.Add(ETrainZone::FirstClass);
		break;
	case ETrainZone::Security:
		// Security has posts throughout, adjacent to most zones
		Adjacent.Add(ETrainZone::ThirdClass);
		Adjacent.Add(ETrainZone::SecondClass);
		Adjacent.Add(ETrainZone::FirstClass);
		break;
	case ETrainZone::Medical:
		Adjacent.Add(ETrainZone::SecondClass);
		break;
	}

	return Adjacent;
}

EAlertLevel UAlertPropagationSubsystem::ReduceAlertLevel(EAlertLevel Level) const
{
	switch (Level)
	{
	case EAlertLevel::Red:		return EAlertLevel::Orange;
	case EAlertLevel::Orange:	return EAlertLevel::Yellow;
	case EAlertLevel::Yellow:	return EAlertLevel::Green;
	default:					return EAlertLevel::Green;
	}
}
