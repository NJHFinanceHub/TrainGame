// InventoryComponent.h - Inventory management with weight limits and stacking
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryTypes.h"
#include "InventoryComponent.generated.h"

/**
 * UInventoryComponent
 *
 * Grid/list-based inventory with weight limits. Manages item instances,
 * stacking, durability tracking, and provides queries for the crafting system.
 *
 * Items are defined in data tables (FItemDefinition). Runtime instances (FInventoryItem)
 * track per-stack state like durability and count.
 *
 * Scrap and Influence are tracked as special resource counters (not inventory items)
 * for performance and UX simplicity.
 */
UCLASS(ClassGroup=(Inventory), meta=(BlueprintSpawnableComponent))
class SNOWYENGINE_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// --- Item Operations ---

	/** Try to add an item. Returns the number of items actually added (may be less if weight-capped). */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 AddItem(FName ItemID, int32 Count = 1, float Durability = -1.0f);

	/** Remove items by ID. Returns number actually removed. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 RemoveItem(FName ItemID, int32 Count = 1);

	/** Remove a specific item instance by GUID. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItemByInstance(const FGuid& InstanceID);

	/** Check if inventory contains at least Count of the given item. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Count = 1) const;

	/** Get total count of an item across all stacks. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	/** Get all items in inventory. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventoryItem>& GetAllItems() const { return Items; }

	/** Find a specific item instance. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool FindItem(FName ItemID, FInventoryItem& OutItem) const;

	// --- Weight ---

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const { return MaxCarryWeight; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetWeightPercent() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsOverweight() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetMaxCarryWeight(float NewMax) { MaxCarryWeight = NewMax; }

	// --- Durability ---

	/** Reduce durability of a specific item instance. Returns true if item broke. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool DegradeDurability(const FGuid& InstanceID, float Amount);

	/** Repair a specific item instance. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RepairItem(const FGuid& InstanceID, float Amount);

	// --- Special Resources (not inventory items) ---

	UFUNCTION(BlueprintPure, Category = "Inventory|Resources")
	int32 GetScrap() const { return Scrap; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Resources")
	void ModifyScrap(int32 Delta);

	UFUNCTION(BlueprintPure, Category = "Inventory|Resources")
	float GetInfluence() const { return Influence; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Resources")
	void ModifyInfluence(float Delta);

	// --- Data Table ---

	/** Look up an item definition from the data table. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetItemDefinition(FName ItemID, FItemDefinition& OutDef) const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

protected:
	virtual void BeginPlay() override;

	// The data table containing all item definitions
	UPROPERTY(EditAnywhere, Category = "Inventory|Config")
	UDataTable* ItemDataTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Inventory|Config")
	float MaxCarryWeight = 50.0f;

	// Current inventory contents
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Runtime")
	TArray<FInventoryItem> Items;

	// Special resource counters
	UPROPERTY(VisibleAnywhere, Category = "Inventory|Runtime")
	int32 Scrap = 0;

	UPROPERTY(VisibleAnywhere, Category = "Inventory|Runtime")
	float Influence = 0.0f;

private:
	float GetItemWeight(FName ItemID) const;
	int32 GetItemMaxStack(FName ItemID) const;
	FInventoryItem* FindItemInstance(const FGuid& InstanceID);
	FInventoryItem* FindStackableSlot(FName ItemID);
};
