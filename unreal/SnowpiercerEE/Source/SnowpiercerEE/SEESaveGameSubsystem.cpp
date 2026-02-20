#include "SEESaveGameSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USEESaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

// ── Slot Management ──

FString USEESaveGameSubsystem::GetSlotFileName(int32 SlotIndex) const
{
    if (SlotIndex == AutosaveSlotIndex)
    {
        return TEXT("SnowpiercerEE_Auto");
    }
    return FString::Printf(TEXT("SnowpiercerEE_Slot%d"), SlotIndex);
}

FSEESaveSlotInfo USEESaveGameSubsystem::GetSlotInfo(int32 SlotIndex) const
{
    FSEESaveSlotInfo Info;
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount)
    {
        return Info;
    }

    Info.SlotName = GetSlotFileName(SlotIndex);
    Info.SlotType = (SlotIndex == AutosaveSlotIndex)
        ? ESEESaveSlotType::Autosave
        : ESEESaveSlotType::Manual;

    if (!UGameplayStatics::DoesSaveGameExist(Info.SlotName, UserIndex))
    {
        return Info;
    }

    USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
        UGameplayStatics::LoadGameFromSlot(Info.SlotName, UserIndex));
    if (SaveObj)
    {
        Info.bOccupied = true;
        Info.Timestamp = SaveObj->Timestamp;
        Info.PlayTimeSeconds = SaveObj->PlayTimeSeconds;
        Info.PlayerLevel = SaveObj->PlayerState.Level;
        Info.Difficulty = SaveObj->Difficulty;

        // Derive zone name from car index
        const int32 CarIdx = SaveObj->PlayerState.CurrentCarIndex;
        if (CarIdx < 15) Info.CurrentZoneName = TEXT("Tail");
        else if (CarIdx < 30) Info.CurrentZoneName = TEXT("Third Class");
        else if (CarIdx < 50) Info.CurrentZoneName = TEXT("Second Class");
        else if (CarIdx < 70) Info.CurrentZoneName = TEXT("First Class");
        else Info.CurrentZoneName = TEXT("Engine");
    }

    return Info;
}

TArray<FSEESaveSlotInfo> USEESaveGameSubsystem::GetAllSlotInfos() const
{
    TArray<FSEESaveSlotInfo> Infos;
    for (int32 i = 0; i < TotalSlotCount; ++i)
    {
        Infos.Add(GetSlotInfo(i));
    }
    return Infos;
}

bool USEESaveGameSubsystem::IsSlotOccupied(int32 SlotIndex) const
{
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount) return false;
    return UGameplayStatics::DoesSaveGameExist(GetSlotFileName(SlotIndex), UserIndex);
}

bool USEESaveGameSubsystem::DeleteSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount) return false;
    const FString SlotName = GetSlotFileName(SlotIndex);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex)) return false;
    return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

// ── Save / Load ──

void USEESaveGameSubsystem::PopulateSaveObject(USEESaveGameData* SaveObj) const
{
    SaveObj->SaveVersion = 1;
    SaveObj->Timestamp = FDateTime::UtcNow();
    SaveObj->PlayTimeSeconds = PlayTimeSeconds;
    SaveObj->Difficulty = CurrentDifficulty;
    SaveObj->LowestDifficultyPlayed = LowestDifficultyPlayed;
    SaveObj->PlayerState = RuntimePlayerState;
    SaveObj->CarStates = RuntimeCarStates;
    SaveObj->NPCStates = RuntimeNPCStates;
    SaveObj->QuestStates = RuntimeQuestStates;
    SaveObj->FactionReputations = RuntimeFactionReps;
}

void USEESaveGameSubsystem::RestoreFromSaveObject(const USEESaveGameData* SaveObj)
{
    PlayTimeSeconds = SaveObj->PlayTimeSeconds;
    CurrentDifficulty = SaveObj->Difficulty;
    LowestDifficultyPlayed = SaveObj->LowestDifficultyPlayed;
    RuntimePlayerState = SaveObj->PlayerState;
    RuntimeCarStates = SaveObj->CarStates;
    RuntimeNPCStates = SaveObj->NPCStates;
    RuntimeQuestStates = SaveObj->QuestStates;
    RuntimeFactionReps = SaveObj->FactionReputations;
}

bool USEESaveGameSubsystem::WriteToSlot(int32 SlotIndex)
{
    USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
        UGameplayStatics::CreateSaveGameObject(USEESaveGameData::StaticClass()));
    if (!SaveObj) return false;

    PopulateSaveObject(SaveObj);
    const FString SlotName = GetSlotFileName(SlotIndex);
    return UGameplayStatics::SaveGameToSlot(SaveObj, SlotName, UserIndex);
}

bool USEESaveGameSubsystem::SaveToSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= ManualSlotCount) return false;

    const bool bSuccess = WriteToSlot(SlotIndex);
    ActiveSlotIndex = SlotIndex;
    OnSaveCompleted.Broadcast(bSuccess);
    return bSuccess;
}

bool USEESaveGameSubsystem::LoadFromSlot(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= TotalSlotCount) return false;

    const FString SlotName = GetSlotFileName(SlotIndex);
    if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        OnLoadCompleted.Broadcast(false);
        return false;
    }

    USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
        UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
    if (!SaveObj)
    {
        OnLoadCompleted.Broadcast(false);
        return false;
    }

    RestoreFromSaveObject(SaveObj);
    ActiveSlotIndex = SlotIndex;
    OnLoadCompleted.Broadcast(true);
    return true;
}

bool USEESaveGameSubsystem::Autosave()
{
    return TryAutosave();
}

bool USEESaveGameSubsystem::TryAutosave()
{
    const UWorld* World = GetGameInstance()->GetWorld();
    if (!World) return false;

    const float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastAutosaveTime < AutosaveCooldown)
    {
        return false;
    }

    const bool bSuccess = WriteToSlot(AutosaveSlotIndex);
    if (bSuccess)
    {
        LastAutosaveTime = CurrentTime;
    }
    OnSaveCompleted.Broadcast(bSuccess);
    return bSuccess;
}

// ── Per-Car State ──

void USEESaveGameSubsystem::SetCarState(int32 CarIndex, const FSEECarState& State)
{
    if (CarIndex < 0) return;
    RuntimeCarStates.Add(CarIndex, State);
}

bool USEESaveGameSubsystem::GetCarState(int32 CarIndex, FSEECarState& OutState) const
{
    if (const FSEECarState* Found = RuntimeCarStates.Find(CarIndex))
    {
        OutState = *Found;
        return true;
    }
    return false;
}

// ── Per-NPC State ──

void USEESaveGameSubsystem::SetNPCState(FName NPCID, const FSEENPCSaveState& State)
{
    RuntimeNPCStates.Add(NPCID, State);
}

bool USEESaveGameSubsystem::GetNPCState(FName NPCID, FSEENPCSaveState& OutState) const
{
    if (const FSEENPCSaveState* Found = RuntimeNPCStates.Find(NPCID))
    {
        OutState = *Found;
        return true;
    }
    return false;
}

// ── Quest State ──

void USEESaveGameSubsystem::SetQuestState(FName QuestID, const FSEEQuestSaveState& State)
{
    RuntimeQuestStates.Add(QuestID, State);
}

bool USEESaveGameSubsystem::GetQuestState(FName QuestID, FSEEQuestSaveState& OutState) const
{
    if (const FSEEQuestSaveState* Found = RuntimeQuestStates.Find(QuestID))
    {
        OutState = *Found;
        return true;
    }
    return false;
}

// ── Faction Reputation ──

void USEESaveGameSubsystem::SetFactionReputation(ESEEFaction Faction, int32 Rep)
{
    RuntimeFactionReps.Add(static_cast<uint8>(Faction), FMath::Clamp(Rep, -100, 100));
}

int32 USEESaveGameSubsystem::GetFactionReputation(ESEEFaction Faction) const
{
    if (const int32* Found = RuntimeFactionReps.Find(static_cast<uint8>(Faction)))
    {
        return *Found;
    }
    return 0;
}

// ── Player State ──

void USEESaveGameSubsystem::SetPlayerState(const FSEEPlayerSaveState& State)
{
    RuntimePlayerState = State;
}

FSEEPlayerSaveState USEESaveGameSubsystem::GetPlayerState() const
{
    return RuntimePlayerState;
}

// ── Difficulty ──

void USEESaveGameSubsystem::SetDifficulty(ESEEDifficulty NewDifficulty)
{
    CurrentDifficulty = NewDifficulty;

    // Track lowest difficulty ever played (for achievement gating)
    if (static_cast<uint8>(NewDifficulty) < static_cast<uint8>(LowestDifficultyPlayed))
    {
        LowestDifficultyPlayed = NewDifficulty;
    }
}

// ── Autosave Triggers ──

void USEESaveGameSubsystem::OnCarEntered(int32 CarIndex)
{
    // Update player car index and trigger autosave
    RuntimePlayerState.CurrentCarIndex = CarIndex;

    // Mark car as visited
    FSEECarState CarState;
    GetCarState(CarIndex, CarState);
    CarState.bVisited = true;
    SetCarState(CarIndex, CarState);

    TryAutosave();
}

void USEESaveGameSubsystem::OnQuestCompleted(FName QuestID)
{
    TryAutosave();
}

void USEESaveGameSubsystem::OnMajorChoice()
{
    TryAutosave();
}
