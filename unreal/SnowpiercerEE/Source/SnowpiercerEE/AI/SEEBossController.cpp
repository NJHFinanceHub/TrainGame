// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEBossController.h"
#include "SEEBossGrey.h"
#include "TrainGame/Combat/BossFightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ASEEBossController::ASEEBossController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASEEBossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	GreyPawn = Cast<ASEEBossGrey>(InPawn);
}

void ASEEBossController::OnUnPossess()
{
	GreyPawn = nullptr;
	Super::OnUnPossess();
}

void ASEEBossController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GreyPawn) return;

	// Check for state transitions based on boss phase
	UBossFightComponent* BossComp = GreyPawn->GetBossFightComponent();
	if (!BossComp) return;

	if (BossComp->IsDefeated())
	{
		SetAIState(EGreyAIState::Defeated);
		return;
	}

	// Auto-transition based on phase and health
	float HealthPct = BossComp->GetHealthPercent();

	if (CurrentState == EGreyAIState::Commanding && ShouldDescendFromPost())
	{
		SetAIState(EGreyAIState::MeleeAssault);
	}
	else if (CurrentState == EGreyAIState::MeleeAssault && HealthPct < DesperateHealthThreshold)
	{
		SetAIState(EGreyAIState::Desperate);
	}

	// Update current behavior
	switch (CurrentState)
	{
	case EGreyAIState::Commanding:		UpdateCommanding(DeltaTime); break;
	case EGreyAIState::EngagingRanged:	UpdateEngagingRanged(DeltaTime); break;
	case EGreyAIState::MeleeAssault:	UpdateMeleeAssault(DeltaTime); break;
	case EGreyAIState::Desperate:		UpdateDesperate(DeltaTime); break;
	case EGreyAIState::Defeated:		StopMovement(); break;
	}
}

void ASEEBossController::SetAIState(EGreyAIState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
}

bool ASEEBossController::ShouldDescendFromPost() const
{
	return GreyPawn && GreyPawn->AreAllGuardsDead();
}

// --- Phase 1: Commanding ---

void ASEEBossController::UpdateCommanding(float DeltaTime)
{
	// Stay at command post
	MoveToLocation(CommandPostLocation, 50.f);

	// Rally Cry on interval
	RallyCryTimer += DeltaTime;
	if (RallyCryTimer >= RallyCryInterval)
	{
		RallyCryTimer = 0.f;
		TrySpecialAttack();
	}

	// Spotlight on interval
	SpotlightTimer += DeltaTime;
	if (SpotlightTimer >= SpotlightInterval && GreyPawn)
	{
		SpotlightTimer = 0.f;
		GreyPawn->ActivateSpotlight();
		// Auto-deactivate after 3 seconds via timer
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, [this]()
		{
			if (GreyPawn) GreyPawn->DeactivateSpotlight();
		}, 3.0f, false);
	}
}

// --- Phase 2: Ranged Engagement ---

void ASEEBossController::UpdateEngagingRanged(float DeltaTime)
{
	// Move between cover points
	CoverSwitchTimer += DeltaTime;
	if (CoverSwitchTimer >= CoverSwitchInterval && CoverPoints.Num() > 0)
	{
		CoverSwitchTimer = 0.f;
		CurrentCoverIndex = (CurrentCoverIndex + 1) % CoverPoints.Num();
		MoveToLocation(CoverPoints[CurrentCoverIndex], 50.f);
	}

	// Try environmental attacks
	AttackTimer += DeltaTime;
	if (AttackTimer >= MeleeAttackInterval * 2.f)
	{
		AttackTimer = 0.f;
		TrySpecialAttack();
	}
}

// --- Phase 3: Melee Assault ---

void ASEEBossController::UpdateMeleeAssault(float DeltaTime)
{
	MoveToTarget();

	AttackTimer += DeltaTime;
	if (AttackTimer >= MeleeAttackInterval)
	{
		AttackTimer = 0.f;
		TrySpecialAttack();
	}
}

// --- Phase 3 (Desperate): All-in aggression ---

void ASEEBossController::UpdateDesperate(float DeltaTime)
{
	MoveToTarget();

	// Faster attack cadence when desperate
	AttackTimer += DeltaTime;
	if (AttackTimer >= MeleeAttackInterval * 0.6f)
	{
		AttackTimer = 0.f;
		TrySpecialAttack();
	}
}

// --- Helpers ---

void ASEEBossController::TrySpecialAttack()
{
	if (!GreyPawn) return;

	AActor* Target = GetTarget();
	if (Target)
	{
		UBossFightComponent* BossComp = GreyPawn->GetBossFightComponent();
		if (BossComp)
		{
			BossComp->TrySpecialAttack(Target);
		}
	}
}

void ASEEBossController::MoveToTarget()
{
	AActor* Target = GetTarget();
	if (Target)
	{
		MoveToActor(Target, 150.f);
	}
}

AActor* ASEEBossController::GetTarget() const
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	return PC ? PC->GetPawn() : nullptr;
}
