#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEItemBase.h"
#include "SEEInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemAdded, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemRemoved, FName, ItemID, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEInventoryComponent();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetQuickSlot(int32 QuickSlotIndex, int32 InventorySlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemID, int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetItemCount(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetCurrentWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	float GetMaxWeight() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsOverweight() const { return GetCurrentWeight() > GetMaxWeight(); }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FSEEInventorySlot> GetAllSlots() const { return Slots; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSlotCount() const { return Slots.Num(); }

	const FSEEItemData* GetItemDataPtr(FName ItemID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetItemData(FName ItemID, FSEEItemData& OutData) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 MaxSlots = 30;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float BaseCarryCapacity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float StrengthWeightBonus = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UDataTable> ItemDataTable;

private:
	TArray<FSEEInventorySlot> Slots;
	TArray<int32> QuickSlots; // Maps quick slot 0-3 -> inventory slot index

	int32 FindSlotWithItem(FName ItemID) const;
	int32 FindEmptySlot() const;

	virtual void BeginPlay() override;
};
