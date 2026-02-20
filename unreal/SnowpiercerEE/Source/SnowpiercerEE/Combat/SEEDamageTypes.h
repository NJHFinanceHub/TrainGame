// SEEDamageTypes.h
// Snowpiercer: Eternal Engine - Damage types, structs, and armor reduction

#pragma once

#include "CoreMinimal.h"
#include "SEEDamageTypes.generated.h"

/** Categories of damage */
UENUM(BlueprintType)
enum class ESEEDamageType : uint8
{
	Physical    UMETA(DisplayName = "Physical"),
	Fire        UMETA(DisplayName = "Fire"),
	Cold        UMETA(DisplayName = "Cold"),
	Electrical  UMETA(DisplayName = "Electrical"),
	Chemical    UMETA(DisplayName = "Chemical")
};

/** Attack direction for blocking */
UENUM(BlueprintType)
enum class ESEEAttackDirection : uint8
{
	High    UMETA(DisplayName = "High"),
	Mid     UMETA(DisplayName = "Mid"),
	Low     UMETA(DisplayName = "Low")
};

/** Result of a combat hit */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float FinalDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	ESEEDamageType DamageType = ESEEDamageType::Physical;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	ESEEAttackDirection Direction = ESEEAttackDirection::Mid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bWasBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	bool bWasCritical = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	AActor* Instigator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	AActor* DamageCauser = nullptr;
};

/** Armor stats for damage reduction */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEArmorData
{
	GENERATED_BODY()

	/** Flat physical damage reduction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	float PhysicalReduction = 0.0f;

	/** Percentage damage reduction (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	float DamageReductionPercent = 0.0f;

	/** Per-type resistance modifiers (multiplied with incoming damage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
	TMap<ESEEDamageType, float> TypeResistances;
};
