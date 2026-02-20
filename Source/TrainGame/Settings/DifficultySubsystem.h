// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DifficultyTypes.h"
#include "DifficultySubsystem.generated.h"

// ============================================================================
// USEEDifficultySubsystem
//
// Manages difficulty tiers, adaptive difficulty, and mid-game changes.
// Provides modifier queries for all gameplay systems (combat, stealth,
// resources, survival, companions).
//
// 4 tiers: Passenger, Survivor, Eternal Engine, Mr. Wilford
// Optional adaptive difficulty layer with ±10% sub-tier adjustments.
// ============================================================================
UCLASS()
class TRAINGAME_API USEEDifficultySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USEEDifficultySubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Tier Management ---

	/** Get the current difficulty tier. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	EDifficultyTier GetCurrentTier() const { return CurrentTier; }

	/** Set difficulty tier. Enforces Mr. Wilford restrictions. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty")
	bool SetDifficultyTier(EDifficultyTier NewTier);

	/** Check if lowering difficulty is allowed (always true). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool CanLowerDifficulty() const { return true; }

	/** Check if raising difficulty is allowed. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool CanRaiseDifficulty(EDifficultyTier TargetTier) const;

	/** Get the lowest difficulty ever used on the current save. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	EDifficultyTier GetLowestDifficultyPlayed() const { return LowestDifficultyPlayed; }

	/** Check if Mr. Wilford was ever active (for exclusive achievements). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool WasEverMrWilford() const { return bEverMrWilford; }

	/** Check if player left Mr. Wilford (disabling exclusive achievements). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool HasLeftMrWilford() const { return bLeftMrWilford; }

	// --- Modifier Queries ---

	/** Get the final modifiers (base tier + adaptive sub-tier adjustment). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	FDifficultyModifiers GetActiveModifiers() const;

	/** Get base modifiers for a specific tier (no adaptive adjustment). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	FDifficultyModifiers GetTierModifiers(EDifficultyTier Tier) const;

	/** Shortcut: get enemy health multiplier including adaptive. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetEnemyHealthMultiplier() const;

	/** Shortcut: get enemy damage multiplier including adaptive. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetEnemyDamageMultiplier() const;

	/** Shortcut: get resource drop rate multiplier including adaptive. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetResourceDropMultiplier() const;

	/** Shortcut: get detection range multiplier. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetDetectionRangeMultiplier() const;

	/** Shortcut: get stat check modifier. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	int32 GetStatCheckModifier() const;

	/** Shortcut: get companion down timer. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	float GetCompanionDownTimer() const;

	/** Check if permadeath is active. */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool IsPermadeathActive() const;

	/** Check if saving is restricted (Eternal Engine / Mr. Wilford). */
	UFUNCTION(BlueprintPure, Category = "Difficulty")
	bool IsSavingRestricted() const;

	// --- Adaptive Difficulty ---

	/** Enable/disable adaptive difficulty. Not available on Mr. Wilford. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void SetAdaptiveDifficultyEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Difficulty|Adaptive")
	bool IsAdaptiveDifficultyEnabled() const { return bAdaptiveEnabled; }

	UFUNCTION(BlueprintPure, Category = "Difficulty|Adaptive")
	EAdaptiveSubTier GetAdaptiveSubTier() const { return CurrentSubTier; }

	/** Report a player death for adaptive scoring. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void ReportPlayerDeath();

	/** Report a combat encounter completion. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void ReportCombatComplete(float DurationSeconds, float HealthPercent);

	/** Report a stealth detection event. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void ReportStealthDetection(bool bWasDetected);

	/** Report current resource status. */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void ReportResourceStatus(float CarryCapacityPercent);

	/** Reset adaptive difficulty (called on zone transitions). */
	UFUNCTION(BlueprintCallable, Category = "Difficulty|Adaptive")
	void ResetAdaptiveData();

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Difficulty")
	FOnDifficultyChanged OnDifficultyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Difficulty|Adaptive")
	FOnAdaptiveSubTierChanged OnAdaptiveSubTierChanged;

protected:

	/** Default tier configurations. Populated in Initialize(). */
	UPROPERTY(EditAnywhere, Category = "Difficulty|Config")
	TMap<EDifficultyTier, FDifficultyTierConfig> TierConfigs;

private:
	void InitializeDefaultTiers();
	void UpdateAdaptiveSubTier();
	float GetAdaptiveMultiplierOffset() const;

	EDifficultyTier CurrentTier = EDifficultyTier::Survivor;
	EDifficultyTier LowestDifficultyPlayed = EDifficultyTier::Survivor;
	bool bEverMrWilford = false;
	bool bLeftMrWilford = false;

	// Adaptive difficulty
	bool bAdaptiveEnabled = false;
	EAdaptiveSubTier CurrentSubTier = EAdaptiveSubTier::Normal;
	FAdaptivePerformanceData PerformanceData;

	// Rolling performance window
	int32 CombatEncounterCount = 0;
	int32 StealthAttemptCount = 0;
	int32 StealthDetectionCount = 0;
};
