#include "SEECarStreamingSubsystem.h"
#include "Kismet/GameplayStatics.h"

void USEECarStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USEECarStreamingSubsystem::Deinitialize()
{
    CarLevelByIndex.Empty();
    LoadedCars.Empty();
    CurrentCarIndex = INDEX_NONE;
    Super::Deinitialize();
}

void USEECarStreamingSubsystem::RegisterCarLevel(int32 CarIndex, FName LevelName)
{
    if (CarIndex < 0 || LevelName.IsNone())
    {
        return;
    }

    CarLevelByIndex.Add(CarIndex, LevelName);
}

void USEECarStreamingSubsystem::RegisterFromDataTable(UDataTable* CarDataTable)
{
    if (!CarDataTable)
    {
        return;
    }

    TArray<FSEECarData*> AllRows;
    CarDataTable->GetAllRows<FSEECarData>(TEXT("RegisterFromDataTable"), AllRows);

    for (const FSEECarData* Row : AllRows)
    {
        if (Row && !Row->SubLevelName.IsNone())
        {
            RegisterCarLevel(Row->CarIndex, Row->SubLevelName);
        }
    }
}

void USEECarStreamingSubsystem::EnterCar(int32 CarIndex)
{
    if (CarIndex < 0)
    {
        return;
    }

    CurrentCarIndex = CarIndex;

    if (bDoorLoadMaskActive)
    {
        bRefreshPending = true;
        return;
    }

    RefreshStreamingSet();
}

void USEECarStreamingSubsystem::SetDoorLoadMaskActive(bool bActive)
{
    bDoorLoadMaskActive = bActive;

    if (!bDoorLoadMaskActive && bRefreshPending)
    {
        bRefreshPending = false;
        RefreshStreamingSet();
    }
}

void USEECarStreamingSubsystem::RefreshStreamingSet()
{
    if (CurrentCarIndex == INDEX_NONE)
    {
        return;
    }

    TSet<int32> DesiredCars;
    DesiredCars.Add(CurrentCarIndex - 1);
    DesiredCars.Add(CurrentCarIndex);
    DesiredCars.Add(CurrentCarIndex + 1);

    for (int32 CarIndex : LoadedCars)
    {
        if (!DesiredCars.Contains(CarIndex))
        {
            if (const FName* Level = CarLevelByIndex.Find(CarIndex))
            {
                StreamLevel(*Level, false);
            }
        }
    }

    for (int32 CarIndex : DesiredCars)
    {
        if (LoadedCars.Contains(CarIndex))
        {
            continue;
        }

        if (const FName* Level = CarLevelByIndex.Find(CarIndex))
        {
            StreamLevel(*Level, true);
            LoadedCars.Add(CarIndex);
        }
    }

    for (int32 CarIndex : LoadedCars.Array())
    {
        if (!DesiredCars.Contains(CarIndex))
        {
            LoadedCars.Remove(CarIndex);
        }
    }

    while (LoadedCars.Num() > MaxLoadedCars)
    {
        LoadedCars.Remove(*LoadedCars.CreateIterator());
    }
}

void USEECarStreamingSubsystem::StreamLevel(FName LevelName, bool bShouldLoad)
{
    if (LevelName.IsNone() || !GetWorld())
    {
        return;
    }

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = nullptr;
    LatentInfo.ExecutionFunction = NAME_None;
    LatentInfo.Linkage = 0;
    LatentInfo.UUID = GetTypeHash(LevelName);

    if (bShouldLoad)
    {
        UGameplayStatics::LoadStreamLevel(this, LevelName, true, false, LatentInfo);
    }
    else
    {
        UGameplayStatics::UnloadStreamLevel(this, LevelName, LatentInfo, false);
        for (int32 CarIndex : LoadedCars.Array())
        {
            if (CarLevelByIndex.FindRef(CarIndex) == LevelName)
            {
                LoadedCars.Remove(CarIndex);
                break;
            }
        }
    }
}
