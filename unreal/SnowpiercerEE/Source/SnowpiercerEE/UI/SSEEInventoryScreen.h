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
class SSEEMenuButton;

/**
 * SSEEInventoryScreen
 *
 * "POSSESSIONS" - the Eternal Engine inventory, in SSEEPanelFrame chrome:
 * - Item list (category letter badge, name, equip marker, stack count, weight)
 *   with category filter tabs
 * - Detail panel from FSEEItemData (description, rarity, weight, value, restores)
 * - USE (consumables - applies HealthRestore via USEEInventoryComponent::UseItem)
 *   and DROP (USEEInventoryComponent::DropItem; quest items refuse) as
 *   SSEEMenuButton chrome
 * - Carry-weight capacity bar that shades engine-amber to blood-red as it fills
 * - Equip indicator: weapons matching the pawn's equipped weapon (queried
 *   read-only from USEECombatComponent) show an amber marker, others a dash
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
	TSharedRef<SWidget> MakeActionButtons();

	void RebuildList();
	void MoveSelection(int32 Delta);
	void SelectEntry(int32 EntryIndex);
	void UseSelected();
	void DropSelected();

	/** Re-query the pawn's combat component (read-only) for the equipped weapon's ItemID. */
	void RefreshEquippedItemID();

	bool HasSelection() const { return Entries.IsValidIndex(SelectedIndex); }
	const FSEEItemData* GetData(FName ItemID) const;
	const FSEEItemData* GetSelectedData() const;

	FText GetCategoryText(ESEEItemCategory Category) const;
	FText GetRarityText(ESEEItemRarity Rarity) const;
	FLinearColor GetRarityColor(ESEEItemRarity Rarity) const;

	/** Single-letter badge (W/A/C/M/Q/J) and its accent color per category. */
	FText GetCategoryBadgeLetter(ESEEItemCategory Category) const;
	FLinearColor GetCategoryBadgeColor(ESEEItemCategory Category) const;

	TWeakObjectPtr<USEEInventoryComponent> InventoryComp;
	FSimpleDelegate OnRequestClose;

	TSharedPtr<SVerticalBox> ListBox;
	TSharedPtr<SScrollBox> ListScrollBox;
	TSharedPtr<SSEEMenuButton> UseButton;
	TSharedPtr<SSEEMenuButton> DropButton;
	TArray<FEntry> Entries;
	TArray<TSharedPtr<SWidget>> RowWidgets;

	int32 SelectedIndex = INDEX_NONE;
	int32 OccupiedSlotCount = 0;
	uint8 ActiveCategory = 255; // 255 = "All" filter

	/** ItemID of the pawn's currently equipped weapon (NAME_None when unarmed). */
	FName EquippedItemID;
};
