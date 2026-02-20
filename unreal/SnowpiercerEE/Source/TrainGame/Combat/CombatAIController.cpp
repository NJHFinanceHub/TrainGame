// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatAIController.h"
#include "CombatComponent.h"
#include "TrainGame/Environment/EnvironmentalHazardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"

ACombatAIController::ACombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CombatComp = InPawn->FindComponentByClass<UCombatComponent>();
	ApplyProfileModifiers();
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CombatComp || !CombatComp->IsAlive()) return;
	if (CombatComp->IsStaggered()) return;

	DecisionTimer -= DeltaTime;
	if (DecisionTimer <= 0.f)
	{
		DecisionTimer = DecisionInterval;
		MakeDecision();
	}
}

void ACombatAIController::MakeDecision()
{
	// Find or update target
	CurrentTarget = FindTarget();
	if (!CurrentTarget) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

	// Corridor discipline: only MaxSimultaneousAttackers can engage at once
	int32 EngagingCount = GetEngagingAICount();
	bool bCanEngage = EngagingCount < MaxSimultaneousAttackers;

	// Decision tree based on profile and situation
	if (DistToTarget <= PreferredCombatRange && bCanEngage)
	{
		// In range and can engage
		// Check for environmental kill opportunity first
		if (FMath::FRand() < EnvironmentalUseChance)
		{
			ConsiderEnvironmentalKill();
			return;
		}

		PerformAttack();
	}
	else if (bCanEngage)
	{
		// Out of range, approach
		MoveToTarget();
	}
	else
	{
		// Too many attackers, hold position and wait for opening
		// Disciplined AI will try to find a flanking route
		if (Profile == ECombatAIProfile::Cunning || Profile == ECombatAIProfile::Disciplined)
		{
			// Try to position to the side or behind
			FVector PerpOffset = ControlledPawn->GetActorRightVector() * 100.f;
			FVector FlankPos = CurrentTarget->GetActorLocation() + PerpOffset;
			MoveToLocation(FlankPos);
		}
		else
		{
			StopMovement();
		}
	}
}

void ACombatAIController::MoveToTarget()
{
	if (!CurrentTarget) return;

	MoveToActor(CurrentTarget, PreferredCombatRange * 0.8f);
	bIsApproaching = true;
}

void ACombatAIController::PerformAttack()
{
	if (!CombatComp) return;

	// Choose attack direction based on profile
	EAttackDirection Direction;
	float Roll = FMath::FRand();

	switch (Profile)
	{
		case ECombatAIProfile::Desperate:
			// Wild swings, mostly mid
			Direction = (Roll < 0.6f) ? EAttackDirection::Mid :
						(Roll < 0.8f) ? EAttackDirection::High : EAttackDirection::Low;
			break;

		case ECombatAIProfile::Disciplined:
			// Varied attacks, harder to predict
			Direction = (Roll < 0.33f) ? EAttackDirection::High :
						(Roll < 0.66f) ? EAttackDirection::Mid : EAttackDirection::Low;
			break;

		case ECombatAIProfile::Cunning:
			// Favors low attacks (dirty fighting)
			Direction = (Roll < 0.2f) ? EAttackDirection::High :
						(Roll < 0.5f) ? EAttackDirection::Mid : EAttackDirection::Low;
			break;

		case ECombatAIProfile::Brute:
			// Heavy overhead strikes
			Direction = (Roll < 0.5f) ? EAttackDirection::High : EAttackDirection::Mid;
			break;

		default:
			Direction = EAttackDirection::Mid;
	}

	CombatComp->PerformAttack(Direction);
}

void ACombatAIController::ConsiderBlock()
{
	if (!CombatComp) return;

	if (FMath::FRand() < BlockChance)
	{
		// Block in a semi-random direction (AI doesn't perfectly predict)
		float Roll = FMath::FRand();
		EBlockDirection Dir = (Roll < 0.4f) ? EBlockDirection::Mid :
							 (Roll < 0.7f) ? EBlockDirection::High : EBlockDirection::Low;
		CombatComp->StartBlock(Dir);
	}
}

void ACombatAIController::ConsiderEnvironmentalKill()
{
	if (!CurrentTarget) return;

	AActor* NearbyHazard = FindNearbyHazard(CurrentTarget);
	if (!NearbyHazard) return;

	UEnvironmentalHazardComponent* HazardComp = NearbyHazard->FindComponentByClass<UEnvironmentalHazardComponent>();
	if (HazardComp && HazardComp->CanTrigger() && HazardComp->IsActorInHazardZone(CurrentTarget))
	{
		// Trigger the hazard on the target
		HazardComp->TriggerHazard(GetPawn());
	}
}

void ACombatAIController::ApplyProfileModifiers()
{
	switch (Profile)
	{
		case ECombatAIProfile::Desperate:
			BlockChance = 0.15f;
			DodgeChance = 0.2f;
			DecisionInterval = 0.8f; // Slower reactions
			EnvironmentalUseChance = 0.05f;
			break;

		case ECombatAIProfile::Disciplined:
			BlockChance = 0.5f;
			DodgeChance = 0.2f;
			DecisionInterval = 0.4f; // Fast reactions
			EnvironmentalUseChance = 0.1f;
			MaxSimultaneousAttackers = 3; // Better coordination
			break;

		case ECombatAIProfile::Cunning:
			BlockChance = 0.3f;
			DodgeChance = 0.35f;
			DecisionInterval = 0.5f;
			EnvironmentalUseChance = 0.4f; // Very likely to use environment
			break;

		case ECombatAIProfile::Brute:
			BlockChance = 0.1f;
			DodgeChance = 0.05f;
			DecisionInterval = 1.0f; // Slow but powerful
			EnvironmentalUseChance = 0.02f;
			break;
	}
}

AActor* ACombatAIController::FindTarget() const
{
	// For prototype: find the player
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

AActor* ACombatAIController::FindNearbyHazard(AActor* NearActor) const
{
	if (!NearActor) return nullptr;

	// Search for hazard components in nearby actors
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	float ClosestDist = 300.f; // Search radius for nearby hazards
	AActor* ClosestHazard = nullptr;

	for (AActor* Actor : AllActors)
	{
		UEnvironmentalHazardComponent* HazardComp = Actor->FindComponentByClass<UEnvironmentalHazardComponent>();
		if (HazardComp && HazardComp->CanTrigger())
		{
			float Dist = FVector::Dist(NearActor->GetActorLocation(), Actor->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestHazard = Actor;
			}
		}
	}

	return ClosestHazard;
}

int32 ACombatAIController::GetEngagingAICount() const
{
	if (!CurrentTarget) return 0;

	int32 Count = 0;
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == GetPawn()) continue; // Don't count ourselves

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn) continue;

		ACombatAIController* OtherAI = Cast<ACombatAIController>(OtherPawn->GetController());
		if (!OtherAI) continue;

		// Check if other AI is close to the same target
		float Dist = FVector::Dist(OtherPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Dist <= PreferredCombatRange * 1.5f)
		{
			Count++;
		}
	}

	return Count;
}
