#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEInventoryWidget.generated.h"

UENUM(BlueprintType)
enum class ESEEEquipmentSlot : uint8
{
	Weapon		UMETA(DisplayName="Weapon"),
	Armor		UMETA(DisplayName="Armor"),
	Headgear	UMETA(DisplayName="Headgear"),
	Accessory	UMETA(DisplayName="Accessory")
};

USTRUCT(BlueprintType)
struct FSEEInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	int32 Quantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
	FText Description;

	bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSEEInventorySlotClicked, int32, SlotIndex, const FSEEInventorySlot&, Slot);

UCLASS(Abstract)
class SNOWPIERCEREE_API USEEInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetGridSize(int32 Columns, int32 Rows);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetSlot(int32 Index, const FSEEInventorySlot& Slot);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ClearSlot(int32 Index);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void SetEquipmentSlot(ESEEEquipmentSlot Slot, const FSEEInventorySlot& Item);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ClearEquipmentSlot(ESEEEquipmentSlot Slot);

	UFUNCTION(BlueprintPure, Category="Inventory")
	const FSEEInventorySlot& GetSlot(int32 Index) const;

	UFUNCTION(BlueprintPure, Category="Inventory")
	int32 GetTotalSlots() const { return InventorySlots.Num(); }

	UPROPERTY(BlueprintAssignable, Category="Inventory")
	FOnSEEInventorySlotClicked OnSlotClicked;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TArray<FSEEInventorySlot> InventorySlots;

	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	TMap<ESEEEquipmentSlot, FSEEInventorySlot> EquipmentSlots;

	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	int32 GridColumns = 6;

	UPROPERTY(BlueprintReadOnly, Category="Inventory")
	int32 GridRows = 4;

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnGridChanged();

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnSlotUpdated(int32 Index);

	UFUNCTION(BlueprintImplementableEvent, Category="Inventory")
	void OnEquipmentSlotUpdated(ESEEEquipmentSlot Slot);
};
