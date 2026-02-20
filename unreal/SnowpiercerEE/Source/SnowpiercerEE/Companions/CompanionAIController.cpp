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
	return nullptr;
}

FVector ACompanionAIController::GetDesiredCombatPosition() const
{
	if (!GetPawn())
	{
		return FVector::ZeroVector;
	}

	return GetPawn()->GetActorLocation();
}

bool ACompanionAIController::ShouldUseUniqueAbility() const
{
	if (!CompanionComp || !CompanionComp->IsUniqueAbilityAvailable())
	{
		return false;
	}

	return false;
}

// --- Formation ---

void ACompanionAIController::MoveToFormationPosition()
{
	// Stub: move to position based on formation setting and corridor width
}

void ACompanionAIController::HandleDoorwayBehavior(FVector DoorLocation)
{
	// Stub: stack at door, wait for player to advance DoorwayAdvanceDistance
}

// --- Private Behavior Updates ---

void ACompanionAIController::UpdateAggressiveBehavior(float DeltaTime)
{
	// Rush enemies, prioritize damage, push to front of formation
}

void ACompanionAIController::UpdateDefensiveBehavior(float DeltaTime)
{
	// Stay near player, prioritize blocking and counters
}

void ACompanionAIController::UpdatePassiveBehavior(float DeltaTime)
{
	// Stay behind player, don't initiate combat
}

void ACompanionAIController::UpdateHoldPositionBehavior(float DeltaTime)
{
	// Stay at HoldPositionLocation, fight anything in melee range
}
