// CollectibleComponent.h
// Snowpiercer: Eternal Engine - World-placed collectible actor component

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CollectibleTypes.h"
#include "CollectibleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollectiblePickedUp, FName, CollectibleID, AActor*, Collector);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollectibleProximity, FName, CollectibleID, float, Distance);

/**
 * Component attached to world actors that represent collectible items.
 * Handles proximity detection, stat-gated access, and pickup logic.
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UCollectibleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCollectibleComponent();

	/** Collectible data row name in the CollectibleDataTable */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collectible")
	FName CollectibleRowName;

	/** Whether this collectible requires an environmental interaction before becoming available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectible")
	bool bRequiresActivation = false;

	/** Whether the collectible has been activated (for gated collectibles) */
	UPROPERTY(BlueprintReadWrite, Category = "Collectible")
	bool bIsActivated = true;

	/** Detection radius override (0 = use PER-based defaults from design) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collectible|Detection", meta = (ClampMin = "0.0"))
	float DetectionRadiusOverride = 0.0f;

	/** Whether to show glow effect regardless of PER (used for Blueprints) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collectible|Detection")
	bool bAlwaysShowGlow = false;

	// Events

	UPROPERTY(BlueprintAssignable, Category = "Collectible|Events")
	FOnCollectiblePickedUp OnPickedUp;

	UPROPERTY(BlueprintAssignable, Category = "Collectible|Events")
	FOnCollectibleProximity OnPlayerProximity;

	// Interface

	/** Attempt to pick up this collectible. Returns false if stat requirements not met. */
	UFUNCTION(BlueprintCallable, Category = "Collectible")
	bool TryPickup(AActor* Collector);

	/** Activate a gated collectible (e.g., after solving puzzle, breaking wall) */
	UFUNCTION(BlueprintCallable, Category = "Collectible")
	void Activate();

	/** Get detection radius for a given PER score */
	UFUNCTION(BlueprintPure, Category = "Collectible")
	float GetDetectionRadius(int32 PerceptionScore) const;

	/** Check if collector meets stat requirements */
	UFUNCTION(BlueprintPure, Category = "Collectible")
	bool MeetsStatRequirements(AActor* Collector) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Cached reference to the collectible data */
	const FCollectibleData* CachedData = nullptr;

	void UpdateProximityDetection(float DeltaTime);
	void ApplyPickupEffects(AActor* Collector);
	void TriggerCompanionResonance(AActor* Collector);
};
