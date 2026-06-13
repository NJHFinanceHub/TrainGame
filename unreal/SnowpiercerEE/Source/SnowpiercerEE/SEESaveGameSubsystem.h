#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "SEETypes.h"
#include "Endings/SEEEndingTypes.h"
#include "Factions/SEEFactionTypes.h"        // FSEEFactionSaveState
#include "SEEQuestManager.h"                  // FSEEQuestSaveState
#include "SEEInventoryComponent.h"            // FSEEItemSaveEntry + USEEInventoryComponent
#include "TrainGame/Economy/ArmorComponent.h" // FEquippedArmor + UArmorComponent
#include "SEESaveGameSubsystem.generated.h"

class APawn;

UCLASS()
class SNOWPIERCEREE_API USEESaveGameData : public USaveGame
{
    GENERATED_BODY()

public:
    // --- Existing world / narrative state ---

    UPROPERTY(SaveGame)
    TMap<int32, FSEECarState> CarStates;

    UPROPERTY(SaveGame)
    FSEELedgerSnapshot LedgerScores;

    UPROPERTY(SaveGame)
    TMap<FName, bool> GlobalFlags;

    UPROPERTY(SaveGame)
    TMap<FName, int32> GlobalIntFlags;

    UPROPERTY(SaveGame)
    TArray<FSEELedgerEntry> ChoiceHistory;

    // --- Player state (only meaningful when bHasPlayerState) ---

    /** True once a live player pawn has been captured into this save. */
    UPROPERTY(SaveGame)
    bool bHasPlayerState = false;

    UPROPERTY(SaveGame)
    FTransform PlayerTransform;

    UPROPERTY(SaveGame)
    float PlayerHealth = 0.0f;

    UPROPERTY(SaveGame)
    float PlayerMaxHealth = 0.0f;

    UPROPERTY(SaveGame)
    float PlayerStamina = 0.0f;

    UPROPERTY(SaveGame)
    float PlayerMaxStamina = 0.0f;

    /** Stats. */
    UPROPERTY(SaveGame)
    int32 PlayerXP = 0;

    UPROPERTY(SaveGame)
    int32 PlayerLevel = 1;

    /** Full inventory snapshot (one entry per non-empty slot). */
    UPROPERTY(SaveGame)
    TArray<FSEEItemSaveEntry> Inventory;

    /** Equipped armor (one entry per occupied slot; full runtime piece incl. durability). */
    UPROPERTY(SaveGame)
    TArray<FEquippedArmor> EquippedArmor;

    // --- World subsystem state ---

    UPROPERTY(SaveGame)
    TArray<FSEEQuestSaveState> QuestStates;

    UPROPERTY(SaveGame)
    FSEEFactionSaveState FactionState;
};

UCLASS()
class SNOWPIERCEREE_API USEESaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetCarState(int32 CarIndex, const FSEECarState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool GetCarState(int32 CarIndex, FSEECarState& OutState) const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool WriteToSlot();

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadFromSlot();

    /** True when the main save slot exists on disk (drives the menu Continue button). */
    UFUNCTION(BlueprintPure, Category="Save")
    bool DoesSaveGameExist() const;

    // --- Pending player restore ---
    //
    // The player pawn may not exist when LoadFromSlot() runs (loading from the
    // main menu before the level streams in). LoadFromSlot() stashes the player
    // slice as a "pending" blob; a short polling timer finds the pawn once it
    // spawns and applies it. The public apply hook is also exposed so a player's
    // BeginPlay can drive the restore explicitly if preferred.

    UFUNCTION(BlueprintPure, Category="Save")
    bool HasPendingPlayerState() const { return bHasPendingPlayerState; }

    /** Apply (and consume) the pending player slice onto the given pawn. No-op when nothing pending or Pawn is null. */
    UFUNCTION(BlueprintCallable, Category="Save")
    void ApplyPendingPlayerStateToPawn(APawn* Pawn);

private:
    /** Capture the live player pawn's slice into SaveObj. Returns true if a pawn was found. */
    bool CapturePlayerState(USEESaveGameData* SaveObj);

    /** Begin polling for the player pawn so the pending slice can be applied once it spawns. */
    void StartPendingRestorePoll();

    /** Timer tick: tries to apply the pending slice to the current player pawn. */
    void PollForPlayerPawn();

    APawn* GetPlayerPawn() const;

    UPROPERTY()
    TMap<int32, FSEECarState> RuntimeCarStates;

    UPROPERTY()
    FString SaveSlotName = TEXT("SnowpiercerEE_Main");

    UPROPERTY()
    int32 UserIndex = 0;

    // --- Pending restore blob (copied out of the loaded save) ---

    bool bHasPendingPlayerState = false;
    FTransform PendingTransform;
    float PendingHealth = 0.0f;
    float PendingMaxHealth = 0.0f;
    float PendingStamina = 0.0f;
    int32 PendingXP = 0;
    TArray<FSEEItemSaveEntry> PendingInventory;
    TArray<FEquippedArmor> PendingArmor;

    FTimerHandle PendingRestoreTimer;
    int32 PendingRestorePolls = 0;
};
