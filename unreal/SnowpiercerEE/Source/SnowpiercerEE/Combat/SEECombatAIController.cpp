// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEECombatAIController.h"
#include "SnowpiercerEE/SEECombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"

ASEECombatAIController::ASEECombatAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASEECombatAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CombatComp = InPawn->FindComponentByClass<USEECombatComponent>();
	ApplyProfileModifiers();
}

void ASEECombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// TODO: Integrate with SEECombatComponent alive/stagger checks
	DecisionTimer -= DeltaTime;
	if (DecisionTimer <= 0.f)
	{
		DecisionTimer = DecisionInterval;
		MakeDecision();
	}
}

void ASEECombatAIController::MakeDecision()
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

	if (DistToTarget <= PreferredCombatRange && bCanEngage)
	{
		if (FMath::FRand() < EnvironmentalUseChance)
		{
			ConsiderEnvironmentalKill();
			return;
		}

		PerformAttack();
	}
	else if (bCanEngage)
	{
		MoveToTarget();
	}
	else
	{
		if (Profile == ESEECombatAIProfile::Cunning || Profile == ESEECombatAIProfile::Disciplined)
		{
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

void ASEECombatAIController::MoveToTarget()
{
	if (!CurrentTarget) return;

	MoveToActor(CurrentTarget, PreferredCombatRange * 0.8f);
	bIsApproaching = true;
}

void ASEECombatAIController::PerformAttack()
{
	// TODO: Integrate attack direction logic with SEECombatComponent
}

void ASEECombatAIController::ConsiderBlock()
{
	// TODO: Integrate block logic with SEECombatComponent
}

void ASEECombatAIController::ConsiderEnvironmentalKill()
{
	// TODO: Integrate with environmental hazard system
}

void ASEECombatAIController::ApplyProfileModifiers()
{
	switch (Profile)
	{
		case ESEECombatAIProfile::Desperate:
			BlockChance = 0.15f;
			DodgeChance = 0.2f;
			DecisionInterval = 0.8f;
			EnvironmentalUseChance = 0.05f;
			break;

		case ESEECombatAIProfile::Disciplined:
			BlockChance = 0.5f;
			DodgeChance = 0.2f;
			DecisionInterval = 0.4f;
			EnvironmentalUseChance = 0.1f;
			MaxSimultaneousAttackers = 3;
			break;

		case ESEECombatAIProfile::Cunning:
			BlockChance = 0.3f;
			DodgeChance = 0.35f;
			DecisionInterval = 0.5f;
			EnvironmentalUseChance = 0.4f;
			break;

		case ESEECombatAIProfile::Brute:
			BlockChance = 0.1f;
			DodgeChance = 0.05f;
			DecisionInterval = 1.0f;
			EnvironmentalUseChance = 0.02f;
			break;
	}
}

AActor* ASEECombatAIController::FindTarget() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

AActor* ASEECombatAIController::FindNearbyHazard(AActor* NearActor) const
{
	// TODO: Integrate with environmental hazard component system
	return nullptr;
}

int32 ASEECombatAIController::GetEngagingAICount() const
{
	if (!CurrentTarget) return 0;

	int32 Count = 0;
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == GetPawn()) continue;

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn) continue;

		ASEECombatAIController* OtherAI = Cast<ASEECombatAIController>(OtherPawn->GetController());
		if (!OtherAI) continue;

		float Dist = FVector::Dist(OtherPawn->GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Dist <= PreferredCombatRange * 1.5f)
		{
			Count++;
		}
	}

	return Count;
}
