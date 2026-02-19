// SurvivalTypes.h - Core type definitions for the survival resource system
#pragma once

#include "CoreMinimal.h"
#include "SurvivalTypes.generated.h"

// Threshold levels for survival stats that trigger gameplay effects
UENUM(BlueprintType)
enum class ESurvivalThreshold : uint8
{
	Critical	UMETA(DisplayName = "Critical"),    // 0-15%
	Low			UMETA(DisplayName = "Low"),          // 15-35%
	Normal		UMETA(DisplayName = "Normal"),       // 35-65%
	Good		UMETA(DisplayName = "Good"),         // 65-85%
	Excellent	UMETA(DisplayName = "Excellent")     // 85-100%
};

// Types of survival resources tracked by the system
UENUM(BlueprintType)
enum class ESurvivalStatType : uint8
{
	Hunger		UMETA(DisplayName = "Hunger"),
	Cold		UMETA(DisplayName = "Cold"),
	Morale		UMETA(DisplayName = "Morale"),
	Health		UMETA(DisplayName = "Health"),
	Stamina		UMETA(DisplayName = "Stamina")
};

// Kronole addiction stages
UENUM(BlueprintType)
enum class EKronoleAddictionStage : uint8
{
	Clean		UMETA(DisplayName = "Clean"),
	Casual		UMETA(DisplayName = "Casual User"),
	Dependent	UMETA(DisplayName = "Dependent"),
	Addicted	UMETA(DisplayName = "Addicted"),
	Terminal	UMETA(DisplayName = "Terminal")
};

// Kronole withdrawal severity
UENUM(BlueprintType)
enum class EWithdrawalSeverity : uint8
{
	None		UMETA(DisplayName = "None"),
	Mild		UMETA(DisplayName = "Mild"),
	Moderate	UMETA(DisplayName = "Moderate"),
	Severe		UMETA(DisplayName = "Severe"),
	Critical	UMETA(DisplayName = "Critical")
};

// Delegate for stat changes - broadcast when any survival stat changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSurvivalStatChanged,
	ESurvivalStatType, StatType,
	float, OldValue,
	float, NewValue
);

// Delegate for threshold crossings - fires when a stat enters a new threshold band
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnThresholdCrossed,
	ESurvivalStatType, StatType,
	ESurvivalThreshold, OldThreshold,
	ESurvivalThreshold, NewThreshold
);

// Delegate for Kronole addiction state changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAddictionStageChanged,
	EKronoleAddictionStage, OldStage,
	EKronoleAddictionStage, NewStage
);

// Configuration for a single survival stat's decay and effect curves
USTRUCT(BlueprintType)
struct FSurvivalStatConfig
{
	GENERATED_BODY()

	// Maximum value for this stat
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 100.0f;

	// Starting value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultValue = 100.0f;

	// Rate of passive decay per second (0 = no decay)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecayRatePerSecond = 0.0f;

	// Multiplier applied to stamina regeneration based on this stat's threshold
	// Indexed by ESurvivalThreshold: Critical=0, Low=1, Normal=2, Good=3, Excellent=4
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> StaminaRegenMultipliers;

	// Multiplier applied to combat effectiveness based on this stat's threshold
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> CombatEffectivenessMultipliers;

	FSurvivalStatConfig()
	{
		// Default multipliers: Critical(0.25), Low(0.5), Normal(1.0), Good(1.15), Excellent(1.3)
		StaminaRegenMultipliers = { 0.25f, 0.5f, 1.0f, 1.15f, 1.3f };
		CombatEffectivenessMultipliers = { 0.5f, 0.75f, 1.0f, 1.1f, 1.2f };
	}
};

// Snapshot of all survival stats for save/load
USTRUCT(BlueprintType)
struct FSurvivalSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	float Hunger = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float Cold = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float Morale = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float Stamina = 100.0f;

	UPROPERTY(BlueprintReadWrite)
	float KronoleAddictionLevel = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float TimeSinceLastKronoleDose = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	float Influence = 0.0f;

	UPROPERTY(BlueprintReadWrite)
	int32 Scrap = 0;
};
