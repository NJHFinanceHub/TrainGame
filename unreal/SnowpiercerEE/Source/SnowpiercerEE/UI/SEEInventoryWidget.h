// SEEInventoryWidget.h - Inventory screen: grid + weight, sorting, item details, comparison
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEItemBase.h"
#include "SEEInventoryWidget.generated.h"

class USEEInventoryComponent;
class UTextBlock;
class UProgressBar;
class UUniformGridPanel;
class UImage;

UENUM(BlueprintType)
enum class ESEEInventorySortMode : uint8
{
	Default		UMETA(DisplayName = "Default"),
	ByName		UMETA(DisplayName = "By Name"),
	ByCategory	UMETA(DisplayName = "By Category"),
	ByRarity	UMETA(DisplayName = "By Rarity"),
	ByWeight	UMETA(DisplayName = "By Weight"),
	ByValue		UMETA(DisplayName = "By Value")
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void InitInventory(USEEInventoryComponent* InInventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshInventory();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetSortMode(ESEEInventorySortMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseSelectedItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void DropSelectedItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AssignToQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetSelectedSlot() const { return SelectedSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool GetSelectedItemData(FSEEItemData& OutData) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FLinearColor GetRarityColor(ESEEItemRarity Rarity) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FText GetCategoryName(ESEEItemCategory Category) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeightText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> WeightBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemDescText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemStatsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SortModeText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnInventoryRefreshed(const TArray<FSEEInventorySlot>& Slots);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnSlotSelected(int32 SlotIndex, const FSEEItemData& ItemData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnSlotEmpty(int32 SlotIndex);

private:
	void UpdateWeightDisplay();
	void UpdateSelectedItemDetails();
	void SortSlots(TArray<FSEEInventorySlot>& Slots) const;

	TWeakObjectPtr<USEEInventoryComponent> InventoryComp;
	int32 SelectedSlotIndex = -1;
	ESEEInventorySortMode CurrentSortMode = ESEEInventorySortMode::Default;
};
