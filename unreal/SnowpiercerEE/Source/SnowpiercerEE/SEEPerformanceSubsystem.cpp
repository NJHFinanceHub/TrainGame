#include "SEEPerformanceSubsystem.h"
#include "SEECarStreamingSubsystem.h"
#include "SEENPCPoolSubsystem.h"
#include "SEETextureStreamingManager.h"
#include "Engine/Engine.h"
#include "Engine/Canvas.h"
#include "HAL/PlatformMemory.h"

DECLARE_STATS_GROUP(TEXT("SEEPerformance"), STATGROUP_SEEPerformance, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Performance Monitor Tick"), STAT_PerfMonitorTick, STATGROUP_SEEPerformance);

TStatId USEEPerformanceSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(USEEPerformanceSubsystem, STATGROUP_SEEPerformance);
}

void USEEPerformanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    SnapshotHistory.Reserve(MaxHistorySize);
    RegisterConsoleCommands();
}

void USEEPerformanceSubsystem::Deinitialize()
{
    // Unregister console commands
    for (IConsoleObject* Cmd : ConsoleCommands)
    {
        IConsoleManager::Get().UnregisterConsoleObject(Cmd);
    }
    ConsoleCommands.Empty();
    SnapshotHistory.Empty();
    Super::Deinitialize();
}

void USEEPerformanceSubsystem::Tick(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_PerfMonitorTick);

    SamplePerformance();

    BudgetCheckTimer += DeltaTime;
    if (BudgetCheckTimer >= BudgetCheckInterval)
    {
        BudgetCheckTimer = 0.0f;
        CheckBudgets();
    }

    if (bDebugOverlayEnabled)
    {
        DrawDebugOverlay();
    }
}

void USEEPerformanceSubsystem::SamplePerformance()
{
    UWorld* World = GetWorld();
    if (!World) return;

    FPerformanceSnapshot Snapshot;

    // Frame timing
    Snapshot.FrameTimeMs = FApp::GetDeltaTime() * 1000.0f;
    Snapshot.FPS = (Snapshot.FrameTimeMs > 0.0f) ? 1000.0f / Snapshot.FrameTimeMs : 0.0f;

    // Thread timing from stat system
    Snapshot.GameThreadMs = FPlatformTime::ToMilliseconds(GGameThreadTime);
    Snapshot.RenderThreadMs = FPlatformTime::ToMilliseconds(GRenderThreadTime);

    // Memory
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Snapshot.UsedPhysicalMemoryMB = static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
    Snapshot.UsedVirtualMemoryMB = static_cast<float>(MemStats.UsedVirtual) / (1024.0f * 1024.0f);

    // Car streaming stats
    USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
    if (StreamingSub)
    {
        Snapshot.LoadedCarCount = StreamingSub->GetLoadedCarCount();
        Snapshot.StreamingMemoryMB = StreamingSub->GetTotalLoadedMemoryMB();
    }

    // NPC pool stats
    USEENPCPoolSubsystem* NPCPool = World->GetSubsystem<USEENPCPoolSubsystem>();
    if (NPCPool)
    {
        Snapshot.ActiveNPCCount = NPCPool->GetTotalPooledNPCs();
    }

    // Texture streaming stats
    USEETextureStreamingManager* TexManager = World->GetSubsystem<USEETextureStreamingManager>();
    if (TexManager)
    {
        Snapshot.TexturePoolUtilization = TexManager->GetStreamingStats().PoolUtilization;
    }

    CurrentSnapshot = Snapshot;

    // Add to history ring buffer
    if (SnapshotHistory.Num() >= MaxHistorySize)
    {
        SnapshotHistory.RemoveAt(0, 1, EAllowShrinking::No);
    }
    SnapshotHistory.Add(Snapshot);
}

FPerformanceSnapshot USEEPerformanceSubsystem::GetAverageSnapshot() const
{
    FPerformanceSnapshot Avg;
    if (SnapshotHistory.Num() == 0) return Avg;

    for (const FPerformanceSnapshot& S : SnapshotHistory)
    {
        Avg.FrameTimeMs += S.FrameTimeMs;
        Avg.GameThreadMs += S.GameThreadMs;
        Avg.RenderThreadMs += S.RenderThreadMs;
        Avg.GPUTimeMs += S.GPUTimeMs;
        Avg.FPS += S.FPS;
        Avg.UsedPhysicalMemoryMB += S.UsedPhysicalMemoryMB;
        Avg.DrawCalls += S.DrawCalls;
        Avg.TrianglesRendered += S.TrianglesRendered;
        Avg.ActiveNPCCount += S.ActiveNPCCount;
        Avg.LoadedCarCount += S.LoadedCarCount;
        Avg.StreamingMemoryMB += S.StreamingMemoryMB;
        Avg.TexturePoolUtilization += S.TexturePoolUtilization;
    }

    float N = static_cast<float>(SnapshotHistory.Num());
    Avg.FrameTimeMs /= N;
    Avg.GameThreadMs /= N;
    Avg.RenderThreadMs /= N;
    Avg.GPUTimeMs /= N;
    Avg.FPS /= N;
    Avg.UsedPhysicalMemoryMB /= N;
    Avg.DrawCalls = static_cast<int32>(Avg.DrawCalls / N);
    Avg.TrianglesRendered = static_cast<int32>(Avg.TrianglesRendered / N);
    Avg.ActiveNPCCount = static_cast<int32>(Avg.ActiveNPCCount / N);
    Avg.LoadedCarCount = static_cast<int32>(Avg.LoadedCarCount / N);
    Avg.StreamingMemoryMB /= N;
    Avg.TexturePoolUtilization /= N;

    return Avg;
}

FPerformanceSnapshot USEEPerformanceSubsystem::GetWorstSnapshot() const
{
    FPerformanceSnapshot Worst;
    for (const FPerformanceSnapshot& S : SnapshotHistory)
    {
        if (S.FrameTimeMs > Worst.FrameTimeMs) Worst.FrameTimeMs = S.FrameTimeMs;
        if (S.GameThreadMs > Worst.GameThreadMs) Worst.GameThreadMs = S.GameThreadMs;
        if (S.RenderThreadMs > Worst.RenderThreadMs) Worst.RenderThreadMs = S.RenderThreadMs;
        if (S.GPUTimeMs > Worst.GPUTimeMs) Worst.GPUTimeMs = S.GPUTimeMs;
        if (S.UsedPhysicalMemoryMB > Worst.UsedPhysicalMemoryMB) Worst.UsedPhysicalMemoryMB = S.UsedPhysicalMemoryMB;
        if (S.DrawCalls > Worst.DrawCalls) Worst.DrawCalls = S.DrawCalls;
        if (S.ActiveNPCCount > Worst.ActiveNPCCount) Worst.ActiveNPCCount = S.ActiveNPCCount;
        if (S.StreamingMemoryMB > Worst.StreamingMemoryMB) Worst.StreamingMemoryMB = S.StreamingMemoryMB;
    }
    return Worst;
}

void USEEPerformanceSubsystem::SetBudget(const FPerformanceBudget& NewBudget)
{
    Budget = NewBudget;
}

TArray<FName> USEEPerformanceSubsystem::CheckBudgetViolations() const
{
    TArray<FName> Violations;
    FPerformanceSnapshot Avg = GetAverageSnapshot();

    if (Avg.FrameTimeMs > Budget.TargetFrameTimeMs)
        Violations.Add(FName(TEXT("FrameTime")));
    if (Avg.GameThreadMs > Budget.MaxGameThreadMs)
        Violations.Add(FName(TEXT("GameThread")));
    if (Avg.RenderThreadMs > Budget.MaxRenderThreadMs)
        Violations.Add(FName(TEXT("RenderThread")));
    if (Avg.StreamingMemoryMB > Budget.MaxStreamingMemoryMB)
        Violations.Add(FName(TEXT("StreamingMemory")));
    if (Avg.TexturePoolUtilization > Budget.MaxTexturePoolUtilization)
        Violations.Add(FName(TEXT("TexturePool")));
    if (Avg.DrawCalls > Budget.MaxDrawCalls)
        Violations.Add(FName(TEXT("DrawCalls")));
    if (Avg.ActiveNPCCount > Budget.MaxActiveNPCs)
        Violations.Add(FName(TEXT("ActiveNPCs")));

    return Violations;
}

void USEEPerformanceSubsystem::SetDebugOverlayEnabled(bool bEnabled)
{
    bDebugOverlayEnabled = bEnabled;
}

void USEEPerformanceSubsystem::LogPerformanceReport() const
{
    FPerformanceSnapshot Avg = GetAverageSnapshot();
    FPerformanceSnapshot Worst = GetWorstSnapshot();
    TArray<FName> Violations = CheckBudgetViolations();

    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time:  Avg %.2f ms | Worst %.2f ms | Budget %.2f ms"),
           Avg.FrameTimeMs, Worst.FrameTimeMs, Budget.TargetFrameTimeMs);
    UE_LOG(LogTemp, Log, TEXT("FPS:         Avg %.1f"),
           Avg.FPS);
    UE_LOG(LogTemp, Log, TEXT("Game Thread: Avg %.2f ms | Worst %.2f ms | Budget %.2f ms"),
           Avg.GameThreadMs, Worst.GameThreadMs, Budget.MaxGameThreadMs);
    UE_LOG(LogTemp, Log, TEXT("Render:      Avg %.2f ms | Worst %.2f ms | Budget %.2f ms"),
           Avg.RenderThreadMs, Worst.RenderThreadMs, Budget.MaxRenderThreadMs);
    UE_LOG(LogTemp, Log, TEXT("Memory:      Physical %.1f MB | Virtual %.1f MB"),
           Avg.UsedPhysicalMemoryMB, Avg.UsedVirtualMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Streaming:   %.1f MB loaded | %d cars | Budget %.1f MB"),
           Avg.StreamingMemoryMB, Avg.LoadedCarCount, Budget.MaxStreamingMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Texture Pool: %.1f%% utilization | Budget %.1f%%"),
           Avg.TexturePoolUtilization * 100.0f, Budget.MaxTexturePoolUtilization * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("NPCs:        %d active | Budget %d"),
           Avg.ActiveNPCCount, Budget.MaxActiveNPCs);
    UE_LOG(LogTemp, Log, TEXT("Draw Calls:  %d | Budget %d"),
           Avg.DrawCalls, Budget.MaxDrawCalls);

    if (Violations.Num() > 0)
    {
        FString ViolationStr;
        for (const FName& V : Violations)
        {
            ViolationStr += V.ToString() + TEXT(", ");
        }
        UE_LOG(LogTemp, Warning, TEXT("BUDGET VIOLATIONS: %s"), *ViolationStr);
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("All budgets within limits."));
    }
    UE_LOG(LogTemp, Log, TEXT("=========================="));
}

void USEEPerformanceSubsystem::CheckBudgets()
{
    TArray<FName> Violations = CheckBudgetViolations();
    for (const FName& V : Violations)
    {
        float Value = 0.0f;
        FPerformanceSnapshot Avg = GetAverageSnapshot();

        if (V == FName(TEXT("FrameTime"))) Value = Avg.FrameTimeMs;
        else if (V == FName(TEXT("GameThread"))) Value = Avg.GameThreadMs;
        else if (V == FName(TEXT("RenderThread"))) Value = Avg.RenderThreadMs;
        else if (V == FName(TEXT("StreamingMemory"))) Value = Avg.StreamingMemoryMB;
        else if (V == FName(TEXT("TexturePool"))) Value = Avg.TexturePoolUtilization;
        else if (V == FName(TEXT("DrawCalls"))) Value = static_cast<float>(Avg.DrawCalls);
        else if (V == FName(TEXT("ActiveNPCs"))) Value = static_cast<float>(Avg.ActiveNPCCount);

        OnBudgetViolation.Broadcast(V, Value);
    }
}

void USEEPerformanceSubsystem::DrawDebugOverlay() const
{
    if (!GEngine) return;

    FPerformanceSnapshot Avg = GetAverageSnapshot();

    auto BudgetColor = [](float Current, float Budget) -> FColor
    {
        float Ratio = (Budget > 0.0f) ? Current / Budget : 0.0f;
        if (Ratio < 0.75f) return FColor::Green;
        if (Ratio < 0.9f) return FColor::Yellow;
        return FColor::Red;
    };

    int32 Line = 0;
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f, FColor::White,
        FString::Printf(TEXT("=== TRAINGAME PERF ===  %.1f FPS (%.2f ms)"), Avg.FPS, Avg.FrameTimeMs));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f,
        BudgetColor(Avg.GameThreadMs, Budget.MaxGameThreadMs),
        FString::Printf(TEXT("Game Thread: %.2f / %.2f ms"), Avg.GameThreadMs, Budget.MaxGameThreadMs));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f,
        BudgetColor(Avg.RenderThreadMs, Budget.MaxRenderThreadMs),
        FString::Printf(TEXT("Render:      %.2f / %.2f ms"), Avg.RenderThreadMs, Budget.MaxRenderThreadMs));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f,
        BudgetColor(Avg.StreamingMemoryMB, Budget.MaxStreamingMemoryMB),
        FString::Printf(TEXT("Streaming:   %.1f / %.1f MB (%d cars)"),
                         Avg.StreamingMemoryMB, Budget.MaxStreamingMemoryMB, Avg.LoadedCarCount));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f,
        BudgetColor(Avg.TexturePoolUtilization, Budget.MaxTexturePoolUtilization),
        FString::Printf(TEXT("Tex Pool:    %.1f%% / %.1f%%"),
                         Avg.TexturePoolUtilization * 100.0f, Budget.MaxTexturePoolUtilization * 100.0f));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f,
        BudgetColor(static_cast<float>(Avg.ActiveNPCCount), static_cast<float>(Budget.MaxActiveNPCs)),
        FString::Printf(TEXT("NPCs:        %d / %d"), Avg.ActiveNPCCount, Budget.MaxActiveNPCs));
    GEngine->AddOnScreenDebugMessage(100 + Line++, 0.0f, FColor::White,
        FString::Printf(TEXT("Memory:      %.0f MB phys"), Avg.UsedPhysicalMemoryMB));
}

void USEEPerformanceSubsystem::RegisterConsoleCommands()
{
    ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("TrainGame.Perf.ShowOverlay"),
        TEXT("Toggle performance debug overlay"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            bDebugOverlayEnabled = !bDebugOverlayEnabled;
            UE_LOG(LogTemp, Log, TEXT("Performance overlay: %s"), bDebugOverlayEnabled ? TEXT("ON") : TEXT("OFF"));
        })
    ));

    ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("TrainGame.Perf.Report"),
        TEXT("Log a full performance report"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            LogPerformanceReport();
        })
    ));

    ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("TrainGame.Perf.CheckBudgets"),
        TEXT("Check and log budget violations"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            TArray<FName> Violations = CheckBudgetViolations();
            if (Violations.Num() == 0)
            {
                UE_LOG(LogTemp, Log, TEXT("All performance budgets OK."));
            }
            else
            {
                for (const FName& V : Violations)
                {
                    UE_LOG(LogTemp, Warning, TEXT("VIOLATION: %s"), *V.ToString());
                }
            }
        })
    ));

    ConsoleCommands.Add(IConsoleManager::Get().RegisterConsoleCommand(
        TEXT("TrainGame.ShowCarStates"),
        TEXT("Show streaming state of all registered cars"),
        FConsoleCommandDelegate::CreateLambda([this]()
        {
            UWorld* World = GetWorld();
            if (!World) return;

            USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
            if (!StreamingSub) return;

            UE_LOG(LogTemp, Log, TEXT("=== CAR STREAMING STATES ==="));
            UE_LOG(LogTemp, Log, TEXT("Current Car: %d"), StreamingSub->GetCurrentCarIndex());
            UE_LOG(LogTemp, Log, TEXT("Loaded Cars: %d"), StreamingSub->GetLoadedCarCount());
            UE_LOG(LogTemp, Log, TEXT("Streaming Memory: %.1f MB"), StreamingSub->GetTotalLoadedMemoryMB());
            UE_LOG(LogTemp, Log, TEXT("Last Load Time: %.1f ms"), StreamingSub->GetLastCarLoadTimeMs());

            TArray<int32> LoadedCars = StreamingSub->GetLoadedCarIndices();
            for (int32 CarIndex : LoadedCars)
            {
                ECarStreamingState State = StreamingSub->GetCarState(CarIndex);
                const TCHAR* StateStr = TEXT("Unknown");
                switch (State)
                {
                case ECarStreamingState::Unloaded:   StateStr = TEXT("UNLOADED"); break;
                case ECarStreamingState::Preloading:  StateStr = TEXT("PRELOADING"); break;
                case ECarStreamingState::Loaded:      StateStr = TEXT("LOADED"); break;
                case ECarStreamingState::Active:      StateStr = TEXT("ACTIVE"); break;
                }
                UE_LOG(LogTemp, Log, TEXT("  Car %d: %s"), CarIndex, StateStr);
            }
            UE_LOG(LogTemp, Log, TEXT("============================"));
        })
    ));
}
