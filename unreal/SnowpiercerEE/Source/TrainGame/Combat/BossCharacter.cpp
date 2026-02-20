// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BossCharacter.h"
#include "BossFightComponent.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"

ABossCharacter::ABossCharacter()
{
	BossComp = CreateDefaultSubobject<UBossFightComponent>(TEXT("BossFightComponent"));

	// Bosses cannot be stealth-takedown'd
	EnemyStats.bCanBeTakenDown = false;
}

void ABossCharacter::BeginPlay()
{
	// Skip EnemyCharacter's archetype initialization — bosses configure themselves
	ATrainGameCombatCharacter::BeginPlay();
	InitializeBoss(BossZone);
}

void ABossCharacter::InitializeBoss(EBossZone Zone)
{
	BossZone = Zone;

	switch (Zone)
	{
	case EBossZone::Zone1_Tail:			SetupTailWarden(); break;
	case EBossZone::Zone2_ThirdClass:	SetupThirdClassOverseer(); break;
	case EBossZone::Zone3_Agriculture:	SetupAgriculturalGuardian(); break;
	case EBossZone::Zone4_Industrial:	SetupIndustrialForeman(); break;
	case EBossZone::Zone5_Hospitality:	SetupHospitalityDirector(); break;
	case EBossZone::Zone6_FirstClass:	SetupFirstClassMarshal(); break;
	case EBossZone::Zone7_Engine:		SetupEngineGuardian(); break;
	}
}

void ABossCharacter::AddPhaseConfig(EBossPhase Phase, float HealthThreshold,
	float DamageMult, float SpeedMult, float Resistance)
{
	FBossPhaseConfig Config;
	Config.Phase = Phase;
	Config.HealthThreshold = HealthThreshold;
	Config.DamageMultiplier = DamageMult;
	Config.SpeedMultiplier = SpeedMult;
	Config.DamageResistance = Resistance;
	BossComp->PhaseConfigs.Add(Config);
}

void ABossCharacter::AddSpecialAttack(FName Name, EBossAttackType Type, float Damage,
	float Range, float Cooldown, float WindUp, bool bBlockable, bool bDodgeable,
	TArray<EBossPhase> Phases)
{
	FBossAttack Attack;
	Attack.AttackName = Name;
	Attack.AttackType = Type;
	Attack.Damage = Damage;
	Attack.Range = Range;
	Attack.Cooldown = Cooldown;
	Attack.WindUpTime = WindUp;
	Attack.bBlockable = bBlockable;
	Attack.bDodgeable = bDodgeable;
	Attack.AvailableInPhases = Phases;
	BossComp->SpecialAttacks.Add(Attack);
}

// ============================================================================
// Zone 1: Tail Warden — Brute enforcer, the Tail's oppressor
// ============================================================================
void ABossCharacter::SetupTailWarden()
{
	EnemyStats.Health = 500.f;
	EnemyStats.DamageMultiplier = 1.2f;
	EnemyStats.MovementSpeed = 400.f;

	// Heavy improvised weapon — reinforced pipe
	DefaultWeapon.WeaponName = FName("WardenHammer");
	DefaultWeapon.Category = EWeaponCategory::Blunt;
	DefaultWeapon.Tier = EWeaponTier::Military;
	DefaultWeapon.BaseDamage = 30.f;
	DefaultWeapon.AttackSpeed = 0.6f;
	DefaultWeapon.Range = 200.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f; // Bosses don't use stamina
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	// Phases
	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.1f);
	AddPhaseConfig(EBossPhase::Phase2, 0.6f, 1.3f, 1.1f, 0.2f);
	AddPhaseConfig(EBossPhase::Phase3, 0.3f, 1.5f, 1.3f, 0.0f); // Removes armor in desperation

	// Attacks
	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> LatePhases = { EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };

	AddSpecialAttack(FName("GroundSlam"), EBossAttackType::AreaSlam,
		40.f, 400.f, 10.f, 2.0f, false, true, AllPhases);
	AddSpecialAttack(FName("ChargeAttack"), EBossAttackType::ChargeRush,
		50.f, 800.f, 15.f, 1.5f, true, true, AllPhases);
	AddSpecialAttack(FName("SummonScrappers"), EBossAttackType::SummonMinions,
		0.f, 0.f, 30.f, 3.0f, false, false, LatePhases);

	// Resistant to blunt (heavy build), weak to bladed
	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Blunt, 0.5f);
		CombatComp->SetDamageResistance(EDamageType::Bladed, 1.3f);
	}
}

// ============================================================================
// Zone 2: Third Class Overseer — Controls car machinery
// ============================================================================
void ABossCharacter::SetupThirdClassOverseer()
{
	EnemyStats.Health = 400.f;
	EnemyStats.DamageMultiplier = 1.0f;
	EnemyStats.MovementSpeed = 500.f;

	DefaultWeapon.WeaponName = FName("OverseerRod");
	DefaultWeapon.Category = EWeaponCategory::Blunt;
	DefaultWeapon.Tier = EWeaponTier::Military;
	DefaultWeapon.BaseDamage = 22.f;
	DefaultWeapon.AttackSpeed = 0.9f;
	DefaultWeapon.Range = 180.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.0f);
	AddPhaseConfig(EBossPhase::Phase2, 0.5f, 1.2f, 1.2f, 0.15f);

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Enraged };
	TArray<EBossPhase> P2Only = { EBossPhase::Phase2, EBossPhase::Enraged };

	// Uses environment heavily
	AddSpecialAttack(FName("SteamBlast"), EBossAttackType::EnvironmentalTrigger,
		35.f, 500.f, 8.f, 1.0f, false, true, AllPhases);
	AddSpecialAttack(FName("MachineryTrap"), EBossAttackType::EnvironmentalTrigger,
		60.f, 300.f, 12.f, 2.5f, false, true, AllPhases);
	AddSpecialAttack(FName("WorkerSwarm"), EBossAttackType::SummonMinions,
		0.f, 0.f, 25.f, 2.0f, false, false, P2Only);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Thermal, 0.5f); // Used to heat
		CombatComp->SetDamageResistance(EDamageType::Cold, 1.5f);
	}
}

// ============================================================================
// Zone 3: Agricultural Guardian — Greenhouse protector
// ============================================================================
void ABossCharacter::SetupAgriculturalGuardian()
{
	EnemyStats.Health = 350.f;
	EnemyStats.DamageMultiplier = 1.1f;
	EnemyStats.MovementSpeed = 550.f;

	DefaultWeapon.WeaponName = FName("PruningScythe");
	DefaultWeapon.Category = EWeaponCategory::Bladed;
	DefaultWeapon.Tier = EWeaponTier::Specialized;
	DefaultWeapon.BaseDamage = 28.f;
	DefaultWeapon.AttackSpeed = 1.1f;
	DefaultWeapon.Range = 220.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.0f);
	AddPhaseConfig(EBossPhase::Phase2, 0.5f, 1.4f, 1.3f, 0.1f);

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Enraged };

	AddSpecialAttack(FName("ScytheWhirlwind"), EBossAttackType::AreaSlam,
		45.f, 300.f, 10.f, 1.5f, true, true, AllPhases);
	AddSpecialAttack(FName("PoisonCloud"), EBossAttackType::EnvironmentalTrigger,
		15.f, 600.f, 15.f, 2.0f, false, true, AllPhases);
	AddSpecialAttack(FName("VineGrab"), EBossAttackType::GrabAttack,
		25.f, 400.f, 12.f, 1.0f, false, true, AllPhases);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Bladed, 0.7f); // Thick skin
		CombatComp->SetDamageResistance(EDamageType::Fire, 2.0f); // Very flammable environment
	}
}

// ============================================================================
// Zone 4: Industrial Foreman — Forge master, fire attacks
// ============================================================================
void ABossCharacter::SetupIndustrialForeman()
{
	EnemyStats.Health = 600.f;
	EnemyStats.DamageMultiplier = 1.3f;
	EnemyStats.MovementSpeed = 350.f;

	DefaultWeapon.WeaponName = FName("ForgeHammer");
	DefaultWeapon.Category = EWeaponCategory::Blunt;
	DefaultWeapon.Tier = EWeaponTier::Specialized;
	DefaultWeapon.BaseDamage = 35.f;
	DefaultWeapon.AttackSpeed = 0.5f;
	DefaultWeapon.Range = 190.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.25f); // Heavy armor from start
	AddPhaseConfig(EBossPhase::Phase2, 0.6f, 1.2f, 1.0f, 0.3f);
	AddPhaseConfig(EBossPhase::Phase3, 0.3f, 1.5f, 0.8f, 0.4f); // Slow but very tanky

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> LatePhases = { EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };

	AddSpecialAttack(FName("MoltenSplash"), EBossAttackType::AreaSlam,
		50.f, 350.f, 12.f, 2.5f, false, true, AllPhases);
	AddSpecialAttack(FName("AnvilStrike"), EBossAttackType::UnblockableStrike,
		70.f, 200.f, 18.f, 3.0f, false, true, LatePhases);
	AddSpecialAttack(FName("ForgeFlameWall"), EBossAttackType::EnvironmentalTrigger,
		30.f, 800.f, 20.f, 2.0f, false, false, AllPhases);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Fire, 0.2f); // Nearly immune to fire
		CombatComp->SetDamageResistance(EDamageType::Thermal, 0.3f);
		CombatComp->SetDamageResistance(EDamageType::Blunt, 0.6f);
		CombatComp->SetDamageResistance(EDamageType::Cold, 1.8f); // Weakness
		CombatComp->SetDamageResistance(EDamageType::Electrical, 1.5f);
	}
}

// ============================================================================
// Zone 5: Hospitality Director — Psychological warfare, minion waves
// ============================================================================
void ABossCharacter::SetupHospitalityDirector()
{
	EnemyStats.Health = 300.f;
	EnemyStats.DamageMultiplier = 0.8f;
	EnemyStats.MovementSpeed = 600.f;

	DefaultWeapon.WeaponName = FName("ConcealedBlade");
	DefaultWeapon.Category = EWeaponCategory::Bladed;
	DefaultWeapon.Tier = EWeaponTier::Specialized;
	DefaultWeapon.BaseDamage = 20.f;
	DefaultWeapon.AttackSpeed = 1.5f;
	DefaultWeapon.Range = 140.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.0f);
	AddPhaseConfig(EBossPhase::Phase2, 0.5f, 1.5f, 1.5f, 0.0f); // Fast and deadly

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Enraged };

	// Relies on minions and misdirection
	AddSpecialAttack(FName("SummonGuards"), EBossAttackType::SummonMinions,
		0.f, 0.f, 20.f, 1.5f, false, false, AllPhases);
	AddSpecialAttack(FName("PoisonDart"), EBossAttackType::RangedBarrage,
		25.f, 1000.f, 8.f, 0.5f, true, true, AllPhases);
	AddSpecialAttack(FName("BackstabLunge"), EBossAttackType::ChargeRush,
		45.f, 500.f, 12.f, 0.8f, true, true, AllPhases);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::NonLethal, 0.5f); // Hard to knockout
	}
}

// ============================================================================
// Zone 6: First Class Marshal — Elite combat specialist
// ============================================================================
void ABossCharacter::SetupFirstClassMarshal()
{
	EnemyStats.Health = 500.f;
	EnemyStats.DamageMultiplier = 1.4f;
	EnemyStats.MovementSpeed = 550.f;

	DefaultWeapon.WeaponName = FName("MarshalSabre");
	DefaultWeapon.Category = EWeaponCategory::Bladed;
	DefaultWeapon.Tier = EWeaponTier::Legendary;
	DefaultWeapon.BaseDamage = 32.f;
	DefaultWeapon.AttackSpeed = 1.2f;
	DefaultWeapon.Range = 180.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.15f);
	AddPhaseConfig(EBossPhase::Phase2, 0.6f, 1.3f, 1.2f, 0.2f);
	AddPhaseConfig(EBossPhase::Phase3, 0.25f, 1.6f, 1.4f, 0.1f);

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> LatePhases = { EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> FinalPhase = { EBossPhase::Phase3, EBossPhase::Enraged };

	AddSpecialAttack(FName("SabreFlurry"), EBossAttackType::UnblockableStrike,
		40.f, 200.f, 10.f, 1.2f, false, true, AllPhases);
	AddSpecialAttack(FName("PistolBarrage"), EBossAttackType::RangedBarrage,
		30.f, 1500.f, 6.f, 0.8f, false, true, AllPhases);
	AddSpecialAttack(FName("ExecutionStrike"), EBossAttackType::GrabAttack,
		80.f, 150.f, 20.f, 2.0f, false, true, FinalPhase);
	AddSpecialAttack(FName("EliteGuardWave"), EBossAttackType::SummonMinions,
		0.f, 0.f, 30.f, 2.0f, false, false, LatePhases);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Bladed, 0.7f); // Knows how to counter
		CombatComp->SetDamageResistance(EDamageType::Piercing, 0.7f);
		CombatComp->SetDamageResistance(EDamageType::Blunt, 1.3f); // Less used to crude weapons
	}
}

// ============================================================================
// Zone 7: Engine Guardian — Controls the Eternal Engine itself
// ============================================================================
void ABossCharacter::SetupEngineGuardian()
{
	EnemyStats.Health = 800.f;
	EnemyStats.DamageMultiplier = 1.5f;
	EnemyStats.MovementSpeed = 300.f;

	DefaultWeapon.WeaponName = FName("EngineTool");
	DefaultWeapon.Category = EWeaponCategory::Blunt;
	DefaultWeapon.Tier = EWeaponTier::Legendary;
	DefaultWeapon.BaseDamage = 40.f;
	DefaultWeapon.AttackSpeed = 0.4f;
	DefaultWeapon.Range = 250.f;
	DefaultWeapon.StaminaCostPerSwing = 0.f;
	DefaultWeapon.Durability = 9999.f;
	DefaultWeapon.MaxDurability = 9999.f;

	AddPhaseConfig(EBossPhase::Phase1, 1.0f, 1.0f, 1.0f, 0.3f); // Heavy armor
	AddPhaseConfig(EBossPhase::Phase2, 0.7f, 1.2f, 1.0f, 0.35f);
	AddPhaseConfig(EBossPhase::Phase3, 0.4f, 1.5f, 1.2f, 0.2f); // Armor breaks

	TArray<EBossPhase> AllPhases = { EBossPhase::Phase1, EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> LatePhases = { EBossPhase::Phase2, EBossPhase::Phase3, EBossPhase::Enraged };
	TArray<EBossPhase> FinalPhase = { EBossPhase::Phase3, EBossPhase::Enraged };

	// Controls the train's systems as weapons
	AddSpecialAttack(FName("ElectricalOverload"), EBossAttackType::EnvironmentalTrigger,
		60.f, 600.f, 12.f, 2.0f, false, true, AllPhases);
	AddSpecialAttack(FName("ColdVent"), EBossAttackType::AreaSlam,
		40.f, 500.f, 10.f, 1.5f, false, true, AllPhases);
	AddSpecialAttack(FName("PressureSurge"), EBossAttackType::ChargeRush,
		55.f, 400.f, 15.f, 2.5f, false, true, LatePhases);
	AddSpecialAttack(FName("EngineOverride"), EBossAttackType::EnvironmentalTrigger,
		100.f, 1000.f, 30.f, 4.0f, false, true, FinalPhase);

	if (CombatComp)
	{
		CombatComp->SetDamageResistance(EDamageType::Electrical, 0.1f); // Nearly immune
		CombatComp->SetDamageResistance(EDamageType::Cold, 0.3f); // Engine insulation
		CombatComp->SetDamageResistance(EDamageType::Fire, 1.5f); // Overheating vulnerability
		CombatComp->SetDamageResistance(EDamageType::Bladed, 1.3f);
	}
}
