// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEEndingTypes.h"
#include "SEEEndingCinematicsSubsystem.generated.h"

class USEEEndingCalculator;

// ============================================================================
// USEEEndingCinematicsSubsystem
//
// Manages the presentation layer for endings: cutscene sequencing,
// epilogue narration, credits music variation, and post-game stats display.
// Each ending has a unique cinematic flow with variations based on Ledger scores.
// ============================================================================

/** Phases of the ending cinematic flow */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingPhaseStarted, ESEEEndingCinematicPhase, Phase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingPhaseCompleted, ESEEEndingCinematicPhase, Phase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEndingCinematicComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreditsStarted, ESEEEnding, Ending);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndgameStatsReady, const FSEEEndgameStats&, Stats);

UCLASS()
class SNOWPIERCEREE_API USEEEndingCinematicsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Cinematic Flow ---

	/** Begin the ending cinematic sequence for the selected ending */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematic")
	void BeginEndingSequence(ESEEEnding Ending);

	/** Advance to the next phase of the ending cinematic */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematic")
	void AdvancePhase();

	/** Get the current cinematic phase */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	ESEEEndingCinematicPhase GetCurrentPhase() const { return CurrentPhase; }

	/** Is a cinematic sequence currently playing? */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	bool IsPlayingEndingCinematic() const { return bCinematicActive; }

	// --- Content Access ---

	/** Get Wilford's dialogue for the current ending + variation */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	FText GetWilfordDialogue() const;

	/** Get epilogue narration for the current ending + variation */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	FText GetEpilogueNarration() const;

	/** Get ending title text */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	FText GetEndingTitle() const;

	/** Get ending subtitle text */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	FText GetEndingSubtitle() const;

	/** Get credits music for the current ending */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	TSoftObjectPtr<USoundBase> GetCreditsMusic() const;

	/** Get the post-game stats (available during Credits phase) */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	FSEEEndgameStats GetEndgameStats() const { return CachedStats; }

	/** Get companions that appear in the ending cinematic */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematic")
	TArray<FName> GetEndingCompanions() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematic")
	FOnEndingPhaseStarted OnEndingPhaseStarted;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematic")
	FOnEndingPhaseCompleted OnEndingPhaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematic")
	FOnEndingCinematicComplete OnEndingCinematicComplete;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematic")
	FOnCreditsStarted OnCreditsStarted;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematic")
	FOnEndgameStatsReady OnEndgameStatsReady;

private:
	ESEEEnding ActiveEnding = ESEEEnding::None;
	ESEEEndingVariation ActiveVariation = ESEEEndingVariation::Bittersweet;
	ESEEEndingCinematicPhase CurrentPhase = ESEEEndingCinematicPhase::Confrontation;
	bool bCinematicActive = false;

	FSEEEndingCinematicData ActiveCinematicData;
	FSEEEndgameStats CachedStats;

	/** The ordered sequence of cinematic phases */
	static const TArray<ESEEEndingCinematicPhase>& GetPhaseSequence();
	int32 CurrentPhaseIndex = 0;
};
