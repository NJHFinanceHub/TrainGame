// SurvivalComponent.h - Actor component managing all survival stats for a character
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalTypes.h"
#include "SurvivalComponent.generated.h"

/**
 * USurvivalComponent
 *
 * Manages the core survival stats: Hunger, Cold, Morale, Health, Stamina.
 * Attach to the player character (or companions) to track resource depletion,
 * apply gameplay modifiers, and broadcast threshold events for UI/AI.
 *
 * Hunger decays over time; when low, stamina regen slows and combat effectiveness drops.
 * Cold is driven by environment (hull breaches, exterior); increases debuffs when high exposure.
 * Morale is affected by events, companion deaths, moral choices; affects companion AI and dialogue.
 * Health/Stamina are combat resources modified by Hunger/Cold/Morale states.
 */
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class SNOWYENGINE_API USurvivalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USurvivalComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Stat Access ---

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetStatValue(ESurvivalStatType StatType) const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	float GetStatPercent(ESurvivalStatType StatType) const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	ESurvivalThreshold GetStatThreshold(ESurvivalStatType StatType) const;

	// --- Stat Modification ---

	/** Add (positive) or subtract (negative) from a survival stat, clamped to [0, Max]. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void ModifyStat(ESurvivalStatType StatType, float Delta);

	/** Set a stat to an absolute value. */
	UFUNCTION(BlueprintCallable, Category = "Survival")
	void SetStat(ESurvivalStatType StatType, float NewValue);

	// --- Derived Modifiers ---

	/** Get the combined stamina regen multiplier from all survival stats. */
	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetStaminaRegenMultiplier() const;

	/** Get the combined combat effectiveness multiplier from all survival stats. */
	UFUNCTION(BlueprintPure, Category = "Survival")
	float GetCombatEffectivenessMultiplier() const;

	// --- Cold System ---

	/** Set the ambient cold level of the current environment (0=warm, 1=freezing). */
	UFUNCTION(BlueprintCallable, Category = "Survival|Cold")
	void SetEnvironmentColdLevel(float ColdLevel);

	/** Returns true if the character has cold protection (e.g., cold suit equipped). */
	UFUNCTION(BlueprintPure, Category = "Survival|Cold")
	bool HasColdProtection() const;

	UFUNCTION(BlueprintCallable, Category = "Survival|Cold")
	void SetColdProtection(bool bProtected);

	// --- Morale System ---

	/** Apply a morale event (positive or negative). Events have diminishing returns. */
	UFUNCTION(BlueprintCallable, Category = "Survival|Morale")
	void ApplyMoraleEvent(float MoraleDelta, FName EventTag);

	// --- Serialization ---

	UFUNCTION(BlueprintCallable, Category = "Survival")
	FSurvivalSnapshot CreateSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Survival")
	void RestoreFromSnapshot(const FSurvivalSnapshot& Snapshot);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Survival")
	FOnSurvivalStatChanged OnSurvivalStatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Survival")
	FOnThresholdCrossed OnThresholdCrossed;

protected:
	virtual void BeginPlay() override;

	// Configuration per stat type
	UPROPERTY(EditAnywhere, Category = "Survival|Config")
	TMap<ESurvivalStatType, FSurvivalStatConfig> StatConfigs;

	// Current stat values
	UPROPERTY(VisibleAnywhere, Category = "Survival|Runtime")
	TMap<ESurvivalStatType, float> CurrentStats;

	// Cached thresholds for detecting crossings
	TMap<ESurvivalStatType, ESurvivalThreshold> CachedThresholds;

	// Cold environment
	UPROPERTY(VisibleAnywhere, Category = "Survival|Runtime")
	float EnvironmentColdLevel = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Survival|Runtime")
	bool bHasColdProtection = false;

	// Rate at which Cold stat drains when exposed (per second, scaled by EnvironmentColdLevel)
	UPROPERTY(EditAnywhere, Category = "Survival|Config")
	float ColdDrainRate = 5.0f;

	// Morale event cooldown tracking to prevent spam
	TMap<FName, float> MoraleEventCooldowns;

	UPROPERTY(EditAnywhere, Category = "Survival|Config")
	float MoraleEventCooldownSeconds = 30.0f;

private:
	void InitializeDefaults();
	void TickDecay(float DeltaTime);
	void TickColdExposure(float DeltaTime);
	void TickStaminaRegen(float DeltaTime);
	void CheckThresholdCrossings(ESurvivalStatType StatType, float OldValue);

	ESurvivalThreshold CalculateThreshold(float Percent) const;
	float GetMultiplierForThreshold(const TArray<float>& Multipliers, ESurvivalThreshold Threshold) const;
	float ClampStat(ESurvivalStatType StatType, float Value) const;
};
