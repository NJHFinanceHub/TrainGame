// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DifficultyTypes.generated.h"

// ============================================================================
// Difficulty System Type Definitions
// 4 tiers themed around the train's social hierarchy
// ============================================================================

UENUM(BlueprintType)
enum class EDifficultyTier : uint8
{
	Passenger		UMETA(DisplayName = "Passenger"),		// Story-focused, forgiving
	Survivor		UMETA(DisplayName = "Survivor"),		// Intended experience
	EternalEngine	UMETA(DisplayName = "Eternal Engine"),	// Hardcore
	MrWilford		UMETA(DisplayName = "Mr. Wilford")		// Permadeath
};

UENUM(BlueprintType)
enum class EAdaptiveSubTier : uint8
{
	Easy	UMETA(DisplayName = "Easy"),
	Normal	UMETA(DisplayName = "Normal"),
	Hard	UMETA(DisplayName = "Hard")
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifficultyChanged, EDifficultyTier, OldTier, EDifficultyTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAdaptiveSubTierChanged, EAdaptiveSubTier, OldSubTier, EAdaptiveSubTier, NewSubTier);

/** Numeric modifiers for a difficulty tier. All values are multipliers (1.0 = baseline). */
USTRUCT(BlueprintType)
struct FDifficultyModifiers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float EnemyHealthMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float EnemyDamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float EnemyDetectionRangeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float EnemyDetectionSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	float ResourceDropRateMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	float ResourceDegradationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
	float ColdExposureTimeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float StaminaDrainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float XPGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	float CompanionDownTimerSeconds = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	float FallDamageMultiplier = 1.0f;

	/** -2, -1, 0, +1, +2: shift to injury severity tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 InjurySeverityShift = 0;

	/** Bonus/penalty to contested stat checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 StatCheckModifier = 0;

	/** Fraction of resources lost on death (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	float DeathResourceLossFraction = 0.1f;

	/** Whether companions can permanently die */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	bool bCompanionPermadeath = true;

	/** Whether death is permanent (Mr. Wilford only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	bool bPermadeath = false;

	/** Whether friendly fire is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bFriendlyFire = false;

	/** Friendly fire damage multiplier (if enabled) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FriendlyFireDamageMultiplier = 0.f;

	/** Whether quest markers are shown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bShowQuestMarkers = true;

	/** Whether saving is restricted to rest points / zone transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bRestrictedSaving = false;
};

/** Complete difficulty tier configuration */
USTRUCT(BlueprintType)
struct FDifficultyTierConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDifficultyTier Tier = EDifficultyTier::Survivor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDifficultyModifiers Modifiers;
};

/** Adaptive difficulty performance tracking */
USTRUCT(BlueprintType)
struct FAdaptivePerformanceData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 PerformanceScore = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 DeathCount = 0;

	UPROPERTY(BlueprintReadOnly)
	float AvgCombatTime = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float AvgHealthAtCombatEnd = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float ResourcePercent = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float DetectionRate = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float TimeSinceLastHeal = 0.f;

	void Reset()
	{
		PerformanceScore = 0;
		DeathCount = 0;
		AvgCombatTime = 0.f;
		AvgHealthAtCombatEnd = 0.f;
		ResourcePercent = 0.5f;
		DetectionRate = 0.f;
		TimeSinceLastHeal = 0.f;
	}
};
