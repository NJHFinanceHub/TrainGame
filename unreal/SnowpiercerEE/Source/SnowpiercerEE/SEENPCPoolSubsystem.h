#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEENPCCharacter.h"
#include "SEECarStreamingSubsystem.h"
#include "SEENPCPoolSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FNPCPoolConfig
{
    GENERATED_BODY()

    /** Maximum NPCs allowed per car */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxNPCsPerCar = 20;

    /** Maximum NPCs with full AI ticking per frame (round-robin) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxFullTickPerFrame = 4;

    /** AI tick rate for non-full-tick NPCs (Hz) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReducedTickRateHz = 10.0f;

    /** Maximum total pooled NPC actors (reuse across cars) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPoolSize = 40;

    /** Distance threshold to force-cull NPC rendering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CullDistanceCm = 5000.0f;
};

USTRUCT()
struct FPooledNPCInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<ASEENPCCharacter> NPC;

    UPROPERTY()
    int32 AssignedCarIndex = INDEX_NONE;

    UPROPERTY()
    bool bActive = false;

    UPROPERTY()
    bool bFullTickThisFrame = false;

    /** Saved state for freeze/thaw */
    UPROPERTY()
    ESEENPCState SavedState = ESEENPCState::Idle;

    UPROPERTY()
    FVector SavedLocation = FVector::ZeroVector;

    UPROPERTY()
    FRotator SavedRotation = FRotator::ZeroRotator;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCPoolChanged, int32, CarIndex, int32, ActiveCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCBudgetExceeded, int32, CarIndex, int32, NPCCount);

UCLASS()
class SNOWPIERCEREE_API USEENPCPoolSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override;

    /** Register an NPC to be managed by the pool */
    UFUNCTION(BlueprintCallable, Category = "NPC Pool")
    void RegisterNPC(ASEENPCCharacter* NPC, int32 CarIndex);

    /** Remove an NPC from pool management */
    UFUNCTION(BlueprintCallable, Category = "NPC Pool")
    void UnregisterNPC(ASEENPCCharacter* NPC);

    /** Get count of active NPCs in a car */
    UFUNCTION(BlueprintPure, Category = "NPC Pool")
    int32 GetActiveNPCCount(int32 CarIndex) const;

    /** Get total pool utilization */
    UFUNCTION(BlueprintPure, Category = "NPC Pool")
    int32 GetTotalPooledNPCs() const { return PooledNPCs.Num(); }

    /** Get all active NPCs in a specific car */
    UFUNCTION(BlueprintCallable, Category = "NPC Pool")
    TArray<ASEENPCCharacter*> GetActiveNPCsInCar(int32 CarIndex) const;

    /** Set pool configuration */
    UFUNCTION(BlueprintCallable, Category = "NPC Pool")
    void SetPoolConfig(const FNPCPoolConfig& NewConfig);

    UPROPERTY(BlueprintAssignable, Category = "NPC Pool|Events")
    FOnNPCPoolChanged OnNPCPoolChanged;

    UPROPERTY(BlueprintAssignable, Category = "NPC Pool|Events")
    FOnNPCBudgetExceeded OnNPCBudgetExceeded;

private:
    void OnCarStreamingStateChanged(int32 CarIndex, ECarStreamingState NewState);
    void ActivateNPCsForCar(int32 CarIndex);
    void FreezeNPCsForCar(int32 CarIndex);
    void DeactivateNPCsForCar(int32 CarIndex);
    void UpdateStaggeredTicking();
    void CleanupStaleEntries();

    UPROPERTY()
    TArray<FPooledNPCInfo> PooledNPCs;

    UPROPERTY()
    FNPCPoolConfig Config;

    /** Round-robin index for full AI tick distribution */
    int32 FullTickRoundRobinIndex = 0;

    /** Accumulator for reduced tick rate */
    float ReducedTickAccumulator = 0.0f;

    float CleanupTimer = 0.0f;
    static constexpr float CleanupInterval = 5.0f;

    FDelegateHandle StreamingStateChangedHandle;
};
