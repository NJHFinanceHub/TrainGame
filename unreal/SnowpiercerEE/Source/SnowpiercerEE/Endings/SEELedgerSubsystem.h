// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEEndingTypes.h"
#include "SEELedgerSubsystem.generated.h"

// ============================================================================
// USEELedgerSubsystem
//
// The Hidden Ledger — tracks five consequence axes across the entire game.
// No good/evil meter. No karma. Just patterns of behavior that accumulate
// and determine which endings are available and how they play out.
//
// The Ledger also manages persistent global game flags that track specific
// story decisions (e.g., "Gilliam_Forgiven", "Kronole_Den_Shut_Down").
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLedgerAxisChanged, ESEELedgerAxis, Axis, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGlobalFlagChanged, FName, FlagName, bool, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoralChoiceRecorded, const FSEELedgerEntry&, Entry);

UCLASS()
class SNOWPIERCEREE_API USEELedgerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Ledger Axis Tracking ---

	/** Record a moral choice with multiple axis impacts */
	UFUNCTION(BlueprintCallable, Category = "Ledger")
	void RecordChoice(const FSEELedgerEntry& Entry);

	/** Directly modify a single axis */
	UFUNCTION(BlueprintCallable, Category = "Ledger")
	void ModifyAxis(ESEELedgerAxis Axis, int32 Delta);

	/** Get the current score for a single axis */
	UFUNCTION(BlueprintPure, Category = "Ledger")
	int32 GetAxisScore(ESEELedgerAxis Axis) const;

	/** Get a snapshot of all axis scores */
	UFUNCTION(BlueprintPure, Category = "Ledger")
	FSEELedgerSnapshot GetLedgerSnapshot() const { return LedgerScores; }

	/** Get the complete history of moral choices */
	UFUNCTION(BlueprintPure, Category = "Ledger")
	TArray<FSEELedgerEntry> GetChoiceHistory() const { return ChoiceHistory; }

	/** Get total number of moral choices made */
	UFUNCTION(BlueprintPure, Category = "Ledger")
	int32 GetTotalChoicesMade() const { return ChoiceHistory.Num(); }

	// --- Global Game Flags ---

	/** Set a persistent global flag (survives across conversations, levels, saves) */
	UFUNCTION(BlueprintCallable, Category = "Ledger|Flags")
	void SetGlobalFlag(FName FlagName, bool Value);

	/** Get a global flag value (false if not set) */
	UFUNCTION(BlueprintPure, Category = "Ledger|Flags")
	bool GetGlobalFlag(FName FlagName) const;

	/** Set an integer global flag */
	UFUNCTION(BlueprintCallable, Category = "Ledger|Flags")
	void SetGlobalIntFlag(FName FlagName, int32 Value);

	/** Get an integer global flag (0 if not set) */
	UFUNCTION(BlueprintPure, Category = "Ledger|Flags")
	int32 GetGlobalIntFlag(FName FlagName) const;

	/** Check if a specific moral choice has been made */
	UFUNCTION(BlueprintPure, Category = "Ledger|Flags")
	bool HasChoiceBeenMade(FName ChoiceID) const;

	/** Get all set global flags */
	UFUNCTION(BlueprintPure, Category = "Ledger|Flags")
	TMap<FName, bool> GetAllGlobalFlags() const { return GlobalFlags; }

	// --- Gameplay Stat Tracking ---

	UFUNCTION(BlueprintCallable, Category = "Ledger|Stats")
	void IncrementEnemiesKilled() { EnemiesKilled++; }

	UFUNCTION(BlueprintCallable, Category = "Ledger|Stats")
	void IncrementEnemiesSpared() { EnemiesSpared++; }

	UFUNCTION(BlueprintPure, Category = "Ledger|Stats")
	int32 GetEnemiesKilled() const { return EnemiesKilled; }

	UFUNCTION(BlueprintPure, Category = "Ledger|Stats")
	int32 GetEnemiesSpared() const { return EnemiesSpared; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Ledger")
	FOnLedgerAxisChanged OnLedgerAxisChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ledger")
	FOnGlobalFlagChanged OnGlobalFlagChanged;

	UPROPERTY(BlueprintAssignable, Category = "Ledger")
	FOnMoralChoiceRecorded OnMoralChoiceRecorded;

	// --- Serialization (called by SaveGameSubsystem) ---

	FSEELedgerSnapshot GetScoresForSave() const { return LedgerScores; }
	void LoadScoresFromSave(const FSEELedgerSnapshot& Snapshot) { LedgerScores = Snapshot; }

	TMap<FName, bool> GetGlobalFlagsForSave() const { return GlobalFlags; }
	void LoadGlobalFlagsFromSave(const TMap<FName, bool>& Flags) { GlobalFlags = Flags; }

	TMap<FName, int32> GetGlobalIntFlagsForSave() const { return GlobalIntFlags; }
	void LoadGlobalIntFlagsFromSave(const TMap<FName, int32>& Flags) { GlobalIntFlags = Flags; }

	TArray<FSEELedgerEntry> GetHistoryForSave() const { return ChoiceHistory; }
	void LoadHistoryFromSave(const TArray<FSEELedgerEntry>& History) { ChoiceHistory = History; }

private:
	FSEELedgerSnapshot LedgerScores;

	TArray<FSEELedgerEntry> ChoiceHistory;

	TMap<FName, bool> GlobalFlags;

	TMap<FName, int32> GlobalIntFlags;

	int32 EnemiesKilled = 0;
	int32 EnemiesSpared = 0;
};
