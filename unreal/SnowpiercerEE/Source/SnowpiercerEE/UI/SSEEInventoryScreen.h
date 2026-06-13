// SSEEInventoryScreen.h - Full-screen inventory display (USEEInventoryComponent-backed)
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowpiercerEE/SEEItemBase.h"
#include "TrainGame/Economy/ArmorComponent.h"

class USEEInventoryComponent;
class UInventoryComponent; // legacy SnowyEngine inventory - accepted for source compat, unused
class SVerticalBox;
class SScrollBox;
class SSEEMenuButton;
class SSEEArmorSlotWidget;

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
 * - Paper-doll armor panel (right column): HEAD/TORSO/SHIELD slot cells
 *   - Armor inventory rows are draggable; drop onto a slot cell to equip
 *   - E key or EQUIP button as keyboard/button fallback
 *   - Each slot cell has an UNEQUIP affordance returning the piece to inventory
 *   - Total DR% and cold resistance shown above the slots
 * - Keyboard navigation: Up/Down select, Enter use, Delete drop, E equip, Esc close
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

	/** Build the right-hand paper-doll armor panel. */
	TSharedRef<SWidget> MakeArmorPanel();

	void RebuildList();
	void MoveSelection(int32 Delta);
	void SelectEntry(int32 EntryIndex);
	void UseSelected();
	void DropSelected();

	/**
	 * Shared equip path used by both drag-drop and the keyboard/button fallback.
	 * Equips ItemID into Slot via ArmorComponent->EquipFromItem, removes one from
	 * inventory, and returns the evicted previous piece (if any) to inventory.
	 * Calls Refresh() on success.
	 */
	void EquipArmorItem(FName ItemID, EArmorSlot Slot);

	/**
	 * Unequips the piece in Slot and adds it back to inventory.
	 * Calls Refresh() on success.
	 */
	void UnequipArmorSlot(EArmorSlot Slot);

	/**
	 * Infer the best equip slot from an item's ID name.
	 * Helm/Mask/Hood → Head; Shield/Guard → Shield; everything else → Torso.
	 */
	static EArmorSlot InferSlotFromItemID(FName ItemID);

	/** Re-query the pawn's combat component (read-only) for the equipped weapon's ItemID. */
	void RefreshEquippedItemID();

	bool HasSelection() const { return Entries.IsValidIndex(SelectedIndex); }
	const FSEEItemData* GetData(FName ItemID) const;
	const FSEEItemData* GetSelectedData() const;

	/** Returns the pawn's UArmorComponent, or nullptr. */
	UArmorComponent* GetArmorComponent() const;

	FText GetCategoryText(ESEEItemCategory Category) const;
	FText GetRarityText(ESEEItemRarity Rarity) const;
	FLinearColor GetRarityColor(ESEEItemRarity Rarity) const;

	/** Single-letter badge (W/A/C/M/Q/J) and its accent color per category. */
	FText GetCategoryBadgeLetter(ESEEItemCategory Category) const;
	FLinearColor GetCategoryBadgeColor(ESEEItemCategory Category) const;

	TWeakObjectPtr<USEEInventoryComponent> InventoryComp;
	FSimpleDelegate OnRequestClose;

	TSharedPtr<SVerticalBox> ListBox;
	TSharedPtr<SScrollBox>   ListScrollBox;
	TSharedPtr<SSEEMenuButton> UseButton;
	TSharedPtr<SSEEMenuButton> DropButton;
	TSharedPtr<SSEEMenuButton> EquipButton;

	/** The three slot cell widgets so we can issue targeted refreshes. */
	TSharedPtr<SSEEArmorSlotWidget> HeadSlotWidget;
	TSharedPtr<SSEEArmorSlotWidget> TorsoSlotWidget;
	TSharedPtr<SSEEArmorSlotWidget> ShieldSlotWidget;

	TArray<FEntry>              Entries;
	TArray<TSharedPtr<SWidget>> RowWidgets;

	int32  SelectedIndex    = INDEX_NONE;
	int32  OccupiedSlotCount = 0;
	uint8  ActiveCategory   = 255; // 255 = "All" filter

	/** ItemID of the pawn's currently equipped weapon (NAME_None when unarmed). */
	FName EquippedItemID;
};
