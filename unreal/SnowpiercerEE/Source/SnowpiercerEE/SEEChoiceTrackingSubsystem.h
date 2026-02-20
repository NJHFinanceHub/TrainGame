#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEETypes.h"
#include "SEEChoiceTrackingSubsystem.generated.h"

/**
 * Hidden Ledger axis — five moral dimensions tracked across all choices.
 * Each axis has two poles. The score represents the balance between them.
 * Positive values lean toward the first pole, negative toward the second.
 */
UENUM(BlueprintType)
enum class ESEELedgerAxis : uint8
{
	MercyVsPragmatism		UMETA(DisplayName = "Mercy vs Pragmatism"),
	IndividualVsCollective	UMETA(DisplayName = "Individual vs Collective"),
	TruthVsStability		UMETA(DisplayName = "Truth vs Stability"),
	ForceVsCunning			UMETA(DisplayName = "Force vs Cunning"),
	PresentVsFuture			UMETA(DisplayName = "Present vs Future")
};

/**
 * Record of a single moral choice the player made.
 */
USTRUCT(BlueprintType)
struct FSEEChoiceRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName ChoiceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName SelectedOptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 CarIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FString Zone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	float GameTimeWhenChosen = 0.0f;
};

/**
 * Snapshot of the Hidden Ledger for save/load.
 */
USTRUCT(BlueprintType)
struct FSEELedgerSnapshot
{
	GENERATED_BODY()

	/** Mercy (+) vs Pragmatism (-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 MercyPragmatism = 0;

	/** Individual (+) vs Collective (-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 IndividualCollective = 0;

	/** Truth (+) vs Stability (-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 TruthStability = 0;

	/** Force (+) vs Cunning (-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 ForceCunning = 0;

	/** Present (+) vs Future (-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 PresentFuture = 0;
};

/**
 * Full choice tracking save data.
 */
USTRUCT(BlueprintType)
struct FSEEChoiceTrackingSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FSEELedgerSnapshot Ledger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FSEEChoiceRecord> ChoiceHistory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TMap<FName, bool> ChoiceFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TMap<FName, int32> CompoundingCounters;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChoiceMade, FName, ChoiceID, FName, OptionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLedgerChanged, ESEELedgerAxis, Axis, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlagSet, FName, FlagName);

/**
 * Tracks all player moral choices, the Hidden Ledger, and consequence flags.
 * Integrates with the save system for persistence.
 */
UCLASS()
class SNOWPIERCEREE_API USEEChoiceTrackingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Choice Recording ---

	/** Record a moral choice. Applies ledger shifts, sets flags, and logs to history. */
	UFUNCTION(BlueprintCallable, Category = "Choices")
	void RecordChoice(FName ChoiceID, FName OptionID, int32 CarIndex, const FString& Zone);

	/** Check if a specific choice has been made. */
	UFUNCTION(BlueprintPure, Category = "Choices")
	bool HasMadeChoice(FName ChoiceID) const;

	/** Get which option was selected for a choice. Returns NAME_None if not yet made. */
	UFUNCTION(BlueprintPure, Category = "Choices")
	FName GetChosenOption(FName ChoiceID) const;

	/** Get the full choice history. */
	UFUNCTION(BlueprintPure, Category = "Choices")
	TArray<FSEEChoiceRecord> GetChoiceHistory() const { return SaveData.ChoiceHistory; }

	// --- Hidden Ledger ---

	/** Modify a ledger axis by delta. Clamped to [-100, 100]. */
	UFUNCTION(BlueprintCallable, Category = "Choices|Ledger")
	void ModifyLedger(ESEELedgerAxis Axis, int32 Delta);

	/** Get current value of a ledger axis. */
	UFUNCTION(BlueprintPure, Category = "Choices|Ledger")
	int32 GetLedgerValue(ESEELedgerAxis Axis) const;

	/** Get the full ledger snapshot. */
	UFUNCTION(BlueprintPure, Category = "Choices|Ledger")
	FSEELedgerSnapshot GetLedgerSnapshot() const { return SaveData.Ledger; }

	/** Check if a specific ending path is available based on Ledger scores. */
	UFUNCTION(BlueprintPure, Category = "Choices|Ledger")
	bool IsEndingAvailable(FName EndingID) const;

	// --- Consequence Flags ---

	/** Set a named flag (e.g., "Flag_Gilliam_Deposed"). */
	UFUNCTION(BlueprintCallable, Category = "Choices|Flags")
	void SetFlag(FName FlagName);

	/** Clear a named flag. */
	UFUNCTION(BlueprintCallable, Category = "Choices|Flags")
	void ClearFlag(FName FlagName);

	/** Check if a flag is set. */
	UFUNCTION(BlueprintPure, Category = "Choices|Flags")
	bool IsFlagSet(FName FlagName) const;

	/** Get all set flags. */
	UFUNCTION(BlueprintPure, Category = "Choices|Flags")
	TArray<FName> GetAllFlags() const;

	// --- Compounding Counters ---

	/** Increment a compounding counter (e.g., "InfrastructureDestroyed"). */
	UFUNCTION(BlueprintCallable, Category = "Choices|Compounding")
	void IncrementCounter(FName CounterName, int32 Delta = 1);

	/** Get a compounding counter value. */
	UFUNCTION(BlueprintPure, Category = "Choices|Compounding")
	int32 GetCounter(FName CounterName) const;

	// --- Save/Load ---

	/** Get full save data for serialization. */
	UFUNCTION(BlueprintPure, Category = "Choices|Save")
	FSEEChoiceTrackingSaveData GetSaveData() const { return SaveData; }

	/** Restore from save data. */
	UFUNCTION(BlueprintCallable, Category = "Choices|Save")
	void RestoreFromSaveData(const FSEEChoiceTrackingSaveData& Data);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Choices")
	FOnChoiceMade OnChoiceMade;

	UPROPERTY(BlueprintAssignable, Category = "Choices")
	FOnLedgerChanged OnLedgerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Choices")
	FOnFlagSet OnFlagSet;

private:
	FSEEChoiceTrackingSaveData SaveData;

	int32& GetLedgerRef(ESEELedgerAxis Axis);
	int32 ClampLedger(int32 Value) const { return FMath::Clamp(Value, -100, 100); }
};
