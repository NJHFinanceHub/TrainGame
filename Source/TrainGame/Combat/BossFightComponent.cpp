// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BossFightComponent.h"
#include "CombatComponent.h"
#include "TrainGame/Environment/EnvironmentalHazardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UBossFightComponent::UBossFightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBossFightComponent::BeginPlay()
{
	Super::BeginPlay();

	CombatComp = GetOwner()->FindComponentByClass<UCombatComponent>();
}

void UBossFightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsInitialized || !CombatComp) return;
	if (!CombatComp->IsAlive())
	{
		if (CurrentPhase != EBossPhase::Defeated)
		{
			CurrentPhase = EBossPhase::Defeated;
			OnBossDefeated.Broadcast();
		}
		return;
	}

	// Phase transition invulnerability
	if (bIsTransitioning)
	{
		TransitionTimer -= DeltaTime;
		if (TransitionTimer <= 0.f)
		{
			bIsTransitioning = false;
		}
		return; // Don't act during transitions
	}

	CheckPhaseTransition();
	UpdateEnrageTimer(DeltaTime);
	UpdateSpecialAttackTimer(DeltaTime);

	// Health regen if current phase allows it
	if (CurrentPhaseIndex < BossData.Phases.Num())
	{
		float RegenRate = BossData.Phases[CurrentPhaseIndex].HealthRegenPerSecond;
		if (RegenRate > 0.f)
		{
			CombatComp->Heal(RegenRate * DeltaTime);
		}
	}
}

void UBossFightComponent::InitBoss(const FBossData& InBossData)
{
	BossData = InBossData;
	bIsInitialized = true;

	if (CombatComp)
	{
		CombatComp->SetMaxHealth(BossData.MaxHealth);
		CombatComp->Heal(BossData.MaxHealth); // Full heal on init

		// Bosses can't be stealth-takedown'd
		// (they use bCanBeTakenDown = false, set via the CombatComponent)
	}

	// Apply phase 1 modifiers
	if (BossData.Phases.Num() > 0)
	{
		ApplyPhaseModifiers(BossData.Phases[0]);
	}

	CurrentPhase = EBossPhase::Phase1;
	CurrentPhaseIndex = 0;
}

void UBossFightComponent::CheckPhaseTransition()
{
	if (!CombatComp || BossData.Phases.Num() == 0) return;

	float HealthPercent = CombatComp->GetHealthPercent();

	// Check if we should move to the next phase
	int32 NextPhase = CurrentPhaseIndex + 1;
	if (NextPhase < BossData.Phases.Num())
	{
		if (HealthPercent <= BossData.Phases[NextPhase].HealthThreshold)
		{
			TransitionToPhase(NextPhase);
		}
	}
}

void UBossFightComponent::TransitionToPhase(int32 PhaseIndex)
{
	if (PhaseIndex >= BossData.Phases.Num()) return;
	if (PhaseIndex == CurrentPhaseIndex) return;

	CurrentPhaseIndex = PhaseIndex;

	// Determine the EBossPhase enum value
	if (PhaseIndex == 0)
		CurrentPhase = EBossPhase::Phase1;
	else if (PhaseIndex == 1)
		CurrentPhase = EBossPhase::Phase2;
	else if (PhaseIndex == 2)
		CurrentPhase = EBossPhase::Phase3;
	else
		CurrentPhase = EBossPhase::Phase3; // Extra phases stay as Phase3

	ApplyPhaseModifiers(BossData.Phases[PhaseIndex]);

	// Brief invulnerability during transition
	bIsTransitioning = true;
	TransitionTimer = PhaseTransitionDuration;

	OnBossPhaseChanged.Broadcast(CurrentPhase, PhaseIndex);
}

void UBossFightComponent::ForcePhaseTransition(int32 PhaseIndex)
{
	TransitionToPhase(PhaseIndex);
}

void UBossFightComponent::ApplyPhaseModifiers(const FBossPhaseConfig& PhaseConfig)
{
	if (!CombatComp) return;

	CombatComp->SetStaggerResistance(PhaseConfig.StaggerResistance);

	// Summon minions if the phase calls for it
	if (PhaseConfig.MinionCount > 0)
	{
		// Check if Summon is in available attacks
		for (EBossAttackType Attack : PhaseConfig.AvailableAttacks)
		{
			if (Attack == EBossAttackType::Summon)
			{
				SummonMinions(PhaseConfig.MinionCount);
				break;
			}
		}
	}
}

float UBossFightComponent::GetPhaseProgress() const
{
	if (!CombatComp || BossData.Phases.Num() == 0) return 0.f;

	float HealthPercent = CombatComp->GetHealthPercent();
	float CurrentThreshold = (CurrentPhaseIndex < BossData.Phases.Num())
		? BossData.Phases[CurrentPhaseIndex].HealthThreshold : 0.f;

	float PreviousThreshold = (CurrentPhaseIndex > 0)
		? BossData.Phases[CurrentPhaseIndex - 1].HealthThreshold : 1.f;

	float Range = PreviousThreshold - CurrentThreshold;
	if (Range <= 0.f) return 1.f;

	return 1.f - (HealthPercent - CurrentThreshold) / Range;
}

// ============================================================================
// Timer Updates
// ============================================================================

void UBossFightComponent::UpdateEnrageTimer(float DeltaTime)
{
	if (bIsEnraged) return;

	FightTimer += DeltaTime;
	if (FightTimer >= EnrageTimer)
	{
		bIsEnraged = true;
		CurrentPhase = EBossPhase::Enraged;

		// Enrage: massive stat boost
		if (CombatComp)
		{
			CombatComp->SetStaggerResistance(1.f); // Immune to stagger
		}

		OnBossEnraged.Broadcast();
	}
}

void UBossFightComponent::UpdateSpecialAttackTimer(float DeltaTime)
{
	if (SpecialAttackTimer > 0.f)
	{
		SpecialAttackTimer -= DeltaTime;
		return;
	}

	// Attempt a special attack based on current phase
	AttemptSpecialAttack();
}

void UBossFightComponent::AttemptSpecialAttack()
{
	if (CurrentPhaseIndex >= BossData.Phases.Num()) return;

	const FBossPhaseConfig& Phase = BossData.Phases[CurrentPhaseIndex];
	if (Phase.AvailableAttacks.Num() == 0) return;

	// Pick a random available special attack
	int32 AttackIdx = FMath::RandRange(0, Phase.AvailableAttacks.Num() - 1);
	EBossAttackType ChosenAttack = Phase.AvailableAttacks[AttackIdx];

	// Only trigger special attacks probabilistically
	float SpecialAttackChance = bIsEnraged ? 0.6f : 0.3f;
	if (FMath::FRand() > SpecialAttackChance) return;

	PerformSpecialAttack(ChosenAttack);
}

// ============================================================================
// Special Attacks
// ============================================================================

void UBossFightComponent::PerformSpecialAttack(EBossAttackType AttackType)
{
	SpecialAttackTimer = SpecialAttackCooldown;

	switch (AttackType)
	{
		case EBossAttackType::AreaOfEffect:
			PerformAOEAttack();
			break;
		case EBossAttackType::Charge:
			PerformChargeAttack();
			break;
		case EBossAttackType::GrabThrow:
			PerformGrabThrow();
			break;
		case EBossAttackType::Summon:
			if (CurrentPhaseIndex < BossData.Phases.Num())
			{
				SummonMinions(BossData.Phases[CurrentPhaseIndex].MinionCount);
			}
			break;
		case EBossAttackType::EnvironmentalTrigger:
			TriggerArenaHazard();
			break;
		case EBossAttackType::Unblockable:
		{
			// Unblockable heavy strike — bypasses block entirely
			AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
			if (Player && CombatComp)
			{
				UCombatComponent* PlayerCombat = Player->FindComponentByClass<UCombatComponent>();
				if (PlayerCombat && PlayerCombat->IsAlive())
				{
					float Damage = BossData.BaseDamage * 2.f;
					if (bIsEnraged) Damage *= EnrageDamageMultiplier;
					if (CurrentPhaseIndex < BossData.Phases.Num())
						Damage *= BossData.Phases[CurrentPhaseIndex].DamageMultiplier;

					// Apply damage directly, bypassing block (still dodgeable)
					PlayerCombat->ReceiveAttack(Damage, EAttackDirection::High, EDamageType::Blunt, GetOwner());
				}
			}
			break;
		}
		default:
			break;
	}

	OnBossSpecialAttack.Broadcast(AttackType);
}

void UBossFightComponent::PerformAOEAttack()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	float AOERadius = 400.f;
	float AOEDamage = BossData.BaseDamage * 1.5f;
	if (bIsEnraged) AOEDamage *= EnrageDamageMultiplier;
	if (CurrentPhaseIndex < BossData.Phases.Num())
		AOEDamage *= BossData.Phases[CurrentPhaseIndex].DamageMultiplier;

	// Damage all enemies in radius
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(AOERadius),
		Params
	);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor) continue;

		UCombatComponent* TargetCombat = HitActor->FindComponentByClass<UCombatComponent>();
		if (TargetCombat && TargetCombat->IsAlive())
		{
			TargetCombat->ReceiveAttack(AOEDamage, EAttackDirection::Mid, EDamageType::Blunt, Owner);

			// AOE also knockbacks
			ACharacter* TargetChar = Cast<ACharacter>(HitActor);
			if (TargetChar)
			{
				FVector KnockDir = (HitActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
				TargetChar->LaunchCharacter(KnockDir * 600.f, true, true);
			}
		}
	}
}

void UBossFightComponent::PerformChargeAttack()
{
	AActor* Owner = GetOwner();
	ACharacter* BossChar = Cast<ACharacter>(Owner);
	if (!BossChar) return;

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	// Charge toward the player
	FVector ChargeDir = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
	BossChar->LaunchCharacter(ChargeDir * 1200.f, true, false);

	// Charge damage is applied via collision (handled by the melee trace in CombatComponent)
}

void UBossFightComponent::PerformGrabThrow()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	float GrabRange = 200.f;
	float Dist = FVector::Dist(Owner->GetActorLocation(), Player->GetActorLocation());
	if (Dist > GrabRange) return;

	UCombatComponent* PlayerCombat = Player->FindComponentByClass<UCombatComponent>();
	if (!PlayerCombat || !PlayerCombat->IsAlive()) return;

	// Can't grab if player is in i-frames
	if (PlayerCombat->GetCurrentStance() == ECombatStance::Dodging) return;

	float ThrowDamage = BossData.BaseDamage;
	if (CurrentPhaseIndex < BossData.Phases.Num())
		ThrowDamage *= BossData.Phases[CurrentPhaseIndex].DamageMultiplier;

	PlayerCombat->ReceiveAttack(ThrowDamage, EAttackDirection::Mid, EDamageType::Blunt, Owner);

	// Throw the player backward
	ACharacter* PlayerChar = Cast<ACharacter>(Player);
	if (PlayerChar)
	{
		FVector ThrowDir = (Player->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
		ThrowDir.Z = 0.3f; // Slight upward arc
		PlayerChar->LaunchCharacter(ThrowDir * 800.f, true, true);
	}
}

void UBossFightComponent::SummonMinions(int32 Count)
{
	if (!MinionClass || Count <= 0) return;

	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!World || !Owner) return;

	// Clean up dead minions
	SpawnedMinions.RemoveAll([](AActor* Minion) {
		if (!Minion || !IsValid(Minion)) return true;
		UCombatComponent* Combat = Minion->FindComponentByClass<UCombatComponent>();
		return Combat && !Combat->IsAlive();
	});

	// Limit total active minions
	int32 MaxMinions = 6;
	int32 ToSpawn = FMath::Min(Count, MaxMinions - SpawnedMinions.Num());

	for (int32 i = 0; i < ToSpawn; i++)
	{
		FVector SpawnOffset = Owner->GetActorRightVector() * (i % 2 == 0 ? 1.f : -1.f) * 200.f
			+ Owner->GetActorForwardVector() * -300.f;
		FVector SpawnLoc = Owner->GetActorLocation() + SpawnOffset;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AActor* Minion = World->SpawnActor<AActor>(MinionClass, SpawnLoc, Owner->GetActorRotation(), SpawnParams);
		if (Minion)
		{
			SpawnedMinions.Add(Minion);
		}
	}
}

void UBossFightComponent::TriggerArenaHazard()
{
	AActor* Owner = GetOwner();
	if (!Owner || BossData.ArenaHazards.Num() == 0) return;

	// Find and trigger nearby environmental hazards that match the boss's arena hazards
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		UEnvironmentalHazardComponent* HazardComp = Actor->FindComponentByClass<UEnvironmentalHazardComponent>();
		if (!HazardComp || !HazardComp->CanTrigger()) continue;

		// Check if player is in the hazard zone
		AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (Player && HazardComp->IsActorInHazardZone(Player))
		{
			HazardComp->TriggerHazard(Owner);
			break; // Only trigger one hazard per special attack
		}
	}
}
