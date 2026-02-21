// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BossFightComponent.h"
#include "CombatComponent.h"

UBossFightComponent::UBossFightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBossFightComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>();
	EnrageCountdown = EnrageTimer;

	// Sort phase configs by health threshold (descending)
	PhaseConfigs.Sort([](const FBossPhaseConfig& A, const FBossPhaseConfig& B)
	{
		return A.HealthThreshold > B.HealthThreshold;
	});
}

void UBossFightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CombatComp || !CombatComp->IsAlive() || CurrentPhase == EBossPhase::Defeated)
	{
		return;
	}

	// Phase transition invulnerability
	if (bInTransition)
	{
		TransitionTimer -= DeltaTime;
		if (TransitionTimer <= 0.f)
		{
			bInTransition = false;
		}
		return;
	}

	// Check for phase transitions
	CheckPhaseTransition();

	// Wind-up timer for pending attacks
	if (bIsWindingUp && PendingAttack)
	{
		WindUpTimer -= DeltaTime;
		if (WindUpTimer <= 0.f)
		{
			bIsWindingUp = false;
			ExecuteSpecialAttack(*PendingAttack, PendingTarget);
			PendingAttack = nullptr;
			PendingTarget = nullptr;
		}
		return; // Don't do other actions while winding up
	}

	// Special attack cooldowns
	UpdateSpecialAttackCooldowns(DeltaTime);
	SpecialAttackTimer -= DeltaTime;

	// Enrage timer
	if (EnrageTimer > 0.f && CurrentPhase != EBossPhase::Enraged)
	{
		EnrageCountdown -= DeltaTime;
		if (EnrageCountdown <= 0.f)
		{
			TransitionToPhase(EBossPhase::Enraged);
		}
	}

	// Check death
	if (!CombatComp->IsAlive())
	{
		TransitionToPhase(EBossPhase::Defeated);
	}
}

float UBossFightComponent::GetHealthPercent() const
{
	if (!CombatComp) return 0.f;
	return CombatComp->GetHealthPercent();
}

bool UBossFightComponent::TrySpecialAttack(AActor* Target)
{
	if (bIsWindingUp || bInTransition) return false;
	if (SpecialAttackTimer > 0.f) return false;

	FBossAttack* Attack = SelectBestAttack(Target);
	if (!Attack) return false;

	// Begin wind-up (telegraph to player)
	bIsWindingUp = true;
	WindUpTimer = Attack->WindUpTime;
	PendingAttack = Attack;
	PendingTarget = Target;

	OnBossAttackWindUp.Broadcast(*Attack);
	return true;
}

void UBossFightComponent::CheckPhaseTransition()
{
	if (!CombatComp) return;

	float HealthPct = CombatComp->GetHealthPercent();

	for (const FBossPhaseConfig& Config : PhaseConfigs)
	{
		if (HealthPct <= Config.HealthThreshold && Config.Phase > CurrentPhase
			&& Config.Phase != EBossPhase::Enraged)
		{
			TransitionToPhase(Config.Phase);
			return;
		}
	}
}

void UBossFightComponent::TransitionToPhase(EBossPhase NewPhase)
{
	if (NewPhase == CurrentPhase) return;

	EBossPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	// Apply phase modifiers
	const FBossPhaseConfig* Config = GetPhaseConfig(NewPhase);
	if (Config && CombatComp)
	{
		// Apply damage resistance for this phase
		if (Config->DamageResistance > 0.f)
		{
			CombatComp->SetDamageResistance(EDamageType::Physical, 1.f - Config->DamageResistance);
			CombatComp->SetDamageResistance(EDamageType::Blunt, 1.f - Config->DamageResistance);
			CombatComp->SetDamageResistance(EDamageType::Bladed, 1.f - Config->DamageResistance);
			CombatComp->SetDamageResistance(EDamageType::Piercing, 1.f - Config->DamageResistance);
		}
	}

	// Phase transition invulnerability
	if (bInvulnerableDuringTransition && NewPhase != EBossPhase::Defeated)
	{
		bInTransition = true;
		TransitionTimer = TransitionDuration;
	}

	OnPhaseChanged.Broadcast(OldPhase, NewPhase);

	if (NewPhase == EBossPhase::Enraged)
	{
		OnBossEnraged.Broadcast();
	}
	else if (NewPhase == EBossPhase::Defeated)
	{
		OnBossDefeated.Broadcast();
	}
}

void UBossFightComponent::UpdateSpecialAttackCooldowns(float DeltaTime)
{
	for (FBossAttack& Attack : SpecialAttacks)
	{
		if (Attack.CooldownTimer > 0.f)
		{
			Attack.CooldownTimer -= DeltaTime;
		}
	}
}

void UBossFightComponent::ExecuteSpecialAttack(FBossAttack& Attack, AActor* Target)
{
	if (!CombatComp || !Target) return;

	// Apply attack cooldown
	Attack.CooldownTimer = Attack.Cooldown;
	SpecialAttackTimer = SpecialAttackInterval;

	// Apply enrage damage multiplier
	float FinalDamage = Attack.Damage;
	const FBossPhaseConfig* Config = GetPhaseConfig(CurrentPhase);
	if (Config)
	{
		FinalDamage *= Config->DamageMultiplier;
	}
	if (CurrentPhase == EBossPhase::Enraged)
	{
		FinalDamage *= EnrageDamageMultiplier;
	}

	// Apply damage based on attack type
	UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
	if (TargetCombat)
	{
		// Check range
		float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
		if (Dist <= Attack.Range)
		{
			// Determine if target can defend
			EDamageType DmgType = EDamageType::Physical;

			// Unblockable attacks bypass blocking entirely
			if (!Attack.bBlockable)
			{
				// Direct damage, no block check
			}

			TargetCombat->ReceiveAttack(FinalDamage, EAttackDirection::Mid, DmgType, GetOwner());
		}
	}

	OnBossAttackExecuted.Broadcast(Attack);
}

FBossAttack* UBossFightComponent::SelectBestAttack(AActor* Target)
{
	if (!Target) return nullptr;

	float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());

	FBossAttack* BestAttack = nullptr;
	float BestScore = -1.f;

	for (FBossAttack& Attack : SpecialAttacks)
	{
		if (!Attack.IsReady()) continue;

		// Check if available in current phase
		bool bAvailable = false;
		for (EBossPhase Phase : Attack.AvailableInPhases)
		{
			if (Phase == CurrentPhase)
			{
				bAvailable = true;
				break;
			}
		}
		if (!bAvailable) continue;

		// Score based on range appropriateness
		float Score = 0.f;
		if (Dist <= Attack.Range)
		{
			Score = Attack.Damage; // Prefer high-damage attacks when in range
		}
		else
		{
			Score = Attack.Range - Dist; // Prefer long-range attacks when far
		}

		if (Score > BestScore)
		{
			BestScore = Score;
			BestAttack = &Attack;
		}
	}

	return BestAttack;
}

const FBossPhaseConfig* UBossFightComponent::GetPhaseConfig(EBossPhase Phase) const
{
	for (const FBossPhaseConfig& Config : PhaseConfigs)
	{
		if (Config.Phase == Phase)
		{
			return &Config;
		}
	}
	return nullptr;
}
