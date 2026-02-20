#include "SEESaveGameSubsystem.h"
#include "Endings/SEELedgerSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USEESaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadFromSlot();
}

void USEESaveGameSubsystem::SetCarState(int32 CarIndex, const FSEECarState& State)
{
    if (CarIndex < 0)
    {
        return;
    }

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

bool USEESaveGameSubsystem::WriteToSlot()
{
    USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
        UGameplayStatics::CreateSaveGameObject(USEESaveGameData::StaticClass()));

    if (!SaveObj)
    {
        return false;
    }

    SaveObj->CarStates = RuntimeCarStates;

    // Persist Ledger data
    USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
    if (Ledger)
    {
        SaveObj->LedgerScores = Ledger->GetScoresForSave();
        SaveObj->GlobalFlags = Ledger->GetGlobalFlagsForSave();
        SaveObj->GlobalIntFlags = Ledger->GetGlobalIntFlagsForSave();
        SaveObj->ChoiceHistory = Ledger->GetHistoryForSave();
    }

    return UGameplayStatics::SaveGameToSlot(SaveObj, SaveSlotName, UserIndex);
}

bool USEESaveGameSubsystem::LoadFromSlot()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
    {
        return false;
    }

    USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
        UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

    if (!SaveObj)
    {
        return false;
    }

    RuntimeCarStates = SaveObj->CarStates;

    // Restore Ledger data
    USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
    if (Ledger)
    {
        Ledger->LoadScoresFromSave(SaveObj->LedgerScores);
        Ledger->LoadGlobalFlagsFromSave(SaveObj->GlobalFlags);
        Ledger->LoadGlobalIntFlagsFromSave(SaveObj->GlobalIntFlags);
        Ledger->LoadHistoryFromSave(SaveObj->ChoiceHistory);
    }

    return true;
}
