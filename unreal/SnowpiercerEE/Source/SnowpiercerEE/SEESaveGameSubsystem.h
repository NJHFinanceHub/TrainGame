#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "SEETypes.h"
#include "SEESaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

// ── Save Data Object ──

UCLASS()
class SNOWPIERCEREE_API USEESaveGameData : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    int32 SaveVersion = 1;

    UPROPERTY(SaveGame)
    FDateTime Timestamp;

    UPROPERTY(SaveGame)
    float PlayTimeSeconds = 0.0f;

    UPROPERTY(SaveGame)
    ESEEDifficulty Difficulty = ESEEDifficulty::Survivor;

    UPROPERTY(SaveGame)
    ESEEDifficulty LowestDifficultyPlayed = ESEEDifficulty::Survivor;

    UPROPERTY(SaveGame)
    FSEEPlayerSaveState PlayerState;

    UPROPERTY(SaveGame)
    TMap<int32, FSEECarState> CarStates;

    UPROPERTY(SaveGame)
    TMap<FName, FSEENPCSaveState> NPCStates;

    UPROPERTY(SaveGame)
    TMap<FName, FSEEQuestSaveState> QuestStates;

    UPROPERTY(SaveGame)
    TMap<uint8, int32> FactionReputations;
};

// ── Save Game Subsystem ──

UCLASS()
class SNOWPIERCEREE_API USEESaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // ── Slot Management ──

    // Manual slots: 0, 1, 2. Autosave slot: index 3.
    static constexpr int32 ManualSlotCount = 3;
    static constexpr int32 AutosaveSlotIndex = 3;
    static constexpr int32 TotalSlotCount = 4;

    UFUNCTION(BlueprintPure, Category="Save")
    FSEESaveSlotInfo GetSlotInfo(int32 SlotIndex) const;

    UFUNCTION(BlueprintPure, Category="Save")
    TArray<FSEESaveSlotInfo> GetAllSlotInfos() const;

    UFUNCTION(BlueprintPure, Category="Save")
    bool IsSlotOccupied(int32 SlotIndex) const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool DeleteSlot(int32 SlotIndex);

    // ── Save / Load ──

    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveToSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadFromSlot(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool Autosave();

    // ── Per-Car State ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetCarState(int32 CarIndex, const FSEECarState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool GetCarState(int32 CarIndex, FSEECarState& OutState) const;

    // ── Per-NPC State ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetNPCState(FName NPCID, const FSEENPCSaveState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool GetNPCState(FName NPCID, FSEENPCSaveState& OutState) const;

    // ── Quest State ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetQuestState(FName QuestID, const FSEEQuestSaveState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool GetQuestState(FName QuestID, FSEEQuestSaveState& OutState) const;

    // ── Faction Reputation ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetFactionReputation(ESEEFaction Faction, int32 Rep);

    UFUNCTION(BlueprintCallable, Category="Save")
    int32 GetFactionReputation(ESEEFaction Faction) const;

    // ── Player State ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetPlayerState(const FSEEPlayerSaveState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    FSEEPlayerSaveState GetPlayerState() const;

    // ── Difficulty (stored in save) ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetDifficulty(ESEEDifficulty NewDifficulty);

    UFUNCTION(BlueprintPure, Category="Save")
    ESEEDifficulty GetDifficulty() const { return CurrentDifficulty; }

    UFUNCTION(BlueprintPure, Category="Save")
    ESEEDifficulty GetLowestDifficultyPlayed() const { return LowestDifficultyPlayed; }

    // ── Play Time ──

    UFUNCTION(BlueprintPure, Category="Save")
    float GetPlayTimeSeconds() const { return PlayTimeSeconds; }

    void TickPlayTime(float DeltaTime) { PlayTimeSeconds += DeltaTime; }

    // ── Autosave Triggers ──

    UFUNCTION(BlueprintCallable, Category="Save")
    void OnCarEntered(int32 CarIndex);

    UFUNCTION(BlueprintCallable, Category="Save")
    void OnQuestCompleted(FName QuestID);

    UFUNCTION(BlueprintCallable, Category="Save")
    void OnMajorChoice();

    // ── Active Slot ──

    UFUNCTION(BlueprintPure, Category="Save")
    int32 GetActiveSlot() const { return ActiveSlotIndex; }

    // ── Events ──

    UPROPERTY(BlueprintAssignable, Category="Save")
    FOnSaveCompleted OnSaveCompleted;

    UPROPERTY(BlueprintAssignable, Category="Save")
    FOnLoadCompleted OnLoadCompleted;

private:
    FString GetSlotFileName(int32 SlotIndex) const;
    bool WriteToSlot(int32 SlotIndex);
    void PopulateSaveObject(USEESaveGameData* SaveObj) const;
    void RestoreFromSaveObject(const USEESaveGameData* SaveObj);
    bool TryAutosave();

    // Runtime state
    TMap<int32, FSEECarState> RuntimeCarStates;
    TMap<FName, FSEENPCSaveState> RuntimeNPCStates;
    TMap<FName, FSEEQuestSaveState> RuntimeQuestStates;
    TMap<uint8, int32> RuntimeFactionReps;
    FSEEPlayerSaveState RuntimePlayerState;

    ESEEDifficulty CurrentDifficulty = ESEEDifficulty::Survivor;
    ESEEDifficulty LowestDifficultyPlayed = ESEEDifficulty::Survivor;
    float PlayTimeSeconds = 0.0f;
    int32 ActiveSlotIndex = -1;
    int32 UserIndex = 0;

    // Autosave cooldown (60s between autosaves)
    float LastAutosaveTime = -60.0f;
    static constexpr float AutosaveCooldown = 60.0f;
};
