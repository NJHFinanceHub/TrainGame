#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEECarStreamingSubsystem.h"
#include "SEECarLODSubsystem.generated.h"

UENUM(BlueprintType)
enum class ECarLODLevel : uint8
{
    LOD0_Full     UMETA(DisplayName = "LOD0 - Full Detail (Active Car)"),
    LOD1_Reduced  UMETA(DisplayName = "LOD1 - Reduced (Adjacent, through doors)"),
    LOD2_Shell    UMETA(DisplayName = "LOD2 - Shell (2-5 cars away)"),
    LOD3_Impostor UMETA(DisplayName = "LOD3 - Billboard Impostor (5+ cars)")
};

USTRUCT(BlueprintType)
struct FCarLODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LOD1_MaxCarDistance = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LOD2_MaxCarDistance = 5;

    // Beyond LOD2_MaxCarDistance uses LOD3 (impostor)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxVisibleExteriorCars = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD1_TriangleBudget = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD2_TriangleBudget = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LOD3_TriangleBudget = 100.0f;
};

USTRUCT(BlueprintType)
struct FCarLODEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 CarIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly)
    ECarLODLevel CurrentLOD = ECarLODLevel::LOD3_Impostor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UStaticMesh> ExteriorShellMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ImpostorTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ExteriorTint = FLinearColor::White;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCarLODChanged, int32, CarIndex, ECarLODLevel, NewLOD);

UCLASS()
class SNOWPIERCEREE_API USEECarLODSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void RegisterCarLOD(int32 CarIndex, TSoftObjectPtr<UStaticMesh> ExteriorShell,
                        TSoftObjectPtr<UTexture2D> Impostor, FLinearColor Tint = FLinearColor::White);

    UFUNCTION(BlueprintPure, Category = "LOD")
    ECarLODLevel GetCarLODLevel(int32 CarIndex) const;

    UFUNCTION(BlueprintPure, Category = "LOD")
    ECarLODLevel CalculateLODForDistance(int32 CarDistance) const;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODConfig(const FCarLODConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "LOD")
    const FCarLODConfig& GetLODConfig() const { return Config; }

    UPROPERTY(BlueprintAssignable, Category = "LOD|Events")
    FOnCarLODChanged OnCarLODChanged;

private:
    void RefreshAllLODs();

    UPROPERTY()
    TMap<int32, FCarLODEntry> CarLODEntries;

    UPROPERTY()
    FCarLODConfig Config;

    int32 CachedCurrentCarIndex = INDEX_NONE;
    float LODUpdateTimer = 0.0f;
    static constexpr float LODUpdateInterval = 0.25f; // Update LODs 4 times per second
};
