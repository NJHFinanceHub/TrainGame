// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "EndingTypes.h"
#include "MoralLedgerSubsystem.generated.h"

// ============================================================================
// Moral Ledger Subsystem
// Tracks the Hidden Ledger — five moral axes accumulated from player choices.
// This data feeds into the Ending Calculator to determine ending eligibility.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMoralAxisChanged, EMoralAxis, Axis, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryFlagSet, FName, FlagName, bool, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoralChoiceRecorded, const FMoralChoiceRecord&, Record);

UCLASS()
class TRAINGAME_API UMoralLedgerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// -------------------------------------------------------------------
	// Moral Axis Operations
	// -------------------------------------------------------------------

	/** Apply a delta to a single moral axis. Clamped to [-100, 100]. */
	UFUNCTION(BlueprintCallable, Category = "Moral Ledger")
	void ModifyAxis(EMoralAxis Axis, int32 Delta);

	/** Get current value of a moral axis. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	int32 GetAxisValue(EMoralAxis Axis) const;

	/** Get normalized axis value (-1.0 to 1.0). */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	float GetAxisNormalized(EMoralAxis Axis) const;

	// -------------------------------------------------------------------
	// Choice Recording
	// -------------------------------------------------------------------

	/** Record a moral choice with its axis deltas. Called by the dialogue/quest systems. */
	UFUNCTION(BlueprintCallable, Category = "Moral Ledger")
	void RecordChoice(FName ChoiceID, FName OptionChosen, int32 CarIndex, const TMap<uint8, int32>& AxisDeltas);

	/** Check if a specific choice has already been made. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	bool HasMadeChoice(FName ChoiceID) const;

	/** Get the option chosen for a specific choice. Returns NAME_None if not made. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	FName GetChosenOption(FName ChoiceID) const;

	/** Get total number of choices recorded. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	int32 GetTotalChoicesMade() const { return ChoiceHistory.Num(); }

	// -------------------------------------------------------------------
	// Story Flags
	// -------------------------------------------------------------------

	/** Set a global story flag. */
	UFUNCTION(BlueprintCallable, Category = "Moral Ledger")
	void SetStoryFlag(FName FlagName, bool bValue = true);

	/** Check a global story flag. Returns false if not set. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	bool GetStoryFlag(FName FlagName) const;

	/** Get all set story flags. */
	UFUNCTION(BlueprintPure, Category = "Moral Ledger")
	TMap<FName, bool> GetAllStoryFlags() const { return StoryFlags; }

	// -------------------------------------------------------------------
	// Save/Load
	// -------------------------------------------------------------------

	/** Export full ledger state for saving. */
	UFUNCTION(BlueprintCallable, Category = "Moral Ledger")
	FMoralLedgerSnapshot CreateSnapshot() const;

	/** Restore ledger state from a snapshot. */
	UFUNCTION(BlueprintCallable, Category = "Moral Ledger")
	void RestoreFromSnapshot(const FMoralLedgerSnapshot& Snapshot);

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Moral Ledger")
	FOnMoralAxisChanged OnMoralAxisChanged;

	UPROPERTY(BlueprintAssignable, Category = "Moral Ledger")
	FOnStoryFlagSet OnStoryFlagSet;

	UPROPERTY(BlueprintAssignable, Category = "Moral Ledger")
	FOnMoralChoiceRecorded OnMoralChoiceRecorded;

private:
	/** Axis values indexed by EMoralAxis. Clamped [-100, 100]. */
	int32 AxisValues[static_cast<int32>(EMoralAxis::MAX)] = {};

	TArray<FMoralChoiceRecord> ChoiceHistory;
	TMap<FName, bool> StoryFlags;
};
