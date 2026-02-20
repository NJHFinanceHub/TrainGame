// SEEWeaponBase.h
// Snowpiercer: Eternal Engine - Base weapon actor with tiers, durability, degradation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEDamageTypes.h"
#include "SEEWeaponBase.generated.h"

/** Weapon tier (higher = rarer and more powerful) */
UENUM(BlueprintType)
enum class ESEEWeaponTier : uint8
{
	Tier1_Improvised  UMETA(DisplayName = "Tier 1 - Improvised"),
	Tier2_Common      UMETA(DisplayName = "Tier 2 - Common"),
	Tier3_Military    UMETA(DisplayName = "Tier 3 - Military"),
	Tier4_Elite       UMETA(DisplayName = "Tier 4 - Elite"),
	Tier5_Legendary   UMETA(DisplayName = "Tier 5 - Legendary")
};

/** Weapon category */
UENUM(BlueprintType)
enum class ESEEWeaponCategory : uint8
{
	Melee_Light    UMETA(DisplayName = "Light Melee"),
	Melee_Heavy    UMETA(DisplayName = "Heavy Melee"),
	Ranged_Pistol  UMETA(DisplayName = "Pistol"),
	Ranged_Rifle   UMETA(DisplayName = "Rifle"),
	Ranged_Thrown  UMETA(DisplayName = "Thrown")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponDurabilityChanged, float, NewDurability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponBroken);

/**
 * Base weapon actor. Holds damage stats, tier, durability, and degradation.
 * Attach to character via combat component weapon slots.
 */
UCLASS(Blueprintable)
class SNOWPIERCEREE_API ASEEWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASEEWeaponBase();

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float CriticalChance = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float CriticalMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	float StaminaCostPerAttack = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Stats")
	ESEEDamageType DamageType = ESEEDamageType::Physical;

	// --- Classification ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ESEEWeaponTier Tier = ESEEWeaponTier::Tier1_Improvised;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	ESEEWeaponCategory Category = ESEEWeaponCategory::Melee_Light;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName = NAME_None;

	// --- Durability ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Durability")
	float MaxDurability = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Weapon|Durability")
	float CurrentDurability = 100.0f;

	/** Durability lost per hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Durability")
	float DegradationPerHit = 1.0f;

	/** Damage multiplier when durability is low (at 0 durability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Durability")
	float BrokenDamageMultiplier = 0.25f;

	// --- Interface ---

	/** Get effective damage accounting for durability degradation */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetEffectiveDamage() const;

	/** Get durability as 0-1 ratio */
	UFUNCTION(BlueprintPure, Category = "Weapon|Durability")
	float GetDurabilityPercent() const;

	/** Apply degradation from a hit */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Durability")
	void ApplyDegradation(float Multiplier = 1.0f);

	/** Repair weapon by amount */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Durability")
	void Repair(float Amount);

	/** Check if weapon is broken */
	UFUNCTION(BlueprintPure, Category = "Weapon|Durability")
	bool IsBroken() const;

	/** Check if this is a melee weapon */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsMelee() const;

	/** Check if this is a ranged weapon */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsRanged() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponDurabilityChanged OnDurabilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon|Events")
	FOnWeaponBroken OnWeaponBroken;
};
