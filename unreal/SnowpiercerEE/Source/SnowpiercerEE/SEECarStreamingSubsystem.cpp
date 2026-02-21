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

void USEECarStreamingSubsystem::RegisterZone1Cars()
{
    // Zone 1 (The Tail) — 15 streaming sublevels built by Scripts/build_zone1.py
    // Each sublevel lives under /Game/Maps/Zone1/ with actors at world-space positions.
    static const struct { int32 Index; const TCHAR* Name; } Zone1Cars[] =
    {
        {  0, TEXT("Z1_Car00_Caboose")         },
        {  1, TEXT("Z1_Car01_Tail_Quarters_A")  },
        {  2, TEXT("Z1_Car02_Tail_Quarters_B")  },
        {  3, TEXT("Z1_Car03_The_Pit")          },
        {  4, TEXT("Z1_Car04_Nursery")          },
        {  5, TEXT("Z1_Car05_Elders_Car")       },
        {  6, TEXT("Z1_Car06_Sickbay")          },
        {  7, TEXT("Z1_Car07_Workshop")         },
        {  8, TEXT("Z1_Car08_Listening_Post")   },
        {  9, TEXT("Z1_Car09_Blockade")         },
        { 10, TEXT("Z1_Car10_Dark_Car")         },
        { 11, TEXT("Z1_Car11_Freezer_Breach")   },
        { 12, TEXT("Z1_Car12_Kronole_Den")      },
        { 13, TEXT("Z1_Car13_Smugglers_Cache")  },
        { 14, TEXT("Z1_Car14_Martyrs_Gate")     },
    };

    for (const auto& Car : Zone1Cars)
    {
        RegisterCarLevel(Car.Index, FName(Car.Name));
    }

    UE_LOG(LogTemp, Log, TEXT("SEECarStreaming: Registered %d Zone 1 car sublevels"), CarLevelByIndex.Num());
}

void USEECarStreamingSubsystem::RegisterCarLevel(int32 CarIndex, FName LevelName)
{
    if (CarIndex < 0 || LevelName.IsNone())
    {
        return;
    }

    CarLevelByIndex.Add(CarIndex, LevelName);
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
