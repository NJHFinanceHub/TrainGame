// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEBossGrey.h"
#include "TrainGame/Combat/BossFightComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Combat/EnemyCharacter.h"
#include "Engine/World.h"

ASEEBossGrey::ASEEBossGrey()
{
	BossZone = EBossZone::Zone1_Tail;
}

void ASEEBossGrey::BeginPlay()
{
	// Skip ABossCharacter's SetupTailWarden — we use our own configuration
	ATrainGameCombatCharacter::BeginPlay();
	SetupCommanderGrey();
}

void ASEEBossGrey::SetupCommanderGrey()
{
	// Commander Grey stats — 3x player HP baseline, heavy armor
	EnemyStats.Health = 300.f;
	EnemyStats.DamageMultiplier = 1.3f;
	EnemyStats.MovementSpeed = 400.f;
	EnemyStats.bCanBeTakenDown = false;

	// Officer's Sword — his personal blade
	DefaultWeapon.WeaponName = FName("OfficersSword");
	DefaultWeapon.Category = EWeaponCategory::Bladed;
	DefaultWeapon.Tier = EWeaponTier::Military;
	DefaultWeapon.BaseDamage = 28.f;
	DefaultWeapon.AttackSpeed = 1.0f;
	DefaultWeapon.Range = 170.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	BossComp->BossName = FName("Commander Grey");
	BossComp->EnrageTimer = 240.f; // 4 minute enrage
	BossComp->SpecialAttackInterval = 8.f;

	// Phase 1: Gauntlet — Grey commands from post, guards engage
	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 0.8f, 0.8f, 0.3f);
	// Phase 2: Arena — shield wall, environmental attacks
	AddPhaseConfig(EBossPhase::Phase2, 0.75f, 1.0f, 1.0f, 0.2f);
	// Phase 3: 1v1 melee — aggressive, drops armor for speed
	AddPhaseConfig(EBossPhase::Phase3, 0.4f, 1.4f, 1.3f, 0.0f);

	TArray<EBossPhase> CommandPhases = { EBossPhase::Phase1, EBossPhase::Phase2 };
	TArray<EBossPhase> ArenaPhases = { EBossPhase::Phase2, EBossPhase::Enraged };
	TArray<EBossPhase> MeleePhases = { EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };

	// Spotlight Blind — from command post
	AddSpecialAttack(FName("SpotlightBlind"), EBossAttackType::EnvironmentalTrigger,
		0.f, 800.f, 10.f, 0.5f, false, true, CommandPhases);

	// Rally Cry — buffs remaining guards
	AddSpecialAttack(FName("RallyCry"), EBossAttackType::SummonMinions,
		0.f, 0.f, 15.f, 1.0f, false, false, CommandPhases);

	// Steam Conduit — area denial
	AddSpecialAttack(FName("SteamConduit"), EBossAttackType::EnvironmentalTrigger,
		35.f, 400.f, 12.f, 1.5f, false, true, ArenaPhases);

	// Sword Combo — 3-hit chain in melee
	AddSpecialAttack(FName("SwordCombo"), EBossAttackType::ChargeRush,
		45.f, 200.f, 8.f, 1.2f, true, true, MeleePhases);

	// Shield Bash — staggers player
	AddSpecialAttack(FName("ShieldBash"), EBossAttackType::AreaSlam,
		20.f, 150.f, 6.f, 0.8f, false, true, MeleePhases);

	// Unblockable Overhead — telegraphed heavy strike
	AddSpecialAttack(FName("OverheadStrike"), EBossAttackType::UnblockableStrike,
		60.f, 180.f, 14.f, 2.0f, false, true, MeleePhases);

	// Damage resistances — heavy armor
	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Physical, 0.7f);
		CombatComp->SetDamageResistance(EDamageType::Blunt, 0.6f);
		CombatComp->SetDamageResistance(EDamageType::Piercing, 0.8f);
		CombatComp->SetDamageResistance(EDamageType::Bladed, 1.0f);
	}
}

// --- Guard Squad ---

void ASEEBossGrey::SpawnGuardSquad()
{
	UWorld* World = GetWorld();
	if (!World) return;

	auto SpawnGuard = [&](TSubclassOf<AEnemyCharacter> Class, const FTransform& Transform, EEnemyType Type)
	{
		if (!Class) return;
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		AEnemyCharacter* Guard = World->SpawnActor<AEnemyCharacter>(Class, Transform, Params);
		if (Guard)
		{
			Guard->InitializeArchetype(Type);
			Guard->OnEnemyDowned.AddDynamic(this, &ASEEBossGrey::OnGuardKilled);
			SpawnedGuards.Add(Guard);
			GuardsRemaining++;
		}
	};

	// 2 Elites
	for (const FTransform& T : EliteSpawnTransforms)
	{
		SpawnGuard(JackbootEliteClass, T, EEnemyType::JackbootCaptain);
	}

	// 2 Shield Wall
	for (const FTransform& T : ShieldSpawnTransforms)
	{
		SpawnGuard(JackbootShieldClass, T, EEnemyType::JackbootGrunt);
	}

	// 1 Crossbow
	SpawnGuard(JackbootCrossbowClass, CrossbowSpawnTransform, EEnemyType::JackbootGrunt);

	OnGuardSquadChanged.Broadcast(GuardsRemaining);
}

void ASEEBossGrey::OnGuardKilled(AEnemyCharacter* Guard)
{
	GuardsRemaining = FMath::Max(0, GuardsRemaining - 1);
	OnGuardSquadChanged.Broadcast(GuardsRemaining);
}

// --- Damage with left-side weakness ---

float ASEEBossGrey::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = DamageAmount;

	// Apply left-side weakness if intel revealed and attack comes from left
	if (bLeftWeaknessRevealed && DamageCauser && IsAttackFromLeft(DamageCauser))
	{
		FinalDamage *= LeftSideDamageMultiplier;
	}

	return Super::TakeDamage(FinalDamage, DamageEvent, EventInstigator, DamageCauser);
}

bool ASEEBossGrey::IsAttackFromLeft(AActor* DamageCauser) const
{
	FVector ToAttacker = DamageCauser->GetActorLocation() - GetActorLocation();
	ToAttacker.Z = 0.f;
	ToAttacker.Normalize();

	FVector RightVector = GetActorRightVector();
	float Dot = FVector::DotProduct(ToAttacker, RightVector);

	// Negative dot = attacker is on our left side
	return Dot < -0.3f;
}

// --- Environment ---

void ASEEBossGrey::ActivateSpotlight()
{
	bSpotlightActive = true;
	// Blueprint-implementable: trigger spotlight effects
}

void ASEEBossGrey::DeactivateSpotlight()
{
	bSpotlightActive = false;
}

void ASEEBossGrey::TriggerSteamConduit()
{
	// Blueprint-implementable: activate steam VFX and damage volume
}

void ASEEBossGrey::DestroyCommEquipment()
{
	bCommEquipmentDestroyed = true;
	// Prevents reinforcement spawning in Phase 3
}

// --- Door Sequence ---

void ASEEBossGrey::BeginDoorSequence()
{
	bDoorSequenceActive = true;
	DoorSequenceTimer = DoorSequenceTimeLimit;
	OnDoorSequenceStarted.Broadcast();
}
