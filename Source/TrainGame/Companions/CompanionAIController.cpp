// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CompanionAIController.h"
#include "CompanionComponent.h"

ACompanionAIController::ACompanionAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACompanionAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		CompanionComp = InPawn->FindComponentByClass<UCompanionComponent>();
	}
}

void ACompanionAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CompanionComp || !CompanionComp->IsAlive())
	{
		return;
	}

	// Handle formation swap animation
	if (bIsSwappingFormation)
	{
		SwapTimer -= DeltaTime;
		if (SwapTimer <= 0.0f)
		{
			bIsSwappingFormation = false;
		}
		return; // No combat actions during swap
	}

	// Route to behavior-specific update
	switch (CompanionComp->GetBehaviorMode())
	{
	case ECompanionBehavior::Aggressive:
		UpdateAggressiveBehavior(DeltaTime);
		break;
	case ECompanionBehavior::Defensive:
		UpdateDefensiveBehavior(DeltaTime);
		break;
	case ECompanionBehavior::Passive:
		UpdatePassiveBehavior(DeltaTime);
		break;
	case ECompanionBehavior::HoldPosition:
		UpdateHoldPositionBehavior(DeltaTime);
		break;
	}
}

// --- Behavior Mode ---

void ACompanionAIController::UpdateBehaviorMode(ECompanionBehavior NewBehavior)
{
	if (NewBehavior == ECompanionBehavior::HoldPosition && GetPawn())
	{
		HoldPositionLocation = GetPawn()->GetActorLocation();
	}
}

// --- Corridor Navigation ---

bool ACompanionAIController::IsInNarrowCorridor() const
{
	return GetCurrentCorridorWidth() < NarrowCorridorThreshold;
}

float ACompanionAIController::GetCurrentCorridorWidth() const
{
	// Stub: perform lateral traces to determine corridor width
	// In production, this queries the car's geometry data
	return 3.2f; // Default train corridor width
}

// --- Combat AI ---

AActor* ACompanionAIController::SelectCombatTarget() const
{
	// Stub: target selection based on behavior mode
	// Aggressive: nearest enemy, then lowest HP
	// Defensive: enemies targeting player first
	// Passive: only enemies directly attacking this companion
	// Hold: enemies within melee range of hold position
	return nullptr;
}

FVector ACompanionAIController::GetDesiredCombatPosition() const
{
	if (!GetPawn())
	{
		return FVector::ZeroVector;
	}

	// Stub: calculate position based on behavior and formation
	return GetPawn()->GetActorLocation();
}

bool ACompanionAIController::ShouldUseUniqueAbility() const
{
	if (!CompanionComp || !CompanionComp->IsUniqueAbilityAvailable())
	{
		return false;
	}

	// Stub: ability-specific logic determined by companion role
	// Each companion subclass overrides the evaluation criteria
	return false;
}

// --- Formation ---

void ACompanionAIController::MoveToFormationPosition()
{
	// Stub: move to position based on formation setting and corridor width
	// In narrow corridors, enforce single-file
	// In wide corridors, allow side-by-side positioning
}

void ACompanionAIController::HandleDoorwayBehavior(FVector DoorLocation)
{
	// Stub: stack at door, wait for player to advance DoorwayAdvanceDistance
	// into the new car, then follow through
}

// --- Private Behavior Updates ---

void ACompanionAIController::UpdateAggressiveBehavior(float DeltaTime)
{
	// Rush enemies, prioritize damage, push to front of formation
	// Burns stamina aggressively
}

void ACompanionAIController::UpdateDefensiveBehavior(float DeltaTime)
{
	// Stay near player, prioritize blocking and counters
	// Maintain stamina reserve for blocks
}

void ACompanionAIController::UpdatePassiveBehavior(float DeltaTime)
{
	// Stay behind player, don't initiate combat
	// Only retaliate when damage exceeds PassiveRetaliationThreshold
}

void ACompanionAIController::UpdateHoldPositionBehavior(float DeltaTime)
{
	// Stay at HoldPositionLocation, fight anything in melee range
}
