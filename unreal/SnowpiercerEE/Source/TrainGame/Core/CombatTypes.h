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
	Physical		UMETA(DisplayName = "Physical"),
	Blunt			UMETA(DisplayName = "Blunt"),
	Bladed			UMETA(DisplayName = "Bladed"),
	Piercing		UMETA(DisplayName = "Piercing"),
	Fire			UMETA(DisplayName = "Fire"),
	Thermal			UMETA(DisplayName = "Thermal"),
	Electrical		UMETA(DisplayName = "Electrical"),
	Cold			UMETA(DisplayName = "Cold"),
	Explosive		UMETA(DisplayName = "Explosive"),
	StealthTakedown	UMETA(DisplayName = "Stealth Takedown"),
	NonLethal		UMETA(DisplayName = "Non-Lethal")
};

/** Enemy archetype on the train */
UENUM(BlueprintType)
enum class EEnemyType : uint8
{
	JackbootGrunt		UMETA(DisplayName = "Jackboot Grunt"),
	JackbootCaptain		UMETA(DisplayName = "Jackboot Captain"),
	OrderZealot			UMETA(DisplayName = "Order Zealot"),
	FirstClassGuard		UMETA(DisplayName = "First Class Guard"),
	TailScrapper		UMETA(DisplayName = "Tail Scrapper")
};

/** Projectile type for ranged weapons */
UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	CrossbowBolt	UMETA(DisplayName = "Crossbow Bolt"),
	ThrownObject	UMETA(DisplayName = "Thrown Object"),
	Bullet			UMETA(DisplayName = "Bullet"),
	TranqDart		UMETA(DisplayName = "Tranq Dart")
};

/** Boss fight health phase */
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
	AreaSlam		UMETA(DisplayName = "Area Slam"),
	ChargeRush		UMETA(DisplayName = "Charge Rush"),
	EnvironmentalTrigger	UMETA(DisplayName = "Environmental Trigger"),
	SummonMinions	UMETA(DisplayName = "Summon Minions"),
	RangedBarrage	UMETA(DisplayName = "Ranged Barrage"),
	GrabAttack		UMETA(DisplayName = "Grab Attack"),
	UnblockableStrike	UMETA(DisplayName = "Unblockable Strike")
};

/** Stealth takedown animation type */
UENUM(BlueprintType)
enum class ETakedownType : uint8
{
	GrabFromBehind		UMETA(DisplayName = "Grab From Behind"),
	Chokehold			UMETA(DisplayName = "Chokehold"),
	KnockoutPunch		UMETA(DisplayName = "Knockout Punch"),
	LethalNeckSnap		UMETA(DisplayName = "Lethal Neck Snap"),
	LethalKnifeKill		UMETA(DisplayName = "Lethal Knife Kill")
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

	/** Get the damage type based on weapon category */
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
		default:						return EDamageType::Physical;
		}
	}
};

/** Enemy stat block for archetype configuration */
USTRUCT(BlueprintType)
struct FEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType = EEnemyType::JackbootGrunt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovementSpeed = 600.f;

	/** Resistance multiplier per damage type (0 = immune, 1 = normal, >1 = weakness) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EDamageType, float> DamageResistances;

	/** Can be stealth-takedown'd */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeTakenDown = true;

	/** Strength required for non-lethal takedown (0 = anyone can) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownStrengthRequired = 0;

	/** Does this enemy call for reinforcements when alerted? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCallsReinforcements = false;

	/** Time before reinforcements arrive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReinforcementDelay = 10.f;
};

/** Boss special attack definition */
USTRUCT(BlueprintType)
struct FBossAttack
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AttackName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossAttackType AttackType = EBossAttackType::AreaSlam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 8.f;

	/** Which phases this attack is available in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EBossPhase> AvailableInPhases;

	/** Wind-up time before the attack hits (telegraph window for player) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindUpTime = 1.5f;

	/** Can this attack be blocked? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlockable = true;

	/** Can this attack be dodged? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDodgeable = true;

	float CooldownTimer = 0.f;

	bool IsReady() const { return CooldownTimer <= 0.f; }
};

/** Boss phase definition — health thresholds and behavior changes */
USTRUCT(BlueprintType)
struct FBossPhaseConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossPhase Phase = EBossPhase::Phase1;

	/** Health percentage at which this phase begins (1.0 = full, 0.0 = dead) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float HealthThreshold = 1.f;

	/** Damage multiplier during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.f;

	/** Attack speed multiplier during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedMultiplier = 1.f;

	/** Does the boss gain damage resistance in this phase? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float DamageResistance = 0.f;

	/** Environmental hazards activated during this phase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EEnvironmentalHazard> ActivatedHazards;
};
