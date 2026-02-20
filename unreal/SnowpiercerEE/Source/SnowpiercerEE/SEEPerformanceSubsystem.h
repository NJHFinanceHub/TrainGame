#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEEPerformanceSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FPerformanceSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GPUTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float UsedPhysicalMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float UsedVirtualMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 TrianglesRendered = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveNPCCount = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 LoadedCarCount = 0;

    UPROPERTY(BlueprintReadOnly)
    float StreamingMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TexturePoolUtilization = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActivePhysicsBodies = 0;
};

USTRUCT(BlueprintType)
struct FPerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetFrameTimeMs = 16.6f;  // 60 fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxGameThreadMs = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxRenderThreadMs = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxStreamingMemoryMB = 240.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxTexturePoolUtilization = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxDrawCalls = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxActiveNPCs = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPhysicsBodies = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxCarLoadTimeMs = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxPerCarMemoryMB = 60.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBudgetViolation, FName, BudgetName, float, CurrentValue);

UCLASS()
class SNOWPIERCEREE_API USEEPerformanceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    /** Get the latest performance snapshot */
    UFUNCTION(BlueprintPure, Category = "Performance")
    const FPerformanceSnapshot& GetCurrentSnapshot() const { return CurrentSnapshot; }

    /** Get rolling average over the sample window */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceSnapshot GetAverageSnapshot() const;

    /** Get the worst-case snapshot in the sample window */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerformanceSnapshot GetWorstSnapshot() const;

    /** Get the current budget configuration */
    UFUNCTION(BlueprintPure, Category = "Performance")
    const FPerformanceBudget& GetBudget() const { return Budget; }

    /** Set performance budget thresholds */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBudget(const FPerformanceBudget& NewBudget);

    /** Check all budgets and return list of violations */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<FName> CheckBudgetViolations() const;

    /** Enable/disable on-screen debug overlay */
    UFUNCTION(BlueprintCallable, Category = "Performance|Debug")
    void SetDebugOverlayEnabled(bool bEnabled);

    /** Log a performance report to the output log */
    UFUNCTION(BlueprintCallable, Category = "Performance|Debug")
    void LogPerformanceReport() const;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnBudgetViolation OnBudgetViolation;

private:
    void SamplePerformance();
    void CheckBudgets();
    void DrawDebugOverlay() const;
    void RegisterConsoleCommands();

    FPerformanceSnapshot CurrentSnapshot;
    FPerformanceBudget Budget;

    TArray<FPerformanceSnapshot> SnapshotHistory;
    int32 MaxHistorySize = 120; // 2 seconds at 60fps

    bool bDebugOverlayEnabled = false;
    float SampleTimer = 0.0f;
    float BudgetCheckTimer = 0.0f;

    static constexpr float SampleInterval = 0.0f;     // Every frame
    static constexpr float BudgetCheckInterval = 1.0f; // Every second

    // Console command handles
    TArray<IConsoleObject*> ConsoleCommands;
};
