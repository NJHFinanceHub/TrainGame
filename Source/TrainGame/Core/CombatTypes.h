// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

// ============================================================================
// Combat System Type Definitions
// Snowpiercer: Eternal Engine - Advanced Combat System
// ============================================================================

/** The stance a combatant is currently in */
UENUM(BlueprintType)
enum class ECombatStance : uint8
{
	Neutral		UMETA(DisplayName = "Neutral"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Blocking	UMETA(DisplayName = "Blocking"),
	Dodging		UMETA(DisplayName = "Dodging"),
	Staggered	UMETA(DisplayName = "Staggered"),
	Downed		UMETA(DisplayName = "Downed"),
	KronoleMode	UMETA(DisplayName = "Kronole Mode"),
	Grappled	UMETA(DisplayName = "Grappled")
};

/** Attack direction for positional combat in tight corridors */
UENUM(BlueprintType)
enum class EAttackDirection : uint8
{
	High		UMETA(DisplayName = "High"),
	Mid			UMETA(DisplayName = "Mid"),
	Low			UMETA(DisplayName = "Low"),
	Left		UMETA(DisplayName = "Left"),
	Right		UMETA(DisplayName = "Right")
};

/** Block direction must match attack direction to fully mitigate */
UENUM(BlueprintType)
enum class EBlockDirection : uint8
{
	High		UMETA(DisplayName = "High"),
	Mid			UMETA(DisplayName = "Mid"),
	Low			UMETA(DisplayName = "Low")
};

/** Weapon categories */
UENUM(BlueprintType)
enum class EWeaponCategory : uint8
{
	Unarmed		UMETA(DisplayName = "Unarmed"),
	Blunt		UMETA(DisplayName = "Blunt"),
	Bladed		UMETA(DisplayName = "Bladed"),
	Piercing	UMETA(DisplayName = "Piercing"),
	Ranged		UMETA(DisplayName = "Ranged"),
	Thrown		UMETA(DisplayName = "Thrown"),
	Explosive	UMETA(DisplayName = "Explosive")
};

/** Weapon tier determines quality and durability */
UENUM(BlueprintType)
enum class EWeaponTier : uint8
{
	Improvised	UMETA(DisplayName = "Improvised"),   // Tail - breaks fast
	Functional	UMETA(DisplayName = "Functional"),   // Third Class
	Military	UMETA(DisplayName = "Military"),     // Jackboot armory
	Specialized	UMETA(DisplayName = "Specialized"),  // Quest/boss rewards
	Legendary	UMETA(DisplayName = "Legendary")     // One-of-a-kind
};

/** Environmental hazard types for environmental kills */
UENUM(BlueprintType)
enum class EEnvironmentalHazard : uint8
{
	None			UMETA(DisplayName = "None"),
	SteamVent		UMETA(DisplayName = "Steam Vent"),
	ElectricalPanel	UMETA(DisplayName = "Electrical Panel"),
	WindowBreach	UMETA(DisplayName = "Window Breach"),
	MachineryGear	UMETA(DisplayName = "Machinery Gear"),
	FlammableLeak	UMETA(DisplayName = "Flammable Leak"),
	PressureValve	UMETA(DisplayName = "Pressure Valve")
};

/** Damage types — physical subtypes + elemental */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Blunt			UMETA(DisplayName = "Blunt"),
	Bladed			UMETA(DisplayName = "Bladed"),
	Piercing		UMETA(DisplayName = "Piercing"),
	Fire			UMETA(DisplayName = "Fire"),
	Cold			UMETA(DisplayName = "Cold"),
	Electric		UMETA(DisplayName = "Electric"),
	Explosive		UMETA(DisplayName = "Explosive"),
	StealthTakedown	UMETA(DisplayName = "Stealth Takedown"),
	NonLethal		UMETA(DisplayName = "Non-Lethal")
};

/** Stealth takedown type */
UENUM(BlueprintType)
enum class ETakedownType : uint8
{
	GrabChokehold	UMETA(DisplayName = "Grab & Chokehold"),
	Knockout		UMETA(DisplayName = "Knockout"),
	LethalStab		UMETA(DisplayName = "Lethal Stab"),
	NeckSnap		UMETA(DisplayName = "Neck Snap"),
	EnvironmentalSlam	UMETA(DisplayName = "Environmental Slam")
};

/** Enemy type for distinct behavior and stats */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	TailFighter		UMETA(DisplayName = "Tail Fighter"),
	JackbootGrunt	UMETA(DisplayName = "Jackboot Grunt"),
	JackbootCaptain	UMETA(DisplayName = "Jackboot Captain"),
	OrderZealot		UMETA(DisplayName = "Order Zealot"),
	FirstClassGuard	UMETA(DisplayName = "First Class Guard"),
	Boss			UMETA(DisplayName = "Boss")
};

/** Boss phase state */
UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1		UMETA(DisplayName = "Phase 1"),
	Phase2		UMETA(DisplayName = "Phase 2"),
	Phase3		UMETA(DisplayName = "Phase 3"),
	Enraged		UMETA(DisplayName = "Enraged"),
	Defeated	UMETA(DisplayName = "Defeated")
};

/** Boss special attack type */
UENUM(BlueprintType)
enum class EBossAttackType : uint8
{
	Normal			UMETA(DisplayName = "Normal"),
	AreaOfEffect	UMETA(DisplayName = "Area of Effect"),
	Charge			UMETA(DisplayName = "Charge"),
	GrabThrow		UMETA(DisplayName = "Grab & Throw"),
	Summon			UMETA(DisplayName = "Summon Minions"),
	EnvironmentalTrigger	UMETA(DisplayName = "Environmental Trigger"),
	Unblockable		UMETA(DisplayName = "Unblockable")
};

// ============================================================================
// Structs
// ============================================================================

/** Result of a combat hit */
USTRUCT(BlueprintType)
struct FHitResult_Combat
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bHit = false;

	UPROPERTY(BlueprintReadOnly)
	bool bBlocked = false;

	UPROPERTY(BlueprintReadOnly)
	bool bDodged = false;

	UPROPERTY(BlueprintReadOnly)
	bool bCritical = false;

	UPROPERTY(BlueprintReadOnly)
	bool bEnvironmentalKill = false;

	UPROPERTY(BlueprintReadOnly)
	bool bStealthTakedown = false;

	UPROPERTY(BlueprintReadOnly)
	float DamageDealt = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float StaminaDrained = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float WeaponDurabilityLost = 0.f;

	UPROPERTY(BlueprintReadOnly)
	EAttackDirection AttackDirection = EAttackDirection::Mid;

	UPROPERTY(BlueprintReadOnly)
	EDamageType DamageType = EDamageType::Blunt;
};

/** Stamina/fatigue state snapshot */
USTRUCT(BlueprintType)
struct FStaminaState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaRegenRate = 8.f;

	/** Fatigue accumulates over extended combat, reducing max stamina temporarily */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FatigueLevel = 0.f;

	/** Max fatigue before forced rest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxFatigue = 100.f;

	/** How much fatigue reduces max stamina (multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FatigueStaminaPenalty = 0.5f;

	float GetEffectiveMaxStamina() const
	{
		return MaxStamina * (1.f - (FatigueLevel / MaxFatigue) * FatigueStaminaPenalty);
	}

	bool CanPerformAction(float StaminaCost) const
	{
		return CurrentStamina >= StaminaCost;
	}
};

/** Weapon stats for the degradation system */
USTRUCT(BlueprintType)
struct FWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WeaponName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponCategory Category = EWeaponCategory::Unarmed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponTier Tier = EWeaponTier::Improvised;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaCostPerSwing = 15.f;

	/** Current durability - weapon breaks at 0 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Durability = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDurability = 100.f;

	/** Durability lost per hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityLossPerHit = 2.f;

	/** Durability lost when blocked by an enemy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurabilityLossOnBlock = 5.f;

	/** Ammo count for ranged weapons (-1 = melee, no ammo needed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmo = -1;

	/** Max ammo capacity (-1 = melee) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = -1;

	/** Projectile speed for ranged weapons (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProjectileSpeed = 5000.f;

	/** Damage multiplier based on current durability */
	float GetDurabilityDamageMultiplier() const
	{
		if (MaxDurability <= 0.f) return 1.f;
		float Ratio = Durability / MaxDurability;
		// Below 25% durability, damage starts dropping
		if (Ratio < 0.25f)
		{
			return 0.5f + (Ratio / 0.25f) * 0.5f;
		}
		return 1.f;
	}

	float GetEffectiveDamage() const
	{
		return BaseDamage * GetDurabilityDamageMultiplier();
	}

	bool IsBroken() const
	{
		return Durability <= 0.f;
	}

	bool IsRanged() const
	{
		return Category == EWeaponCategory::Ranged || Category == EWeaponCategory::Thrown;
	}

	bool HasAmmo() const
	{
		return CurrentAmmo != 0; // -1 = unlimited (melee), >0 = has ammo
	}

	/** Get the damage type this weapon deals based on category */
	EDamageType GetDamageType() const
	{
		switch (Category)
		{
			case EWeaponCategory::Blunt:	return EDamageType::Blunt;
			case EWeaponCategory::Bladed:	return EDamageType::Bladed;
			case EWeaponCategory::Piercing:	return EDamageType::Piercing;
			case EWeaponCategory::Ranged:	return EDamageType::Piercing;
			case EWeaponCategory::Thrown:	return EDamageType::Blunt;
			case EWeaponCategory::Explosive:return EDamageType::Explosive;
			default:						return EDamageType::Blunt;
		}
	}
};

/** Enemy stats configuration for different enemy types */
USTRUCT(BlueprintType)
struct FEnemyConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType = EEnemyType::TailFighter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 600.f;

	/** Resistance to stagger (0 = normal, 1 = immune) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float StaggerResistance = 0.f;

	/** Damage resistances per type (0 = full damage, 1 = immune) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EDamageType, float> DamageResistances;

	/** Can this enemy be stealth-takedown'd? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeTakenDown = true;

	/** Default weapon loadout name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefaultWeapon = NAME_None;
};

/** Boss phase configuration */
USTRUCT(BlueprintType)
struct FBossPhaseConfig
{
	GENERATED_BODY()

	/** Health threshold to enter this phase (1.0 = full, 0.0 = dead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float HealthThreshold = 0.75f;

	/** Damage multiplier during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.f;

	/** Attack speed multiplier during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.f;

	/** Stagger resistance during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float StaggerResistance = 0.f;

	/** Special attacks available during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EBossAttackType> AvailableAttacks;

	/** Number of minions to summon (if Summon attack is available) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinionCount = 0;

	/** Whether boss can use environmental hazards in this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanUseEnvironment = false;

	/** Whether boss regenerates health in this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRegenPerSecond = 0.f;
};

/** Boss identity — data for a specific zone boss */
USTRUCT(BlueprintType)
struct FBossData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BossName = NAME_None;

	/** Which zone this boss rules (0-6 for 7 zones) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ZoneIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 25.f;

	/** Phases ordered by health threshold (highest first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBossPhaseConfig> Phases;

	/** Weapon the boss uses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FWeaponStats BossWeapon;

	/** Arena hazards the boss can trigger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EEnvironmentalHazard> ArenaHazards;
};
