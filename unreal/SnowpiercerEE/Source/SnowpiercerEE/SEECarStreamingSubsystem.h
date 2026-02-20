#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEECarStreamingSubsystem.generated.h"

UCLASS()
class SNOWPIERCEREE_API USEECarStreamingSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void RegisterCarLevel(int32 CarIndex, FName LevelName);

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void EnterCar(int32 CarIndex);

    UFUNCTION(BlueprintCallable, Category="Streaming")
    void SetDoorLoadMaskActive(bool bActive);

    UFUNCTION(BlueprintPure, Category="Streaming")
    int32 GetCurrentCarIndex() const { return CurrentCarIndex; }

private:
    void RefreshStreamingSet();
    void StreamLevel(FName LevelName, bool bShouldLoad);

    UPROPERTY()
    TMap<int32, FName> CarLevelByIndex;

    UPROPERTY()
    TSet<int32> LoadedCars;

    UPROPERTY()
    int32 CurrentCarIndex = INDEX_NONE;

    // 3-car window is sufficient even at 10x scale — larger cars mean
    // longer traversal time, giving more streaming headroom.
    UPROPERTY()
    int32 MaxLoadedCars = 3;

    UPROPERTY()
    bool bDoorLoadMaskActive = false;

    UPROPERTY()
    bool bRefreshPending = false;
};
