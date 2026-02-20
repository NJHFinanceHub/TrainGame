// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EnemyCharacter.h"
#include "CombatComponent.h"
#include "BossFightComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	BossComp = CreateDefaultSubobject<UBossFightComponent>(TEXT("BossFightComponent"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyConfig(EnemyConfig);
}

void AEnemyCharacter::ApplyConfig(const FEnemyConfig& Config)
{
	EnemyConfig = Config;

	if (CombatComp)
	{
		CombatComp->SetMaxHealth(Config.MaxHealth);
		CombatComp->Heal(Config.MaxHealth);
		CombatComp->SetStaggerResistance(Config.StaggerResistance);

		// Apply damage resistances
		for (const auto& Pair : Config.DamageResistances)
		{
			CombatComp->SetDamageResistance(Pair.Key, Pair.Value);
		}
	}

	EquipDefaultWeapon();
}

void AEnemyCharacter::InitAsBoss(const FBossData& InBossData)
{
	if (BossComp && CombatComp)
	{
		// Configure combat component for boss
		CombatComp->SetMaxHealth(InBossData.MaxHealth);
		CombatComp->Heal(InBossData.MaxHealth);

		// Bosses can't be stealth-takedown'd
		FEnemyConfig BossConfig;
		BossConfig.EnemyType = EEnemyType::Boss;
		BossConfig.MaxHealth = InBossData.MaxHealth;
		BossConfig.bCanBeTakenDown = false;
		BossConfig.StaggerResistance = 0.5f;
		EnemyConfig = BossConfig;

		// Equip boss weapon
		if (WeaponComp && InBossData.BossWeapon.WeaponName != NAME_None)
		{
			WeaponComp->EquipWeapon(InBossData.BossWeapon);
		}

		// Initialize boss fight component
		BossComp->InitBoss(InBossData);
	}
}

void AEnemyCharacter::EquipDefaultWeapon()
{
	if (!WeaponComp) return;

	FName WeaponName = EnemyConfig.DefaultWeapon;
	if (WeaponName == NAME_None)
	{
		// Auto-assign based on enemy type
		switch (EnemyConfig.EnemyType)
		{
			case EEnemyType::TailFighter:
				WeaponComp->EquipWeapon(UWeaponComponent::MakePipeClub());
				break;
			case EEnemyType::JackbootGrunt:
				WeaponComp->EquipWeapon(UWeaponComponent::MakeJackbootBaton());
				break;
			case EEnemyType::JackbootCaptain:
				WeaponComp->EquipWeapon(UWeaponComponent::MakeOfficerSword());
				break;
			case EEnemyType::OrderZealot:
				WeaponComp->EquipWeapon(UWeaponComponent::MakeZealotBlade());
				break;
			case EEnemyType::FirstClassGuard:
				WeaponComp->EquipWeapon(UWeaponComponent::MakeElectricProd());
				break;
			default:
				break;
		}
	}
	else
	{
		// Look up named weapon presets
		if (WeaponName == "PipeClub") WeaponComp->EquipWeapon(UWeaponComponent::MakePipeClub());
		else if (WeaponName == "Shiv") WeaponComp->EquipWeapon(UWeaponComponent::MakeShiv());
		else if (WeaponName == "NailBat") WeaponComp->EquipWeapon(UWeaponComponent::MakeNailBat());
		else if (WeaponName == "ReinforcedAxe") WeaponComp->EquipWeapon(UWeaponComponent::MakeReinforcedAxe());
		else if (WeaponName == "JackbootBaton") WeaponComp->EquipWeapon(UWeaponComponent::MakeJackbootBaton());
		else if (WeaponName == "Crossbow") WeaponComp->EquipWeapon(UWeaponComponent::MakeCrossbow());
		else if (WeaponName == "PipeGun") WeaponComp->EquipWeapon(UWeaponComponent::MakeImprovisedFirearm());
		else if (WeaponName == "OfficerSword") WeaponComp->EquipWeapon(UWeaponComponent::MakeOfficerSword());
		else if (WeaponName == "ZealotBlade") WeaponComp->EquipWeapon(UWeaponComponent::MakeZealotBlade());
		else if (WeaponName == "ElectricProd") WeaponComp->EquipWeapon(UWeaponComponent::MakeElectricProd());
		else if (WeaponName == "ThrowingKnife") WeaponComp->EquipWeapon(UWeaponComponent::MakeThrowingKnife());
		else if (WeaponName == "FirstClassPistol") WeaponComp->EquipWeapon(UWeaponComponent::MakeFirstClassPistol());
	}
}

// ============================================================================
// Preset Enemy Configurations
// ============================================================================

FEnemyConfig AEnemyCharacter::MakeTailFighterConfig()
{
	FEnemyConfig Config;
	Config.EnemyType = EEnemyType::TailFighter;
	Config.MaxHealth = 60.f;
	Config.BaseDamageMultiplier = 0.8f;
	Config.MoveSpeed = 550.f;
	Config.StaggerResistance = 0.f;
	Config.bCanBeTakenDown = true;
	Config.DefaultWeapon = FName("PipeClub");
	return Config;
}

FEnemyConfig AEnemyCharacter::MakeJackbootGruntConfig()
{
	FEnemyConfig Config;
	Config.EnemyType = EEnemyType::JackbootGrunt;
	Config.MaxHealth = 100.f;
	Config.BaseDamageMultiplier = 1.f;
	Config.MoveSpeed = 600.f;
	Config.StaggerResistance = 0.2f;
	Config.bCanBeTakenDown = true;
	Config.DefaultWeapon = FName("JackbootBaton");
	// Jackboots have slight blunt resistance (armor)
	Config.DamageResistances.Add(EDamageType::Blunt, 0.15f);
	return Config;
}

FEnemyConfig AEnemyCharacter::MakeJackbootCaptainConfig()
{
	FEnemyConfig Config;
	Config.EnemyType = EEnemyType::JackbootCaptain;
	Config.MaxHealth = 180.f;
	Config.BaseDamageMultiplier = 1.3f;
	Config.MoveSpeed = 550.f;
	Config.StaggerResistance = 0.4f;
	Config.bCanBeTakenDown = true;
	Config.DefaultWeapon = FName("OfficerSword");
	// Officers have better armor
	Config.DamageResistances.Add(EDamageType::Blunt, 0.2f);
	Config.DamageResistances.Add(EDamageType::Piercing, 0.15f);
	return Config;
}

FEnemyConfig AEnemyCharacter::MakeOrderZealotConfig()
{
	FEnemyConfig Config;
	Config.EnemyType = EEnemyType::OrderZealot;
	Config.MaxHealth = 80.f; // Less health, but relentless
	Config.BaseDamageMultiplier = 1.5f; // Hit hard
	Config.MoveSpeed = 650.f; // Fast
	Config.StaggerResistance = 0.3f;
	Config.bCanBeTakenDown = true;
	Config.DefaultWeapon = FName("ZealotBlade");
	// Zealots resist fire (religious fervor, willpower)
	Config.DamageResistances.Add(EDamageType::Fire, 0.3f);
	return Config;
}

FEnemyConfig AEnemyCharacter::MakeFirstClassGuardConfig()
{
	FEnemyConfig Config;
	Config.EnemyType = EEnemyType::FirstClassGuard;
	Config.MaxHealth = 140.f;
	Config.BaseDamageMultiplier = 1.2f;
	Config.MoveSpeed = 500.f; // Methodical, not fast
	Config.StaggerResistance = 0.35f;
	Config.bCanBeTakenDown = true;
	Config.DefaultWeapon = FName("ElectricProd");
	// First Class guards have excellent armor
	Config.DamageResistances.Add(EDamageType::Blunt, 0.25f);
	Config.DamageResistances.Add(EDamageType::Bladed, 0.2f);
	Config.DamageResistances.Add(EDamageType::Electric, 0.5f); // Insulated
	return Config;
}
