#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEECarStreamingSubsystem.generated.h"

UENUM(BlueprintType)
enum class ECarStreamingState : uint8
{
    Unloaded,
    Preloading,
    Loaded,
    Active
};

USTRUCT(BlueprintType)
struct FCarStreamingInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 CarIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    FName LevelName;

    UPROPERTY(BlueprintReadOnly)
    ECarStreamingState State = ECarStreamingState::Unloaded;

    UPROPERTY(BlueprintReadOnly)
    float EstimatedMemoryMB = 60.0f;

    UPROPERTY(BlueprintReadOnly)
    double LoadStartTime = 0.0;

    UPROPERTY(BlueprintReadOnly)
    double LoadCompleteTime = 0.0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCarStreamingStateChanged, int32, CarIndex, ECarStreamingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCarLoadComplete, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreamingBudgetExceeded, float, CurrentMB, float, BudgetMB);

UCLASS()
class SNOWPIERCEREE_API USEECarStreamingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void RegisterCarLevel(int32 CarIndex, FName LevelName, float EstimatedMemoryMB = 60.0f);

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void EnterCar(int32 CarIndex);

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void SetDoorLoadMaskActive(bool bActive);

    UFUNCTION(BlueprintPure, Category="Streaming")
    int32 GetCurrentCarIndex() const { return CurrentCarIndex; }

    UFUNCTION(BlueprintPure, Category="Streaming")
    ECarStreamingState GetCarState(int32 CarIndex) const;

    UFUNCTION(BlueprintPure, Category="Streaming")
    float GetTotalLoadedMemoryMB() const;

    UFUNCTION(BlueprintPure, Category="Streaming")
    float GetLastCarLoadTimeMs() const { return LastCarLoadTimeMs; }

    UFUNCTION(BlueprintPure, Category="Streaming")
    int32 GetLoadedCarCount() const { return LoadedCars.Num(); }

    UFUNCTION(BlueprintPure, Category="Streaming")
    bool IsCarLoaded(int32 CarIndex) const { return LoadedCars.Contains(CarIndex); }

    UFUNCTION(BlueprintPure, Category="Streaming")
    TArray<int32> GetLoadedCarIndices() const { return LoadedCars.Array(); }

    // Events
    UPROPERTY(BlueprintAssignable, Category="Streaming|Events")
    FOnCarStreamingStateChanged OnCarStreamingStateChanged;

    UPROPERTY(BlueprintAssignable, Category="Streaming|Events")
    FOnCarLoadComplete OnCarLoadComplete;

    UPROPERTY(BlueprintAssignable, Category="Streaming|Events")
    FOnStreamingBudgetExceeded OnStreamingBudgetExceeded;

    // Memory budget (MB) for all loaded cars combined
    UPROPERTY(EditAnywhere, Category="Streaming|Budget")
    float MemoryBudgetMB = 240.0f;

private:
    void RefreshStreamingSet();
    void StreamLevel(FName LevelName, bool bShouldLoad);
    void BeginPreload(int32 CarIndex);
    void SetCarState(int32 CarIndex, ECarStreamingState NewState);

    UPROPERTY()
    TMap<int32, FCarStreamingInfo> CarInfoByIndex;

    UPROPERTY()
    TSet<int32> LoadedCars;

    UPROPERTY()
    TSet<int32> PreloadingCars;

    UPROPERTY()
    int32 CurrentCarIndex = INDEX_NONE;

    UPROPERTY()
    int32 PreviousCarIndex = INDEX_NONE;

    // 3-car window: active + 2 adjacent
    UPROPERTY()
    int32 MaxLoadedCars = 3;

    UPROPERTY()
    bool bDoorLoadMaskActive = false;

    UPROPERTY()
    bool bRefreshPending = false;

    float LastCarLoadTimeMs = 0.0f;
};
