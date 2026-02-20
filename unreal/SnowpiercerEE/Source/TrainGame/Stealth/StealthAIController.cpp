// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "StealthAIController.h"
#include "TrainGame/Stealth/DetectionComponent.h"

AStealthAIController::AStealthAIController()
{
}

void AStealthAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	DetectionComp = InPawn->FindComponentByClass<UDetectionComponent>();
	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.AddDynamic(this, &AStealthAIController::OnDetectionStateChanged);
	}
}

void AStealthAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!DetectionComp)
	{
		return;
	}

	switch (DetectionComp->GetDetectionState())
	{
	case EDetectionState::Unaware:
		TickPatrol(DeltaTime);
		break;
	case EDetectionState::Suspicious:
		TickSuspicious(DeltaTime);
		break;
	case EDetectionState::Alerted:
		TickAlerted(DeltaTime);
		break;
	case EDetectionState::Combat:
		TickCombat(DeltaTime);
		break;
	}
}

void AStealthAIController::OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState)
{
	// TODO: Adjust movement speed based on new state
	// TODO: Draw weapon on ALERTED+
	// TODO: Trigger vocal alert on ALERTED+
	// TODO: Hand off to CombatAI on COMBAT

	if (NewState == EDetectionState::Combat)
	{
		HandOffToCombatAI();
	}
}

void AStealthAIController::TickPatrol(float DeltaTime)
{
	switch (PatrolBehavior)
	{
	case EPatrolBehavior::RoutePatrol:
	{
		// TODO: Move along waypoints, pause at each one
		// TODO: Look around during pause (rotate head)
		if (PatrolWaypoints.Num() == 0)
		{
			return;
		}

		// TODO: Move toward PatrolWaypoints[CurrentWaypointIndex]
		// TODO: On arrival, pause for WaypointPauseDuration then advance index
		break;
	}
	case EPatrolBehavior::Stationary:
		// TODO: Idle animation, periodic look-around
		break;
	case EPatrolBehavior::Wander:
		// TODO: Pick random point within patrol area, walk there, pause, repeat
		break;
	case EPatrolBehavior::Scheduled:
		// TODO: Check current time, move to scheduled location
		break;
	}
}

void AStealthAIController::TickSuspicious(float DeltaTime)
{
	// TODO: Walk toward last known stimulus location
	// TODO: Look around at investigation point
	// TODO: If player is visible and disguised, consider initiating challenge

	if (!bIsInvestigating)
	{
		MoveToInvestigationPoint();
	}

	// TODO: Check if player is nearby and disguised → InitiateChallenge
}

void AStealthAIController::TickAlerted(float DeltaTime)
{
	// TODO: Move to last known player position at AlertSpeed
	// TODO: Systematically check hiding spots
	// TODO: Call for backup (other NPCs)

	SearchNearbyHidingSpots();
}

void AStealthAIController::TickCombat(float DeltaTime)
{
	// Combat is handled by CombatAIController after handoff
	// This tick should be minimal — just verify handoff happened
}

void AStealthAIController::MoveToInvestigationPoint()
{
	bIsInvestigating = true;
	InvestigationTarget = DetectionComp->GetLastKnownTargetLocation();

	// TODO: Use navigation to move to InvestigationTarget
	// TODO: On arrival, look around for InvestigationCheckpoints spots
}

void AStealthAIController::SearchNearbyHidingSpots()
{
	// TODO: Find hiding spots within InvestigationRadius
	// TODO: Move to each one and "search" it (takes HidingSpotComponent::SearchTime)
	// TODO: If player is found, transition to COMBAT
}

void AStealthAIController::ReturnToPatrol()
{
	bIsInvestigating = false;
	HidingSpotsChecked = 0;
	// TODO: Navigate back to nearest patrol waypoint or last patrol position
}

void AStealthAIController::InitiateChallenge(AActor* SuspectedPlayer)
{
	if (!bCanChallenge || !SuspectedPlayer)
	{
		return;
	}

	// TODO: Stop and face the player
	// TODO: Trigger challenge dialogue via DisguiseComponent::ResolveChallenge
	// TODO: On failure, escalate to ALERTED
	// TODO: On success, return to current behavior with grace period
}

void AStealthAIController::HandOffToCombatAI()
{
	// TODO: Swap this AI controller for CombatAIController on the pawn
	// TODO: Pass target information to the combat AI
}

TArray<AActor*> AStealthAIController::FindNearbyHidingSpots() const
{
	// TODO: Sphere overlap query for actors with UHidingSpotComponent
	// TODO: Filter to spots within InvestigationRadius
	// TODO: Sort by distance
	return TArray<AActor*>();
}
