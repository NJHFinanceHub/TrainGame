// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEEndingTypes.h"
#include "SEEEndingCalculator.generated.h"

class USEELedgerSubsystem;
class USEEFactionManager;
class UCompanionRosterSubsystem;
class USEEQuestManager;

// ============================================================================
// USEEEndingCalculator
//
// Aggregates data from the Ledger, companion system, faction system, and
// quest state to determine which endings are available, which is recommended,
// and what variation of each ending should play.
//
// The ending is not a single choice — it is the sum of all choices.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingUnlocked, ESEEEnding, Ending);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingSelected, const FSEEEndingResult&, Result);

UCLASS()
class SNOWPIERCEREE_API USEEEndingCalculator : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Calculate all available endings based on current game state */
	UFUNCTION(BlueprintCallable, Category = "Ending")
	TArray<FSEEEndingResult> CalculateAvailableEndings();

	/** Get the ending with highest affinity score */
	UFUNCTION(BlueprintCallable, Category = "Ending")
	FSEEEndingResult GetRecommendedEnding();

	/** Check if a specific ending is available */
	UFUNCTION(BlueprintPure, Category = "Ending")
	bool IsEndingAvailable(ESEEEnding Ending);

	/** Get the variation quality for a specific ending */
	UFUNCTION(BlueprintPure, Category = "Ending")
	ESEEEndingVariation CalculateEndingVariation(ESEEEnding Ending);

	/** Select an ending — locks the choice and triggers the cinematic pipeline */
	UFUNCTION(BlueprintCallable, Category = "Ending")
	FSEEEndingResult SelectEnding(ESEEEnding Ending);

	/** Get the currently selected ending (None if not yet selected) */
	UFUNCTION(BlueprintPure, Category = "Ending")
	ESEEEnding GetSelectedEnding() const { return SelectedEnding; }

	/** Build the post-game statistics summary */
	UFUNCTION(BlueprintCallable, Category = "Ending")
	FSEEEndgameStats BuildEndgameStats();

	/** Get the cinematic data for a specific ending */
	UFUNCTION(BlueprintPure, Category = "Ending")
	FSEEEndingCinematicData GetEndingCinematicData(ESEEEnding Ending) const;

	UPROPERTY(BlueprintAssignable, Category = "Ending")
	FOnEndingUnlocked OnEndingUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Ending")
	FOnEndingSelected OnEndingSelected;

private:
	ESEEEnding SelectedEnding = ESEEEnding::None;

	/** Cached ending requirements — populated on Initialize */
	TArray<FSEEEndingRequirement> EndingRequirements;

	/** Cached cinematic data per ending */
	TMap<ESEEEnding, FSEEEndingCinematicData> CinematicDataMap;

	/** Set up the hardcoded ending requirements based on design doc */
	void InitializeEndingRequirements();

	/** Set up cinematic data for each ending */
	void InitializeCinematicData();

	/** Evaluate a single ending against current game state */
	FSEEEndingResult EvaluateEnding(const FSEEEndingRequirement& Requirement);

	/** Calculate affinity score — how well the player's profile matches an ending */
	float CalculateAffinity(ESEEEnding Ending, const FSEELedgerSnapshot& Ledger);

	/** Get companion survival count from roster subsystem */
	int32 GetSurvivingCompanionCount() const;

	/** Get surviving companion names */
	TArray<FName> GetSurvivingCompanionNames() const;
};
