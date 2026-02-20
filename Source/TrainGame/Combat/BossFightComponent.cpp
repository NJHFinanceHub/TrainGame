// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BossFightComponent.h"
#include "CombatComponent.h"
#include "EnemyCharacterBase.h"
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

	OwnerCombat = GetOwner() ? GetOwner()->FindComponentByClass<UCombatComponent>() : nullptr;
	if (OwnerCombat)
	{
		OwnerCombat->OnDeath.AddDynamic(this, &UBossFightComponent::OnBossDeath);
	}
}

void UBossFightComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bFightActive) return;
	if (!OwnerCombat || !OwnerCombat->IsAlive()) return;

	// Phase transition invulnerability
	if (bInPhaseTransition)
	{
		PhaseTransitionTimer -= DeltaTime;
		if (PhaseTransitionTimer <= 0.f)
		{
			bInPhaseTransition = false;
		}
		return;
	}

	CheckPhaseTransition();
	UpdateSpecialAttackCooldown(DeltaTime);
}

void UBossFightComponent::InitBossFight()
{
	bFightActive = true;
	CurrentPhase = EBossPhase::Phase1;
	SpecialAttackTimer = SpecialAttackCooldown;

	ApplyBossPreset(BossIdentity);

	UE_LOG(LogTemp, Log, TEXT("Boss fight initiated: %s"), *UEnum::GetValueAsString(BossIdentity));
}

void UBossFightComponent::ApplyBossPreset(EBossIdentity Identity)
{
	BossIdentity = Identity;
	PhaseConfigs.Empty();
	SpecialAttackNames.Empty();

	switch (Identity)
	{
		case EBossIdentity::ThePitChampion:
		{
			BossMaxHealth = 400.f;
			SpecialAttackCooldown = 10.f;
			MaxAdds = 2;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 1.0f; P1.SpecialAttackCooldown = 12.f;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.6f;
			P2.DamageMultiplier = 1.3f; P2.SpeedMultiplier = 1.1f; P2.SpecialAttackCooldown = 8.f;
			P2.bSpawnsAdds = true; P2.AddCount = 2;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Enraged; P3.HealthThreshold = 0.25f;
			P3.DamageMultiplier = 1.8f; P3.SpeedMultiplier = 1.3f; P3.SpecialAttackCooldown = 5.f;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("GroundSlam"));
			SpecialAttackNames.Add(FName("BullRush"));
			SpecialAttackNames.Add(FName("CrowdCall"));
			break;
		}

		case EBossIdentity::ChefPierrot:
		{
			BossMaxHealth = 350.f;
			SpecialAttackCooldown = 8.f;
			MaxAdds = 3;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 1.2f; P1.SpecialAttackCooldown = 10.f;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.5f;
			P2.DamageMultiplier = 1.5f; P2.SpeedMultiplier = 1.3f; P2.SpecialAttackCooldown = 6.f;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Enraged; P3.HealthThreshold = 0.2f;
			P3.DamageMultiplier = 2.0f; P3.SpeedMultiplier = 1.5f; P3.SpecialAttackCooldown = 4.f;
			P3.bSpawnsAdds = true; P3.AddCount = 3;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("CleaverCombo"));
			SpecialAttackNames.Add(FName("BoilingOilThrow"));
			SpecialAttackNames.Add(FName("KitchenFireIgnite"));
			break;
		}

		case EBossIdentity::JackbootCommander:
		{
			BossMaxHealth = 500.f;
			SpecialAttackCooldown = 12.f;
			MaxAdds = 4;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 0.9f; P1.SpecialAttackCooldown = 15.f;
			P1.bSpawnsAdds = true; P1.AddCount = 2;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.5f;
			P2.DamageMultiplier = 1.3f; P2.SpeedMultiplier = 1.0f; P2.SpecialAttackCooldown = 10.f;
			P2.bSpawnsAdds = true; P2.AddCount = 3;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Enraged; P3.HealthThreshold = 0.2f;
			P3.DamageMultiplier = 1.5f; P3.SpeedMultiplier = 1.2f; P3.SpecialAttackCooldown = 6.f;
			P3.bSpawnsAdds = true; P3.AddCount = 4;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("FormationCharge"));
			SpecialAttackNames.Add(FName("ShieldBash"));
			SpecialAttackNames.Add(FName("CallReinforcements"));
			break;
		}

		case EBossIdentity::KronoleLord:
		{
			BossMaxHealth = 300.f;
			SpecialAttackCooldown = 6.f;
			MaxAdds = 0;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.2f; P1.SpeedMultiplier = 1.0f; P1.SpecialAttackCooldown = 8.f;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.5f;
			P2.DamageMultiplier = 1.5f; P2.SpeedMultiplier = 1.5f; P2.SpecialAttackCooldown = 5.f;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Enraged; P3.HealthThreshold = 0.15f;
			P3.DamageMultiplier = 2.0f; P3.SpeedMultiplier = 2.0f; P3.SpecialAttackCooldown = 3.f;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("TimeFracture"));
			SpecialAttackNames.Add(FName("KronoleBurst"));
			SpecialAttackNames.Add(FName("SlowField"));
			break;
		}

		case EBossIdentity::FirstClassDuelist:
		{
			BossMaxHealth = 350.f;
			SpecialAttackCooldown = 5.f;
			MaxAdds = 0;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 1.4f; P1.SpecialAttackCooldown = 7.f;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.4f;
			P2.DamageMultiplier = 1.3f; P2.SpeedMultiplier = 1.6f; P2.SpecialAttackCooldown = 4.f;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Enraged; P3.HealthThreshold = 0.15f;
			P3.DamageMultiplier = 1.8f; P3.SpeedMultiplier = 2.0f; P3.SpecialAttackCooldown = 3.f;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("FlurryOfBlades"));
			SpecialAttackNames.Add(FName("Riposte"));
			SpecialAttackNames.Add(FName("PerfectParry"));
			break;
		}

		case EBossIdentity::OrderHighPriest:
		{
			BossMaxHealth = 450.f;
			SpecialAttackCooldown = 10.f;
			MaxAdds = 6;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 0.8f; P1.SpecialAttackCooldown = 12.f;
			P1.bSpawnsAdds = true; P1.AddCount = 3;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.5f;
			P2.DamageMultiplier = 1.5f; P2.SpeedMultiplier = 1.0f; P2.SpecialAttackCooldown = 8.f;
			P2.bSpawnsAdds = true; P2.AddCount = 4;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Phase3; P3.HealthThreshold = 0.25f;
			P3.DamageMultiplier = 2.0f; P3.SpeedMultiplier = 1.2f; P3.SpecialAttackCooldown = 5.f;
			P3.bSpawnsAdds = true; P3.AddCount = 6;
			PhaseConfigs.Add(P3);

			SpecialAttackNames.Add(FName("DivinePunishment"));
			SpecialAttackNames.Add(FName("SummonZealots"));
			SpecialAttackNames.Add(FName("FaithfulShield"));
			break;
		}

		case EBossIdentity::MrWilford:
		{
			BossMaxHealth = 600.f;
			SpecialAttackCooldown = 8.f;
			MaxAdds = 2;

			FBossPhaseConfig P1; P1.Phase = EBossPhase::Phase1; P1.HealthThreshold = 1.0f;
			P1.DamageMultiplier = 1.0f; P1.SpeedMultiplier = 1.0f; P1.SpecialAttackCooldown = 10.f;
			PhaseConfigs.Add(P1);

			FBossPhaseConfig P2; P2.Phase = EBossPhase::Phase2; P2.HealthThreshold = 0.6f;
			P2.DamageMultiplier = 1.3f; P2.SpeedMultiplier = 1.1f; P2.SpecialAttackCooldown = 8.f;
			PhaseConfigs.Add(P2);

			FBossPhaseConfig P3; P3.Phase = EBossPhase::Phase3; P3.HealthThreshold = 0.3f;
			P3.DamageMultiplier = 1.5f; P3.SpeedMultiplier = 1.2f; P3.SpecialAttackCooldown = 6.f;
			P3.bSpawnsAdds = true; P3.AddCount = 2;
			PhaseConfigs.Add(P3);

			FBossPhaseConfig PE; PE.Phase = EBossPhase::Enraged; PE.HealthThreshold = 0.1f;
			PE.DamageMultiplier = 2.5f; PE.SpeedMultiplier = 1.5f; PE.SpecialAttackCooldown = 3.f;
			PhaseConfigs.Add(PE);

			SpecialAttackNames.Add(FName("EngineOverload"));
			SpecialAttackNames.Add(FName("SteamBlast"));
			SpecialAttackNames.Add(FName("TrainBrake"));
			SpecialAttackNames.Add(FName("FinalOrder"));
			break;
		}
	}

	// Apply health to combat component
	if (OwnerCombat)
	{
		// CombatComponent MaxHealth would be set via the component's EditAnywhere properties
		// For runtime initialization, we'd need a setter — for now log the intended value
		UE_LOG(LogTemp, Log, TEXT("Boss %s configured: %f HP, %d phases, %d special attacks"),
			*UEnum::GetValueAsString(Identity), BossMaxHealth,
			PhaseConfigs.Num(), SpecialAttackNames.Num());
	}
}

void UBossFightComponent::PerformSpecialAttack(FName AttackName)
{
	if (!bFightActive || !OwnerCombat || !OwnerCombat->IsAlive()) return;
	if (SpecialAttackTimer > 0.f) return;

	OnBossSpecialAttack.Broadcast(AttackName);

	// Reset cooldown based on current phase
	float Cooldown = SpecialAttackCooldown;
	for (const FBossPhaseConfig& Config : PhaseConfigs)
	{
		if (Config.Phase == CurrentPhase)
		{
			Cooldown = Config.SpecialAttackCooldown;
			break;
		}
	}
	SpecialAttackTimer = Cooldown;

	UE_LOG(LogTemp, Log, TEXT("Boss special attack: %s"), *AttackName.ToString());
}

void UBossFightComponent::CheckPhaseTransition()
{
	if (!OwnerCombat) return;

	float HealthPercent = OwnerCombat->GetCurrentHealth() / BossMaxHealth;

	// Find the appropriate phase for current health
	EBossPhase TargetPhase = EBossPhase::Phase1;
	for (int32 i = PhaseConfigs.Num() - 1; i >= 0; --i)
	{
		if (HealthPercent <= PhaseConfigs[i].HealthThreshold)
		{
			TargetPhase = PhaseConfigs[i].Phase;
		}
	}

	if (TargetPhase != CurrentPhase)
	{
		TransitionToPhase(TargetPhase);
	}
}

void UBossFightComponent::TransitionToPhase(EBossPhase NewPhase)
{
	EBossPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;

	// Brief invulnerability during transition
	bInPhaseTransition = true;
	PhaseTransitionTimer = PhaseTransitionInvulnDuration;

	// Spawn adds if configured
	for (const FBossPhaseConfig& Config : PhaseConfigs)
	{
		if (Config.Phase == NewPhase && Config.bSpawnsAdds)
		{
			SpawnAdds(Config.AddCount);
		}
	}

	OnBossPhaseChanged.Broadcast(NewPhase, OldPhase);

	UE_LOG(LogTemp, Log, TEXT("Boss phase transition: %s -> %s"),
		*UEnum::GetValueAsString(OldPhase), *UEnum::GetValueAsString(NewPhase));
}

void UBossFightComponent::SpawnAdds(int32 Count)
{
	if (!AddClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	int32 ToSpawn = FMath::Min(Count, MaxAdds - CurrentAddsAlive);
	if (ToSpawn <= 0) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	for (int32 i = 0; i < ToSpawn; ++i)
	{
		// Spawn adds in a semicircle around the boss
		float Angle = (2.f * PI * i) / ToSpawn;
		FVector Offset(FMath::Cos(Angle) * 500.f, FMath::Sin(Angle) * 500.f, 0.f);
		FVector SpawnLoc = Owner->GetActorLocation() + Offset;

		APawn* Add = World->SpawnActor<APawn>(AddClass, SpawnLoc, Owner->GetActorRotation(), SpawnParams);
		if (Add)
		{
			CurrentAddsAlive++;
		}
	}

	OnBossAddsSpawned.Broadcast(ToSpawn);
}

void UBossFightComponent::UpdateSpecialAttackCooldown(float DeltaTime)
{
	if (SpecialAttackTimer > 0.f)
	{
		SpecialAttackTimer -= DeltaTime;
		if (SpecialAttackTimer <= 0.f)
		{
			ChooseSpecialAttack();
		}
	}
}

void UBossFightComponent::ChooseSpecialAttack()
{
	if (SpecialAttackNames.Num() == 0) return;

	int32 Index = FMath::RandRange(0, SpecialAttackNames.Num() - 1);
	PerformSpecialAttack(SpecialAttackNames[Index]);
}

void UBossFightComponent::OnBossDeath(AActor* Killer)
{
	if (!bFightActive) return;

	bFightActive = false;
	CurrentPhase = EBossPhase::Defeated;

	OnBossDefeated.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("Boss %s defeated!"), *UEnum::GetValueAsString(BossIdentity));
}
