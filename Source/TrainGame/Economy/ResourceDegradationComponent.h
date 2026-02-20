// ResourceDegradationComponent.h - Tracks and applies degradation to perishable inventory items
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyTypes.h"
#include "ResourceDegradationComponent.generated.h"

class UInventoryComponent;

/**
 * UResourceDegradationComponent
 *
 * Ticks on the player character alongside UInventoryComponent. Tracks condition
 * values for perishable items and degrades them over time. Items at 0% condition
 * are auto-removed from inventory.
 *
 * Degradation can be halted by storing items in appropriate storage actors
 * (refrigeration, sealed containers, etc.) — those actors call PauseItemDegradation().
 */
UCLASS(ClassGroup=(Economy), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UResourceDegradationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UResourceDegradationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Condition Queries ---

	/** Get the current condition (0.0-1.0) of an item instance. Returns 1.0 for durable items. */
	UFUNCTION(BlueprintPure, Category = "Economy|Degradation")
	float GetItemCondition(const FGuid& InstanceID) const;

	/** Get the condition label (Fresh/Worn/Degraded/Ruined). */
	UFUNCTION(BlueprintPure, Category = "Economy|Degradation")
	FText GetConditionLabel(const FGuid& InstanceID) const;

	/** Get the effectiveness multiplier for a degraded item. */
	UFUNCTION(BlueprintPure, Category = "Economy|Degradation")
	float GetEffectivenessMultiplier(const FGuid& InstanceID) const;

	// --- Degradation Control ---

	/** Pause degradation for an item (stored in preservation container). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Degradation")
	void PauseItemDegradation(const FGuid& InstanceID);

	/** Resume degradation for an item (removed from preservation container). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Degradation")
	void ResumeItemDegradation(const FGuid& InstanceID);

	/** Force set condition for an item (debug/quest use). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Degradation")
	void SetItemCondition(const FGuid& InstanceID, float Condition);

	// --- Registration ---

	/** Register a new item for degradation tracking. Called when items are added to inventory. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Degradation")
	void RegisterItem(const FGuid& InstanceID, FName ItemID);

	/** Unregister an item (removed from inventory). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Degradation")
	void UnregisterItem(const FGuid& InstanceID);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Economy|Degradation")
	FOnItemDegraded OnItemDegraded;

	UPROPERTY(BlueprintAssignable, Category = "Economy|Degradation")
	FOnItemDestroyed OnItemDestroyed;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Economy|Config")
	UDataTable* ResourceEconomyDataTable = nullptr;

	// In-game seconds per "game day" for degradation rate conversion
	UPROPERTY(EditAnywhere, Category = "Economy|Config")
	float SecondsPerGameDay = 1440.0f;

private:
	// Per-item degradation state
	struct FDegradationEntry
	{
		FName ItemID;
		float Condition = 1.0f;
		float DegradationRatePerDay = 0.0f;
		bool bPaused = false;
	};

	TMap<FGuid, FDegradationEntry> TrackedItems;

	UPROPERTY()
	UInventoryComponent* LinkedInventory = nullptr;

	float GetDegradationRate(FName ItemID) const;
};
