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
	Cold		UMETA(DisplayName = "Cold"),
	Explosive	UMETA(DisplayName = "Explosive")
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
};
