#include "SEECarLODSubsystem.h"

DECLARE_STATS_GROUP(TEXT("CarLOD"), STATGROUP_CarLOD, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Car LOD Update"), STAT_CarLODUpdate, STATGROUP_CarLOD);

TStatId USEECarLODSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USEECarLODSubsystem, STATGROUP_CarLOD);
}

void USEECarLODSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void USEECarLODSubsystem::Deinitialize()
{
    CarLODEntries.Empty();
    Super::Deinitialize();
}

void USEECarLODSubsystem::Tick(float DeltaTime)
{
    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer < LODUpdateInterval)
    {
        return;
    }
    LODUpdateTimer = 0.0f;

    SCOPE_CYCLE_COUNTER(STAT_CarLODUpdate);

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
    if (!StreamingSub)
    {
        return;
    }

    int32 NewCurrentCar = StreamingSub->GetCurrentCarIndex();
    if (NewCurrentCar != CachedCurrentCarIndex)
    {
        CachedCurrentCarIndex = NewCurrentCar;
        RefreshAllLODs();
    }
}

void USEECarLODSubsystem::RegisterCarLOD(int32 CarIndex, TSoftObjectPtr<UStaticMesh> ExteriorShell,
                                          TSoftObjectPtr<UTexture2D> Impostor, FLinearColor Tint)
{
    FCarLODEntry Entry;
    Entry.CarIndex = CarIndex;
    Entry.ExteriorShellMesh = ExteriorShell;
    Entry.ImpostorTexture = Impostor;
    Entry.ExteriorTint = Tint;
    Entry.CurrentLOD = ECarLODLevel::LOD3_Impostor;
    CarLODEntries.Add(CarIndex, Entry);
}

ECarLODLevel USEECarLODSubsystem::GetCarLODLevel(int32 CarIndex) const
{
    if (const FCarLODEntry* Entry = CarLODEntries.Find(CarIndex))
    {
        return Entry->CurrentLOD;
    }
    return ECarLODLevel::LOD3_Impostor;
}

ECarLODLevel USEECarLODSubsystem::CalculateLODForDistance(int32 CarDistance) const
{
    if (CarDistance == 0)
    {
        return ECarLODLevel::LOD0_Full;
    }
    if (CarDistance <= Config.LOD1_MaxCarDistance)
    {
        return ECarLODLevel::LOD1_Reduced;
    }
    if (CarDistance <= Config.LOD2_MaxCarDistance)
    {
        return ECarLODLevel::LOD2_Shell;
    }
    return ECarLODLevel::LOD3_Impostor;
}

void USEECarLODSubsystem::SetLODConfig(const FCarLODConfig& NewConfig)
{
    Config = NewConfig;
    RefreshAllLODs();
}

void USEECarLODSubsystem::RefreshAllLODs()
{
    if (CachedCurrentCarIndex == INDEX_NONE)
    {
        return;
    }

    for (auto& Pair : CarLODEntries)
    {
        int32 CarIndex = Pair.Key;
        FCarLODEntry& Entry = Pair.Value;

        int32 Distance = FMath::Abs(CarIndex - CachedCurrentCarIndex);
        ECarLODLevel NewLOD;

        // Cars beyond max visible range get culled (LOD3 impostor)
        if (Distance > Config.MaxVisibleExteriorCars)
        {
            NewLOD = ECarLODLevel::LOD3_Impostor;
        }
        else
        {
            NewLOD = CalculateLODForDistance(Distance);
        }

        if (NewLOD != Entry.CurrentLOD)
        {
            Entry.CurrentLOD = NewLOD;
            OnCarLODChanged.Broadcast(CarIndex, NewLOD);
        }
    }
}
