// SSEEInventoryScreen.h - Full-screen inventory display
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowyEngine/Inventory/InventoryTypes.h"

class UInventoryComponent;

/**
 * SSEEInventoryScreen
 *
 * Full-screen inventory panel showing:
 * - Item list with icons, names, quantities, and durability
 * - Category filter tabs (All, Weapons, Armor, Consumables, Materials, Quest, Misc)
 * - Weight bar (current / max carry weight)
 * - Resource counters (Scrap, Influence)
 * - Item detail panel (shows description, stats when an item is selected)
 *
 * Toggled via ASEEGameHUD::ToggleInventory(). Only one full-screen panel
 * can be open at a time.
 */
class SSEEInventoryScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEInventoryScreen)
		: _InventoryComponent(nullptr)
	{}
		SLATE_ARGUMENT(UInventoryComponent*, InventoryComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeCategoryTabs();
	TSharedRef<SWidget> MakeItemList();
	TSharedRef<SWidget> MakeItemRow(int32 ItemIndex);
	TSharedRef<SWidget> MakeDetailPanel();
	TSharedRef<SWidget> MakeWeightBar();
	TSharedRef<SWidget> MakeResourceCounters();

	// Get filtered items based on active category
	TArray<FInventoryItem> GetFilteredItems() const;

	// Item selection
	void SelectItem(int32 ItemIndex);

	FText GetItemDisplayName(FName ItemID) const;
	FText GetItemDescription(FName ItemID) const;
	FLinearColor GetRarityColor(FName ItemID) const;

	TWeakObjectPtr<UInventoryComponent> InventoryComp;
	EItemCategory ActiveCategory = static_cast<EItemCategory>(255); // 255 = "All" filter
	int32 SelectedItemIndex = INDEX_NONE;
};
