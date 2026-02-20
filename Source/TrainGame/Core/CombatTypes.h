// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.generated.h"

// ============================================================================
// Combat System Type Definitions
// Snowpiercer: Eternal Engine - Combat System Prototype
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
	KronoleMode	UMETA(DisplayName = "Kronole Mode")
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

/** Damage types for the combat system */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Physical	UMETA(DisplayName = "Physical"),
	Thermal		UMETA(DisplayName = "Thermal"),
	Electrical	UMETA(DisplayName = "Electrical"),
	Cold			UMETA(DisplayName = "Cold"),
	Explosive		UMETA(DisplayName = "Explosive"),
	StealthTakedown	UMETA(DisplayName = "Stealth Takedown"),
	NonLethal		UMETA(DisplayName = "Non-Lethal"),
	Blunt		UMETA(DisplayName = "Blunt"),
	Bladed		UMETA(DisplayName = "Bladed"),
	Piercing	UMETA(DisplayName = "Piercing"),
	Fire		UMETA(DisplayName = "Fire"),
	Electric	UMETA(DisplayName = "Electric")
};

/** Enemy class types in Snowpiercer */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	JackbootGrunt		UMETA(DisplayName = "Jackboot Grunt"),
	JackbootCaptain		UMETA(DisplayName = "Jackboot Captain"),
	OrderZealot			UMETA(DisplayName = "Order Zealot"),
	FirstClassGuard		UMETA(DisplayName = "First Class Guard"),
	TailFighter			UMETA(DisplayName = "Tail Fighter"),
	KronoleAddict		UMETA(DisplayName = "Kronole Addict"),
	Breachman			UMETA(DisplayName = "Breachman")
};

/** Stealth takedown type */
UENUM(BlueprintType)
enum class EStealthTakedownType : uint8
{
	GrabFromBehind	UMETA(DisplayName = "Grab From Behind"),
	Chokehold		UMETA(DisplayName = "Chokehold"),
	Knockout		UMETA(DisplayName = "Knockout"),
	SilentKill		UMETA(DisplayName = "Silent Kill")
};

/** Boss health phase */
UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1		UMETA(DisplayName = "Phase 1"),
	Phase2		UMETA(DisplayName = "Phase 2"),
	Phase3		UMETA(DisplayName = "Phase 3"),
	Enraged		UMETA(DisplayName = "Enraged"),
	Defeated	UMETA(DisplayName = "Defeated")
};

/** Boss identity — one per zone */
UENUM(BlueprintType)
enum class EBossIdentity : uint8
{
	ThePitChampion		UMETA(DisplayName = "The Pit Champion"),      // Zone 1: Tail
	ChefPierrot			UMETA(DisplayName = "Chef Pierrot"),           // Zone 2: Third Class
	JackbootCommander	UMETA(DisplayName = "Jackboot Commander"),     // Zone 3: Security
	KronoleLord			UMETA(DisplayName = "Kronole Lord"),           // Zone 4: Night Car
	FirstClassDuelist	UMETA(DisplayName = "First Class Duelist"),    // Zone 5: First Class
	OrderHighPriest		UMETA(DisplayName = "Order High Priest"),      // Zone 6: Engine Shrine
	MrWilford			UMETA(DisplayName = "Mr. Wilford")             // Zone 7: Engine
};

/** Projectile type for ranged weapons */
UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Bolt		UMETA(DisplayName = "Crossbow Bolt"),
	Bullet		UMETA(DisplayName = "Improvised Bullet"),
	ThrownKnife	UMETA(DisplayName = "Thrown Knife"),
	ThrownObject	UMETA(DisplayName = "Thrown Object"),
	Explosive	UMETA(DisplayName = "Explosive Charge")
};

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
	bool bStealthKill = false;

	UPROPERTY(BlueprintReadOnly)
	float DamageDealt = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float StaminaDrained = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float WeaponDurabilityLost = 0.f;

	UPROPERTY(BlueprintReadOnly)
	EAttackDirection AttackDirection = EAttackDirection::Mid;

	UPROPERTY(BlueprintReadOnly)
	EDamageType DamageType = EDamageType::Physical;
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

	/** Ammo count for ranged weapons (-1 = not ranged) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AmmoCount = -1;

	/** Max ammo capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = -1;

	/** Projectile type for ranged weapons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EProjectileType ProjectileType = EProjectileType::Bolt;

	/** Damage multiplier based on current durability */
	float GetDurabilityDamageMultiplier() const
	{
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
		return AmmoCount != 0;
	}
};

/** Enemy stats configuration */
USTRUCT(BlueprintType)
struct FEnemyConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType = EEnemyType::JackbootGrunt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DetectionRange = 1500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackRange = 180.f;

	/** Can this enemy be stealth-killed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeStealthKilled = true;

	/** Resistance to stealth takedowns (0 = always succeeds, 1 = immune) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StealthResistance = 0.f;

	/** Armor values per damage type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EDamageType, float> ArmorValues;
};

/** Boss phase configuration */
USTRUCT(BlueprintType)
struct FBossPhaseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossPhase Phase = EBossPhase::Phase1;

	/** Health percentage threshold to enter this phase (1.0 = full, 0.0 = death) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float HealthThreshold = 1.0f;

	/** Damage multiplier during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	/** Attack speed multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.0f;

	/** Special attack cooldown override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpecialAttackCooldown = 10.f;

	/** Can adds be spawned in this phase? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpawnsAdds = false;

	/** Number of adds to spawn on phase transition */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bSpawnsAdds"))
	int32 AddCount = 0;
};
