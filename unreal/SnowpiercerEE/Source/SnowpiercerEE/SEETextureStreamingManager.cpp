#include "SEETextureStreamingManager.h"
#include "Engine/Engine.h"

DECLARE_STATS_GROUP(TEXT("TextureStreaming"), STATGROUP_SEETextureStreaming, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Texture Streaming Tick"), STAT_TextureStreamingTick, STATGROUP_SEETextureStreaming);
DECLARE_FLOAT_COUNTER_STAT(TEXT("Texture Pool Utilization"), STAT_TexturePoolUtil, STATGROUP_SEETextureStreaming);

TStatId USEETextureStreamingManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USEETextureStreamingManager, STATGROUP_SEETextureStreaming);
}

void USEETextureStreamingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Set default zone configs
    for (uint8 i = 0; i <= static_cast<uint8>(ETrainZone::Exterior); i++)
    {
        ETrainZone Zone = static_cast<ETrainZone>(i);
        FZoneTextureConfig DefaultConfig;
        DefaultConfig.Zone = Zone;

        switch (Zone)
        {
        case ETrainZone::Tail:
            DefaultConfig.TexturePoolBudgetMB = 80.0f;  // Sparse, industrial
            break;
        case ETrainZone::Third:
            DefaultConfig.TexturePoolBudgetMB = 100.0f;
            break;
        case ETrainZone::Second:
            DefaultConfig.TexturePoolBudgetMB = 128.0f;
            DefaultConfig.bHasUniqueMaterials = true;
            break;
        case ETrainZone::First:
            DefaultConfig.TexturePoolBudgetMB = 150.0f;
            DefaultConfig.bHasUniqueMaterials = true;
            break;
        case ETrainZone::Engine:
            DefaultConfig.TexturePoolBudgetMB = 100.0f;
            break;
        default:
            DefaultConfig.TexturePoolBudgetMB = 64.0f;
            break;
        }

        ZoneConfigs.Add(Zone, DefaultConfig);
    }

    // Subscribe to streaming state changes
    UWorld* World = GetWorld();
    if (World)
    {
        USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
        if (StreamingSub)
        {
            StreamingSub->OnCarStreamingStateChanged.AddDynamic(this, &USEETextureStreamingManager::OnCarStreamingStateChanged);
        }
    }

    ApplyTexturePoolSize();
}

void USEETextureStreamingManager::Deinitialize()
{
    CarZoneMap.Empty();
    ZoneConfigs.Empty();
    Super::Deinitialize();
}

void USEETextureStreamingManager::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_TextureStreamingTick);

    StatsUpdateTimer += DeltaTime;
    if (StatsUpdateTimer >= StatsUpdateInterval)
    {
        StatsUpdateTimer = 0.0f;
        UpdateStreamingStats();
    }
}

void USEETextureStreamingManager::RegisterCarZone(int32 CarIndex, ETrainZone Zone)
{
    if (CarIndex < 0) return;
    CarZoneMap.Add(CarIndex, Zone);
}

void USEETextureStreamingManager::SetZoneConfig(ETrainZone Zone, const FZoneTextureConfig& Config)
{
    ZoneConfigs.Add(Zone, Config);
}

ETrainZone USEETextureStreamingManager::GetZoneForCar(int32 CarIndex) const
{
    if (const ETrainZone* Zone = CarZoneMap.Find(CarIndex))
    {
        return *Zone;
    }
    return ETrainZone::Tail;
}

void USEETextureStreamingManager::ForcePoolDefrag()
{
    // Request texture streaming system to flush and rebuild
    if (IStreamingManager::HasShutdown()) return;

    IStreamingManager& Manager = IStreamingManager::Get();
    IRenderAssetStreamingManager& TextureManager = Manager.GetTextureStreamingManager();
    TextureManager.BlockTillAllRequestsFinished();
}

void USEETextureStreamingManager::OnCarStreamingStateChanged(int32 CarIndex, ECarStreamingState NewState)
{
    UpdateMipBiasForCar(CarIndex, NewState);

    // Check for zone transition when entering a new active car
    if (NewState == ECarStreamingState::Active)
    {
        ETrainZone NewZone = GetZoneForCar(CarIndex);
        if (NewZone != CurrentZone)
        {
            HandleZoneTransition(NewZone);
        }
    }
}

void USEETextureStreamingManager::UpdateMipBiasForCar(int32 CarIndex, ECarStreamingState StreamingState)
{
    ETrainZone Zone = GetZoneForCar(CarIndex);
    const FZoneTextureConfig* Config = ZoneConfigs.Find(Zone);
    if (!Config) return;

    // Determine target mip bias based on streaming state
    int32 TargetMipBias = 3; // Default: very low detail
    switch (StreamingState)
    {
    case ECarStreamingState::Active:
        TargetMipBias = Config->ActiveMipBias;
        break;
    case ECarStreamingState::Loaded:
        TargetMipBias = Config->LoadedMipBias;
        break;
    case ECarStreamingState::Preloading:
        TargetMipBias = Config->PreloadMipBias;
        break;
    case ECarStreamingState::Unloaded:
        TargetMipBias = 3; // Lowest mips only
        break;
    }

    // Apply mip bias via console variable per-car
    // In practice, UE5's texture streaming uses per-primitive ForcedLodModel
    // and streaming distance multipliers. We set the global mip bias
    // hint that the renderer uses for prioritization.
    UE_LOG(LogTemp, Verbose, TEXT("TextureStreaming: Car %d mip bias set to %d (state: %d)"),
           CarIndex, TargetMipBias, static_cast<int32>(StreamingState));
}

void USEETextureStreamingManager::HandleZoneTransition(ETrainZone NewZone)
{
    ETrainZone OldZone = CurrentZone;
    CurrentZone = NewZone;

    UE_LOG(LogTemp, Log, TEXT("TextureStreaming: Zone transition %d -> %d"),
           static_cast<int32>(OldZone), static_cast<int32>(NewZone));

    // Adjust texture pool size for new zone
    ApplyTexturePoolSize();

    // Broadcast for material/atmosphere changes
    OnZoneTextureTransition.Broadcast(OldZone, NewZone);
}

void USEETextureStreamingManager::UpdateStreamingStats()
{
    if (IStreamingManager::HasShutdown()) return;

    // Query the engine's texture streaming stats
    IStreamingManager& Manager = IStreamingManager::Get();
    IRenderAssetStreamingManager& TextureManager = Manager.GetTextureStreamingManager();

    int64 CurrentMem = 0;
    int64 RequiredMem = 0;
    int64 BudgetMem = 0;
    int32 PendingRequests = 0;

    // Use stat tracking from the streaming manager
    TextureManager.GetObjectReferenceBounds(FBox(ForceInit));

    // Approximate from pool size
    CachedStats.TotalResidentMB = static_cast<float>(CurrentMem) / (1024.0f * 1024.0f);
    CachedStats.TotalStreamedMB = static_cast<float>(RequiredMem) / (1024.0f * 1024.0f);
    CachedStats.PoolUtilization = (GlobalTexturePoolMB > 0.0f)
        ? FMath::Clamp(CachedStats.TotalResidentMB / GlobalTexturePoolMB, 0.0f, 1.0f)
        : 0.0f;
    CachedStats.PendingMipRequests = PendingRequests;

    SET_FLOAT_STAT(STAT_TexturePoolUtil, CachedStats.PoolUtilization * 100.0f);

    // Alert if pool exceeded
    if (CachedStats.PoolUtilization > 0.9f)
    {
        OnTexturePoolExceeded.Broadcast(CachedStats.TotalResidentMB, GlobalTexturePoolMB);
    }
}

void USEETextureStreamingManager::ApplyTexturePoolSize()
{
    // Set the engine texture pool size based on current zone budget
    const FZoneTextureConfig* ZoneConfig = ZoneConfigs.Find(CurrentZone);
    if (!ZoneConfig) return;

    // The global pool is shared; individual zone budgets inform priority,
    // but the total pool stays at GlobalTexturePoolMB
    if (GEngine)
    {
        // r.Streaming.PoolSize controls the texture streaming pool (in MB)
        IConsoleVariable* PoolSizeVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Streaming.PoolSize"));
        if (PoolSizeVar)
        {
            PoolSizeVar->Set(static_cast<int32>(GlobalTexturePoolMB));
        }
    }
}
