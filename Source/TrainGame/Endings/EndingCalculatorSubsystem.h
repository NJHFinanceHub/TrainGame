// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EndingTypes.h"
#include "SnowyEngine/Faction/FactionTypes.h"
#include "EndingCalculatorSubsystem.generated.h"

// ============================================================================
// Ending Calculator Subsystem
// Aggregates choice flags, companion survival, faction alignment, and the
// moral ledger to determine which endings are available and recommended.
// ============================================================================

class UMoralLedgerSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingTriggered, EEndingType, Ending);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndingEligibilityChanged, EEndingType, Ending, EEndingEligibility, NewEligibility);

UCLASS()
class TRAINGAME_API UEndingCalculatorSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------
	// Eligibility Evaluation
	// -------------------------------------------------------------------

	/** Recalculate eligibility for all 6 endings based on current game state. */
	UFUNCTION(BlueprintCallable, Category = "Ending Calculator")
	void RecalculateAllEligibility();

	/** Get eligibility result for a specific ending. */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	FEndingEligibilityResult GetEligibility(EEndingType Ending) const;

	/** Get all eligibility results, sorted by affinity score descending. */
	UFUNCTION(BlueprintCallable, Category = "Ending Calculator")
	TArray<FEndingEligibilityResult> GetAllEligibility() const;

	/** Get the recommended ending (highest affinity among available endings). */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	EEndingType GetRecommendedEnding() const;

	/** Get the number of endings currently available. */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	int32 GetAvailableEndingCount() const;

	// -------------------------------------------------------------------
	// Ending Trigger
	// -------------------------------------------------------------------

	/** Trigger a specific ending. Only succeeds if ending is Available or Recommended. */
	UFUNCTION(BlueprintCallable, Category = "Ending Calculator")
	bool TriggerEnding(EEndingTrigger Trigger);

	/** Get the ending type associated with a trigger action. */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	static EEndingType TriggerToEnding(EEndingTrigger Trigger);

	/** Get which ending was triggered (None if game hasn't ended). */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	EEndingType GetTriggeredEnding() const { return TriggeredEnding; }

	/** Has an ending been triggered? */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	bool HasEndingBeenTriggered() const { return TriggeredEnding != EEndingType::None; }

	// -------------------------------------------------------------------
	// Companion Fate Evaluation
	// -------------------------------------------------------------------

	/** Evaluate companion fates at ending time. Call after TriggerEnding. */
	UFUNCTION(BlueprintCallable, Category = "Ending Calculator")
	TArray<FCompanionEndingFate> EvaluateCompanionFates() const;

	/** Count how many companions survived to the ending. */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	int32 GetSurvivingCompanionCount() const;

	// -------------------------------------------------------------------
	// Cinematic Data
	// -------------------------------------------------------------------

	/** Get the cinematic data for the triggered ending. */
	UFUNCTION(BlueprintPure, Category = "Ending Calculator")
	FEndingCinematicData GetEndingCinematicData() const;

	/** Register cinematic data for an ending (called during initialization). */
	UFUNCTION(BlueprintCallable, Category = "Ending Calculator")
	void RegisterCinematicData(EEndingType Ending, const FEndingCinematicData& Data);

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Ending Calculator")
	FOnEndingTriggered OnEndingTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Ending Calculator")
	FOnEndingEligibilityChanged OnEndingEligibilityChanged;

private:
	/** Cached eligibility results. Recalculated on demand. */
	TMap<EEndingType, FEndingEligibilityResult> CachedEligibility;

	/** Which ending was triggered (None until endgame). */
	EEndingType TriggeredEnding = EEndingType::None;

	/** Per-ending cinematic data registered at init. */
	TMap<EEndingType, FEndingCinematicData> CinematicDataMap;

	// -------------------------------------------------------------------
	// Per-Ending Prerequisite Evaluation
	// -------------------------------------------------------------------

	FEndingEligibilityResult EvaluateNewWilford(const UMoralLedgerSubsystem* Ledger) const;
	FEndingEligibilityResult EvaluateRevolution(const UMoralLedgerSubsystem* Ledger) const;
	FEndingEligibilityResult EvaluateDerailment(const UMoralLedgerSubsystem* Ledger) const;
	FEndingEligibilityResult EvaluateExodus(const UMoralLedgerSubsystem* Ledger) const;
	FEndingEligibilityResult EvaluateEternalLoop(const UMoralLedgerSubsystem* Ledger) const;
	FEndingEligibilityResult EvaluateBridge(const UMoralLedgerSubsystem* Ledger) const;

	/** Calculate affinity score for an ending based on moral axis alignment. */
	float CalculateAffinity(const UMoralLedgerSubsystem* Ledger, const TMap<EMoralAxis, float>& Weights) const;

	/** Helper: check if a story flag is set. */
	bool CheckFlag(const UMoralLedgerSubsystem* Ledger, FName FlagName) const;

	/** Helper: check faction standing meets threshold. */
	bool CheckFactionStanding(EFaction Faction, float MinRep) const;
};
