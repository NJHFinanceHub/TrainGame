// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveTypes.h"
#include "SaveGameSubsystem.generated.h"

// ============================================================================
// USEESaveGameSubsystem
//
// Game instance subsystem managing save/load operations.
// Binary format with JSON header, LZ4-compressed payload.
// Supports manual saves, quick saves, rotating autosaves.
//
// Autosave fires on: car transition, quest completion, companion recruitment,
// major choice, timed interval, pre-combat — with a 60-second cooldown.
//
// Save/load is asynchronous; the game does not pause during autosave.
// ============================================================================
UCLASS()
class TRAINGAME_API USEESaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USEESaveGameSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Save Operations ---

	/** Save to a manual slot (0-based index within manual slots). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveToManualSlot(int32 SlotIndex, const FString& SlotName);

	/** Quick save to the next rotating quick save slot. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void QuickSave();

	/** Trigger an autosave. Respects cooldown; silently skips if on cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void TriggerAutosave(EAutosaveTrigger Trigger);

	// --- Load Operations ---

	/** Load from a specific slot index (absolute, across all slot types). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadFromSlot(int32 AbsoluteSlotIndex);

	/** Load the most recent autosave (fallback for corruption recovery). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadMostRecentAutosave();

	// --- Slot Queries ---

	/** Get info for all save slots (for the save/load UI). */
	UFUNCTION(BlueprintCallable, Category = "Save")
	TArray<FSaveSlotInfo> GetAllSlotInfo() const;

	/** Get info for a specific slot. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	FSaveSlotInfo GetSlotInfo(int32 AbsoluteSlotIndex) const;

	/** Check if a slot has save data. */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsSlotOccupied(int32 AbsoluteSlotIndex) const;

	/** Delete a save slot. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void DeleteSlot(int32 AbsoluteSlotIndex);

	// --- State Gathering ---

	/** Gather current game state into a save data struct. Called internally before write. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	FSaveGameData GatherCurrentState() const;

	/** Apply loaded save data to the game world. Called internally after read. */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void ApplyLoadedState(const FSaveGameData& Data);

	// --- Per-Car State ---

	/** Register a car's modified state for persistence. */
	UFUNCTION(BlueprintCallable, Category = "Save|Car")
	void MarkCarModified(int32 CarIndex);

	/** Get the persisted delta for a car, or empty if unvisited. */
	UFUNCTION(BlueprintCallable, Category = "Save|Car")
	bool GetCarState(int32 CarIndex, FSaveCarState& OutState) const;

	/** Update a car's state delta (called when loot opened, door broken, NPC killed, etc). */
	UFUNCTION(BlueprintCallable, Category = "Save|Car")
	void UpdateCarState(const FSaveCarState& CarState);

	// --- Autosave Configuration ---

	/** Enable/disable timed autosave interval. */
	UFUNCTION(BlueprintCallable, Category = "Save|Config")
	void SetTimedAutosaveEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Save|Config")
	bool IsTimedAutosaveEnabled() const { return bTimedAutosaveEnabled; }

	// --- Mr. Wilford Permadeath ---

	/** For Mr. Wilford difficulty: delete the save file on player death. */
	UFUNCTION(BlueprintCallable, Category = "Save|Permadeath")
	void DeletePermadeathSave();

	/** Check if current save is a permadeath (Mr. Wilford) save. */
	UFUNCTION(BlueprintPure, Category = "Save|Permadeath")
	bool IsPermadeathSave() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnSaveComplete OnSaveComplete;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnLoadComplete OnLoadComplete;

	UPROPERTY(BlueprintAssignable, Category = "Save")
	FOnAutosaveTriggered OnAutosaveTriggered;

protected:

	UPROPERTY(EditAnywhere, Category = "Save|Config")
	FSaveSlotConfig SlotConfig;

private:
	// --- Internal Save/Load ---

	ESaveResult WriteSaveFile(const FSaveGameData& Data, const FString& FilePath);
	ELoadResult ReadSaveFile(const FString& FilePath, FSaveGameData& OutData);

	FString GetSlotFilePath(int32 AbsoluteSlotIndex) const;
	FString GetSaveDirectory() const;
	ESaveSlotType GetSlotType(int32 AbsoluteSlotIndex) const;
	int32 GetSlotSubIndex(int32 AbsoluteSlotIndex) const;

	// JSON header serialization
	FString SerializeHeader(const FSaveHeaderData& Header) const;
	bool DeserializeHeader(const FString& JsonStr, FSaveHeaderData& OutHeader) const;

	// Checksum
	FString ComputePayloadChecksum(const TArray<uint8>& PayloadBytes) const;
	bool ValidateChecksum(const FString& Expected, const TArray<uint8>& PayloadBytes) const;

	// Async save support
	void PerformAsyncSave(FSaveGameData Data, FString FilePath, int32 SlotIndex);

	// Per-car state cache (in-memory, flushed to save file)
	TMap<int32, FSaveCarState> CarStateCache;

	// Autosave management
	float AutosaveCooldownRemaining = 0.f;
	int32 NextAutosaveSlot = 0;
	int32 NextQuickSaveSlot = 0;
	bool bTimedAutosaveEnabled = true;
	float TimedAutosaveTimer = 0.f;
	FTimerHandle TimedAutosaveHandle;

	void OnTimedAutosaveTick();

	// Active save data (loaded state)
	TOptional<FSaveGameData> ActiveSaveData;
	bool bIsPermadeathSave = false;
};
