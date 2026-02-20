#include "SEESaveGameSubsystem.h"
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
    return true;
}
