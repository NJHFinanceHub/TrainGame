#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEECarStreamingSubsystem.h"
#include "SEETextureStreamingManager.generated.h"

UENUM(BlueprintType)
enum class ETrainZone : uint8
{
    Tail    UMETA(DisplayName = "Tail Section"),
    Third   UMETA(DisplayName = "Third Class"),
    Second  UMETA(DisplayName = "Second Class"),
    First   UMETA(DisplayName = "First Class"),
    Engine  UMETA(DisplayName = "Engine Section"),
    Subtrain UMETA(DisplayName = "Subtrain"),
    Exterior UMETA(DisplayName = "Exterior")
};

USTRUCT(BlueprintType)
struct FZoneTextureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETrainZone Zone = ETrainZone::Tail;

    /** Per-zone texture pool budget in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TexturePoolBudgetMB = 128.0f;

    /** Mip bias for active car textures (0 = full res) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ActiveMipBias = 0;

    /** Mip bias for loaded (adjacent) car textures */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LoadedMipBias = 1;

    /** Mip bias for preloading car textures */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreloadMipBias = 2;

    /** Whether this zone uses unique material sets (vs shared kit materials) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasUniqueMaterials = false;
};

USTRUCT(BlueprintType)
struct FTextureStreamingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float TotalResidentMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float TotalStreamedMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PoolUtilization = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 PendingMipRequests = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 DroppedMipRequests = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTextureTransition, ETrainZone, OldZone, ETrainZone, NewZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTexturePoolExceeded, float, CurrentMB, float, BudgetMB);

UCLASS()
class SNOWPIERCEREE_API USEETextureStreamingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    /** Register which zone a car belongs to */
    UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
    void RegisterCarZone(int32 CarIndex, ETrainZone Zone);

    /** Set zone-specific texture configuration */
    UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
    void SetZoneConfig(ETrainZone Zone, const FZoneTextureConfig& Config);

    /** Get the current zone based on player's car */
    UFUNCTION(BlueprintPure, Category = "Texture Streaming")
    ETrainZone GetCurrentZone() const { return CurrentZone; }

    /** Get zone for a specific car */
    UFUNCTION(BlueprintPure, Category = "Texture Streaming")
    ETrainZone GetZoneForCar(int32 CarIndex) const;

    /** Get streaming statistics */
    UFUNCTION(BlueprintPure, Category = "Texture Streaming")
    FTextureStreamingStats GetStreamingStats() const { return CachedStats; }

    /** Get the global texture pool budget in MB */
    UFUNCTION(BlueprintPure, Category = "Texture Streaming")
    float GetGlobalPoolBudgetMB() const { return GlobalTexturePoolMB; }

    /** Force a texture pool defragmentation */
    UFUNCTION(BlueprintCallable, Category = "Texture Streaming")
    void ForcePoolDefrag();

    UPROPERTY(BlueprintAssignable, Category = "Texture Streaming|Events")
    FOnZoneTextureTransition OnZoneTextureTransition;

    UPROPERTY(BlueprintAssignable, Category = "Texture Streaming|Events")
    FOnTexturePoolExceeded OnTexturePoolExceeded;

    /** Global texture streaming pool budget (MB) */
    UPROPERTY(EditAnywhere, Category = "Texture Streaming")
    float GlobalTexturePoolMB = 512.0f;

private:
    void OnCarStreamingStateChanged(int32 CarIndex, ECarStreamingState NewState);
    void UpdateMipBiasForCar(int32 CarIndex, ECarStreamingState StreamingState);
    void HandleZoneTransition(ETrainZone NewZone);
    void UpdateStreamingStats();
    void ApplyTexturePoolSize();

    UPROPERTY()
    TMap<int32, ETrainZone> CarZoneMap;

    UPROPERTY()
    TMap<ETrainZone, FZoneTextureConfig> ZoneConfigs;

    ETrainZone CurrentZone = ETrainZone::Tail;
    FTextureStreamingStats CachedStats;

    float StatsUpdateTimer = 0.0f;
    static constexpr float StatsUpdateInterval = 0.5f;
};
