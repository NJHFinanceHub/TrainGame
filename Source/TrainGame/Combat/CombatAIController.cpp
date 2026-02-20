// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatAIController.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
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
	WeaponComp = InPawn->FindComponentByClass<UWeaponComponent>();
	ApplyProfileModifiers();
}

void ACombatAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CombatComp || !CombatComp->IsAlive()) return;
	if (CombatComp->IsStaggered()) return;
	if (CombatComp->IsGrappled()) return;

	// Zealot frenzy check
	if (Profile == ECombatAIProfile::Zealot && !bIsFrenzied)
	{
		if (CombatComp->GetHealthPercent() < FrenzyHealthThreshold)
		{
			bIsFrenzied = true;
			DecisionInterval *= 0.5f; // Faster decisions when frenzied
		}
	}

	DecisionTimer -= DeltaTime;
	if (DecisionTimer <= 0.f)
	{
		DecisionTimer = DecisionInterval;
		MakeDecision();
	}
}

void ACombatAIController::SetProfile(ECombatAIProfile NewProfile)
{
	Profile = NewProfile;
	ApplyProfileModifiers();
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

	// Profile-specific behavior hooks
	switch (Profile)
	{
		case ECombatAIProfile::Captain:
			ExecuteCaptainBehavior();
			break;
		case ECombatAIProfile::Zealot:
			ExecuteZealotBehavior();
			if (bIsFrenzied) bCanEngage = true; // Zealots ignore corridor discipline when frenzied
			break;
		case ECombatAIProfile::FirstClassGuard:
			ExecuteGuardBehavior();
			break;
		default:
			break;
	}

	// Ranged attack consideration (Captain, Guard profiles)
	if (RangedAttackChance > 0.f && DistToTarget <= PreferredRangedRange && DistToTarget > PreferredCombatRange)
	{
		if (FMath::FRand() < RangedAttackChance && WeaponComp)
		{
			PerformRangedAttack();
			return;
		}
	}

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
		// Disciplined/Cunning AI will try to find a flanking route
		if (Profile == ECombatAIProfile::Cunning || Profile == ECombatAIProfile::Disciplined ||
			Profile == ECombatAIProfile::Captain)
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

		case ECombatAIProfile::Captain:
			// Tactical, prefers varied mid/high with occasional low
			Direction = (Roll < 0.35f) ? EAttackDirection::High :
						(Roll < 0.75f) ? EAttackDirection::Mid : EAttackDirection::Low;
			break;

		case ECombatAIProfile::Zealot:
		{
			if (bIsFrenzied)
			{
				// Frenzied: all-out wild attacks, even more unpredictable
				Direction = (Roll < 0.25f) ? EAttackDirection::High :
							(Roll < 0.5f) ? EAttackDirection::Mid :
							(Roll < 0.75f) ? EAttackDirection::Low :
							(Roll < 0.875f) ? EAttackDirection::Left : EAttackDirection::Right;
			}
			else
			{
				// Measured religious fervor — calculated strikes
				Direction = (Roll < 0.4f) ? EAttackDirection::High :
							(Roll < 0.7f) ? EAttackDirection::Mid : EAttackDirection::Low;
			}
			break;
		}

		case ECombatAIProfile::FirstClassGuard:
			// Precise, deliberate — favors high (fencing style)
			Direction = (Roll < 0.45f) ? EAttackDirection::High :
						(Roll < 0.8f) ? EAttackDirection::Mid : EAttackDirection::Low;
			break;

		default:
			Direction = EAttackDirection::Mid;
	}

	CombatComp->PerformAttack(Direction);
}

void ACombatAIController::PerformRangedAttack()
{
	if (!CombatComp || !CurrentTarget) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	FVector AimDir = (CurrentTarget->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
	CombatComp->PerformRangedAttack(AimDir);
}

void ACombatAIController::ConsiderBlock()
{
	if (!CombatComp) return;

	float EffectiveBlockChance = BlockChance;

	// First Class Guards are exceptional blockers
	if (Profile == ECombatAIProfile::FirstClassGuard)
	{
		EffectiveBlockChance *= 1.3f;
	}

	if (FMath::FRand() < EffectiveBlockChance)
	{
		// Block in a semi-random direction (AI doesn't perfectly predict)
		float Roll = FMath::FRand();
		EBlockDirection Dir;

		if (Profile == ECombatAIProfile::FirstClassGuard || Profile == ECombatAIProfile::Captain)
		{
			// More accurate blocking for elite enemies
			Dir = (Roll < 0.5f) ? EBlockDirection::Mid :
				  (Roll < 0.8f) ? EBlockDirection::High : EBlockDirection::Low;
		}
		else
		{
			Dir = (Roll < 0.4f) ? EBlockDirection::Mid :
				  (Roll < 0.7f) ? EBlockDirection::High : EBlockDirection::Low;
		}

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
			DecisionInterval = 0.8f;
			EnvironmentalUseChance = 0.05f;
			RangedAttackChance = 0.f;
			CounterAttackChance = 0.f;
			SuicidalChargeChance = 0.f;
			break;

		case ECombatAIProfile::Disciplined:
			BlockChance = 0.5f;
			DodgeChance = 0.2f;
			DecisionInterval = 0.4f;
			EnvironmentalUseChance = 0.1f;
			MaxSimultaneousAttackers = 3;
			RangedAttackChance = 0.f;
			CounterAttackChance = 0.f;
			SuicidalChargeChance = 0.f;
			break;

		case ECombatAIProfile::Cunning:
			BlockChance = 0.3f;
			DodgeChance = 0.35f;
			DecisionInterval = 0.5f;
			EnvironmentalUseChance = 0.4f;
			RangedAttackChance = 0.f;
			CounterAttackChance = 0.f;
			SuicidalChargeChance = 0.f;
			break;

		case ECombatAIProfile::Brute:
			BlockChance = 0.1f;
			DodgeChance = 0.05f;
			DecisionInterval = 1.0f;
			EnvironmentalUseChance = 0.02f;
			RangedAttackChance = 0.f;
			CounterAttackChance = 0.f;
			SuicidalChargeChance = 0.f;
			break;

		case ECombatAIProfile::Captain:
			BlockChance = 0.45f;
			DodgeChance = 0.25f;
			DecisionInterval = 0.35f;
			EnvironmentalUseChance = 0.15f;
			MaxSimultaneousAttackers = 4; // Officers coordinate larger groups
			RangedAttackChance = 0.4f;
			PreferredRangedRange = 1000.f;
			CounterAttackChance = 0.2f;
			SuicidalChargeChance = 0.f;
			break;

		case ECombatAIProfile::Zealot:
			BlockChance = 0.15f; // Faith is their shield
			DodgeChance = 0.1f;
			DecisionInterval = 0.6f;
			EnvironmentalUseChance = 0.05f;
			RangedAttackChance = 0.f;
			CounterAttackChance = 0.f;
			SuicidalChargeChance = 0.5f;
			FrenzyHealthThreshold = 0.3f;
			break;

		case ECombatAIProfile::FirstClassGuard:
			BlockChance = 0.6f; // Best blockers in the train
			DodgeChance = 0.3f;
			DecisionInterval = 0.3f; // Fastest reactions
			EnvironmentalUseChance = 0.08f;
			RangedAttackChance = 0.25f;
			PreferredRangedRange = 800.f;
			CounterAttackChance = 0.5f; // Counter-attack after perfect block
			SuicidalChargeChance = 0.f;
			break;
	}
}

// ============================================================================
// Profile-Specific Behaviors
// ============================================================================

void ACombatAIController::ExecuteCaptainBehavior()
{
	// Captains buff nearby allies periodically
	BuffNearbyAllies();
}

void ACombatAIController::ExecuteZealotBehavior()
{
	if (!bIsFrenzied || !CombatComp || !CurrentTarget) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Frenzied zealots may perform suicidal charges
	float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

	if (DistToTarget > PreferredCombatRange * 2.f && FMath::FRand() < SuicidalChargeChance)
	{
		// Charge directly at the target, ignoring corridor discipline
		ACharacter* ControlledChar = Cast<ACharacter>(ControlledPawn);
		if (ControlledChar)
		{
			FVector ChargeDir = (CurrentTarget->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledChar->LaunchCharacter(ChargeDir * 800.f, true, false);
		}
	}
}

void ACombatAIController::ExecuteGuardBehavior()
{
	// First Class Guards maintain optimal distance — they don't chase aggressively
	// They prefer to let enemies come to them and counter-attack
	if (!CombatComp || !CurrentTarget) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), CurrentTarget->GetActorLocation());

	// If too close, try to create space
	if (DistToTarget < PreferredCombatRange * 0.5f)
	{
		FVector BackDir = (ControlledPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
		FVector RetreatPos = ControlledPawn->GetActorLocation() + BackDir * 200.f;
		MoveToLocation(RetreatPos);
	}
}

void ACombatAIController::BuffNearbyAllies()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Find nearby AI allies and boost their decision speed
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		if (Actor == ControlledPawn) continue;

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn) continue;

		float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), OtherPawn->GetActorLocation());
		if (Dist > CommandRadius) continue;

		ACombatAIController* OtherAI = Cast<ACombatAIController>(OtherPawn->GetController());
		if (!OtherAI) continue;

		// Captain's presence speeds up ally decision-making by 20%
		// (This is re-applied each tick but bounded by the profile's base interval)
		if (OtherAI->Profile == ECombatAIProfile::Disciplined)
		{
			OtherAI->MaxSimultaneousAttackers = FMath::Max(OtherAI->MaxSimultaneousAttackers, 4);
		}
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
