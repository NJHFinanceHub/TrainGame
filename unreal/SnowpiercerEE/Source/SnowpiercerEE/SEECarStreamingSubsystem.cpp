#include "SEECarStreamingSubsystem.h"
#include "Kismet/GameplayStatics.h"

DECLARE_STATS_GROUP(TEXT("CarStreaming"), STATGROUP_CarStreaming, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Car Streaming Tick"), STAT_CarStreamingTick, STATGROUP_CarStreaming);
DECLARE_DWORD_COUNTER_STAT(TEXT("Loaded Cars"), STAT_LoadedCars, STATGROUP_CarStreaming);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Streaming Memory MB"), STAT_StreamingMemoryMB, STATGROUP_CarStreaming);

TStatId USEECarStreamingSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USEECarStreamingSubsystem, STATGROUP_CarStreaming);
}

void USEECarStreamingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USEECarStreamingSubsystem::Deinitialize()
{
    CarInfoByIndex.Empty();
    LoadedCars.Empty();
    PreloadingCars.Empty();
    CurrentCarIndex = INDEX_NONE;
    PreviousCarIndex = INDEX_NONE;
    Super::Deinitialize();
}

void USEECarStreamingSubsystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_CarStreamingTick);

    SET_DWORD_STAT(STAT_LoadedCars, LoadedCars.Num());
    SET_FLOAT_STAT(STAT_StreamingMemoryMB, GetTotalLoadedMemoryMB());

    // Check memory budget
    float CurrentMemory = GetTotalLoadedMemoryMB();
    if (CurrentMemory > MemoryBudgetMB)
    {
        OnStreamingBudgetExceeded.Broadcast(CurrentMemory, MemoryBudgetMB);
    }
}

void USEECarStreamingSubsystem::RegisterCarLevel(int32 CarIndex, FName LevelName, float EstimatedMemoryMB)
{
    if (CarIndex < 0 || LevelName.IsNone())
    {
        return;
    }

    FCarStreamingInfo Info;
    Info.CarIndex = CarIndex;
    Info.LevelName = LevelName;
    Info.EstimatedMemoryMB = EstimatedMemoryMB;
    Info.State = ECarStreamingState::Unloaded;
    CarInfoByIndex.Add(CarIndex, Info);
}

void USEECarStreamingSubsystem::EnterCar(int32 CarIndex)
{
    if (CarIndex < 0)
    {
        return;
    }

    PreviousCarIndex = CurrentCarIndex;
    CurrentCarIndex = CarIndex;

    // Mark the new car as Active
    SetCarState(CarIndex, ECarStreamingState::Active);

    // Demote previous active car to Loaded
    if (PreviousCarIndex != INDEX_NONE && PreviousCarIndex != CarIndex)
    {
        SetCarState(PreviousCarIndex, ECarStreamingState::Loaded);
    }

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

ECarStreamingState USEECarStreamingSubsystem::GetCarState(int32 CarIndex) const
{
    if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
    {
        return Info->State;
    }
    return ECarStreamingState::Unloaded;
}

float USEECarStreamingSubsystem::GetTotalLoadedMemoryMB() const
{
    float Total = 0.0f;
    for (int32 CarIndex : LoadedCars)
    {
        if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
        {
            Total += Info->EstimatedMemoryMB;
        }
    }
    for (int32 CarIndex : PreloadingCars)
    {
        if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
        {
            Total += Info->EstimatedMemoryMB * 0.5f; // Partially loaded
        }
    }
    return Total;
}

void USEECarStreamingSubsystem::SetCarState(int32 CarIndex, ECarStreamingState NewState)
{
    if (FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
    {
        if (Info->State == NewState)
        {
            return;
        }

        ECarStreamingState OldState = Info->State;
        Info->State = NewState;

        if (NewState == ECarStreamingState::Preloading)
        {
            Info->LoadStartTime = FPlatformTime::Seconds();
        }
        else if (NewState == ECarStreamingState::Loaded && OldState == ECarStreamingState::Preloading)
        {
            Info->LoadCompleteTime = FPlatformTime::Seconds();
            LastCarLoadTimeMs = (Info->LoadCompleteTime - Info->LoadStartTime) * 1000.0;
            OnCarLoadComplete.Broadcast(CarIndex);
        }

        OnCarStreamingStateChanged.Broadcast(CarIndex, NewState);
    }
}

void USEECarStreamingSubsystem::RefreshStreamingSet()
{
    if (CurrentCarIndex == INDEX_NONE)
    {
        return;
    }

    // Desired set: current car + immediate neighbors
    TSet<int32> DesiredCars;
    DesiredCars.Add(CurrentCarIndex - 1);
    DesiredCars.Add(CurrentCarIndex);
    DesiredCars.Add(CurrentCarIndex + 1);

    // Determine travel direction for preloading
    int32 PreloadCarIndex = INDEX_NONE;
    if (PreviousCarIndex != INDEX_NONE && PreviousCarIndex != CurrentCarIndex)
    {
        int32 Direction = (CurrentCarIndex > PreviousCarIndex) ? 1 : -1;
        PreloadCarIndex = CurrentCarIndex + (Direction * 2);
    }
    else
    {
        // Default: preload forward
        PreloadCarIndex = CurrentCarIndex + 2;
    }

    // Unload cars no longer in the desired set
    for (int32 CarIndex : LoadedCars.Array())
    {
        if (!DesiredCars.Contains(CarIndex))
        {
            if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
            {
                StreamLevel(Info->LevelName, false);
                SetCarState(CarIndex, ECarStreamingState::Unloaded);
            }
            LoadedCars.Remove(CarIndex);
        }
    }

    // Cancel preloads that are no longer relevant
    for (int32 CarIndex : PreloadingCars.Array())
    {
        if (!DesiredCars.Contains(CarIndex) && CarIndex != PreloadCarIndex)
        {
            if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
            {
                StreamLevel(Info->LevelName, false);
                SetCarState(CarIndex, ECarStreamingState::Unloaded);
            }
            PreloadingCars.Remove(CarIndex);
        }
    }

    // Load desired cars that aren't yet loaded
    for (int32 CarIndex : DesiredCars)
    {
        if (LoadedCars.Contains(CarIndex))
        {
            continue;
        }

        if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
        {
            StreamLevel(Info->LevelName, true);
            LoadedCars.Add(CarIndex);

            if (CarIndex == CurrentCarIndex)
            {
                SetCarState(CarIndex, ECarStreamingState::Active);
            }
            else
            {
                SetCarState(CarIndex, ECarStreamingState::Loaded);
            }
        }
    }

    // Promote preloading cars that are now in desired set
    for (int32 CarIndex : PreloadingCars.Array())
    {
        if (DesiredCars.Contains(CarIndex))
        {
            LoadedCars.Add(CarIndex);
            PreloadingCars.Remove(CarIndex);
            SetCarState(CarIndex, ECarStreamingState::Loaded);
        }
    }

    // Begin preloading the next car in travel direction
    if (PreloadCarIndex != INDEX_NONE)
    {
        BeginPreload(PreloadCarIndex);
    }

    // Enforce max loaded count
    while (LoadedCars.Num() > MaxLoadedCars)
    {
        // Remove the car furthest from current
        int32 FurthestCar = INDEX_NONE;
        int32 MaxDist = 0;
        for (int32 CarIndex : LoadedCars)
        {
            int32 Dist = FMath::Abs(CarIndex - CurrentCarIndex);
            if (Dist > MaxDist)
            {
                MaxDist = Dist;
                FurthestCar = CarIndex;
            }
        }
        if (FurthestCar != INDEX_NONE)
        {
            if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(FurthestCar))
            {
                StreamLevel(Info->LevelName, false);
                SetCarState(FurthestCar, ECarStreamingState::Unloaded);
            }
            LoadedCars.Remove(FurthestCar);
        }
        else
        {
            break;
        }
    }
}

void USEECarStreamingSubsystem::BeginPreload(int32 CarIndex)
{
    if (CarIndex < 0 || LoadedCars.Contains(CarIndex) || PreloadingCars.Contains(CarIndex))
    {
        return;
    }

    if (const FCarStreamingInfo* Info = CarInfoByIndex.Find(CarIndex))
    {
        SetCarState(CarIndex, ECarStreamingState::Preloading);
        PreloadingCars.Add(CarIndex);
        StreamLevel(Info->LevelName, true);
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
    }
}
