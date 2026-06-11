// SSEEInventoryScreen.h - Full-screen inventory display (USEEInventoryComponent-backed)
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowpiercerEE/SEEItemBase.h"

class USEEInventoryComponent;
class UInventoryComponent; // legacy SnowyEngine inventory - accepted for source compat, unused
class SVerticalBox;
class SScrollBox;

/**
 * SSEEInventoryScreen
 *
 * Full-screen inventory panel backed by the pawn's USEEInventoryComponent:
 * - Item list (name, stack count, weight) with category filter tabs
 * - Detail panel from FSEEItemData (description, rarity, weight, value, restores)
 * - Use (consumables - applies HealthRestore via USEEInventoryComponent::UseItem)
 * - Drop (USEEInventoryComponent::DropItem; quest items refuse)
 * - Carry-weight capacity bar
 * - Keyboard navigation: Up/Down select, Enter use, Delete drop, Esc close
 *
 * The widget pulls a snapshot of the slots in Refresh(); the UI subsystem calls
 * Refresh() on open and from the component's OnInventoryChanged delegate.
 */
class SSEEInventoryScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEInventoryScreen)
		: _InventoryComponent(nullptr)
		, _SEEInventoryComponent(nullptr)
	{}
		/** Legacy SnowyEngine inventory argument (kept for older call sites; ignored). */
		SLATE_ARGUMENT(UInventoryComponent*, InventoryComponent)
		/** The pawn's inventory component - the data source for this screen. */
		SLATE_ARGUMENT(USEEInventoryComponent*, SEEInventoryComponent)
		/** Invoked when the user asks to close the screen (Esc / Close button). */
		SLATE_EVENT(FSimpleDelegate, OnRequestClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Re-pull the slot snapshot from the component and rebuild the item list. */
	void Refresh();

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** One visible (non-empty, filter-matching) inventory slot. */
	struct FEntry
	{
		int32 SlotIndex = INDEX_NONE;
		FName ItemID;
		int32 Quantity = 0;
	};

	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeCategoryTabs();
	TSharedRef<SWidget> MakeTab(const FText& Label, uint8 CategoryValue);
	TSharedRef<SWidget> MakeDetailPanel();
	TSharedRef<SWidget> MakeWeightBar();
	TSharedRef<SWidget> MakeActionButton(const FText& Label, bool bDanger);

	void RebuildList();
	void MoveSelection(int32 Delta);
	void SelectEntry(int32 EntryIndex);
	void UseSelected();
	void DropSelected();

	bool HasSelection() const { return Entries.IsValidIndex(SelectedIndex); }
	const FSEEItemData* GetData(FName ItemID) const;
	const FSEEItemData* GetSelectedData() const;

	FText GetCategoryText(ESEEItemCategory Category) const;
	FText GetRarityText(ESEEItemRarity Rarity) const;
	FLinearColor GetRarityColor(ESEEItemRarity Rarity) const;

	TWeakObjectPtr<USEEInventoryComponent> InventoryComp;
	FSimpleDelegate OnRequestClose;

	TSharedPtr<SVerticalBox> ListBox;
	TSharedPtr<SScrollBox> ListScrollBox;
	TArray<FEntry> Entries;
	TArray<TSharedPtr<SWidget>> RowWidgets;

	int32 SelectedIndex = INDEX_NONE;
	int32 OccupiedSlotCount = 0;
	uint8 ActiveCategory = 255; // 255 = "All" filter
};
