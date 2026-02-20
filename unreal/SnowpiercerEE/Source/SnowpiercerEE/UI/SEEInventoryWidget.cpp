// SEEInventoryWidget.cpp
#include "SEEInventoryWidget.h"
#include "SEEInventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"

void USEEInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SelectedSlotIndex = -1;
}

void USEEInventoryWidget::InitInventory(USEEInventoryComponent* InInventory)
{
	InventoryComp = InInventory;
	if (InventoryComp.IsValid())
	{
		InventoryComp->OnInventoryChanged.AddDynamic(this, &USEEInventoryWidget::RefreshInventory);
	}
	RefreshInventory();
}

void USEEInventoryWidget::RefreshInventory()
{
	if (!InventoryComp.IsValid()) return;

	TArray<FSEEInventorySlot> Slots = InventoryComp->GetAllSlots();
	SortSlots(Slots);
	UpdateWeightDisplay();
	OnInventoryRefreshed(Slots);

	if (SelectedSlotIndex >= 0)
	{
		UpdateSelectedItemDetails();
	}
}

void USEEInventoryWidget::SetSortMode(ESEEInventorySortMode Mode)
{
	CurrentSortMode = Mode;

	if (SortModeText)
	{
		switch (Mode)
		{
		case ESEEInventorySortMode::ByName: SortModeText->SetText(NSLOCTEXT("Inv", "SortName", "Sort: Name")); break;
		case ESEEInventorySortMode::ByCategory: SortModeText->SetText(NSLOCTEXT("Inv", "SortCat", "Sort: Category")); break;
		case ESEEInventorySortMode::ByRarity: SortModeText->SetText(NSLOCTEXT("Inv", "SortRar", "Sort: Rarity")); break;
		case ESEEInventorySortMode::ByWeight: SortModeText->SetText(NSLOCTEXT("Inv", "SortWt", "Sort: Weight")); break;
		case ESEEInventorySortMode::ByValue: SortModeText->SetText(NSLOCTEXT("Inv", "SortVal", "Sort: Value")); break;
		default: SortModeText->SetText(NSLOCTEXT("Inv", "SortDef", "Sort: Default")); break;
		}
	}

	RefreshInventory();
}

void USEEInventoryWidget::SelectSlot(int32 SlotIndex)
{
	SelectedSlotIndex = SlotIndex;
	UpdateSelectedItemDetails();
}

void USEEInventoryWidget::UpdateSelectedItemDetails()
{
	if (!InventoryComp.IsValid() || SelectedSlotIndex < 0) return;

	TArray<FSEEInventorySlot> Slots = InventoryComp->GetAllSlots();
	if (!Slots.IsValidIndex(SelectedSlotIndex) || Slots[SelectedSlotIndex].IsEmpty())
	{
		OnSlotEmpty(SelectedSlotIndex);
		if (ItemNameText) ItemNameText->SetText(FText::GetEmpty());
		if (ItemDescText) ItemDescText->SetText(FText::GetEmpty());
		if (ItemStatsText) ItemStatsText->SetText(FText::GetEmpty());
		return;
	}

	FSEEItemData Data;
	if (InventoryComp->GetItemData(Slots[SelectedSlotIndex].ItemID, Data))
	{
		if (ItemNameText) ItemNameText->SetText(Data.DisplayName);
		if (ItemDescText) ItemDescText->SetText(Data.Description);

		if (ItemStatsText)
		{
			FString Stats;
			Stats += FString::Printf(TEXT("Weight: %.1f  Value: %d\n"), Data.Weight, Data.Value);
			Stats += FString::Printf(TEXT("Qty: %d / %d\n"), Slots[SelectedSlotIndex].Quantity, Data.MaxStackSize);

			if (Data.Category == ESEEItemCategory::Consumable)
			{
				if (Data.HealthRestore > 0.0f) Stats += FString::Printf(TEXT("+%.0f HP  "), Data.HealthRestore);
				if (Data.HungerRestore > 0.0f) Stats += FString::Printf(TEXT("+%.0f Hunger  "), Data.HungerRestore);
				if (Data.StaminaRestore > 0.0f) Stats += FString::Printf(TEXT("+%.0f Stamina  "), Data.StaminaRestore);
			}
			else if (Data.Category == ESEEItemCategory::Armor)
			{
				Stats += FString::Printf(TEXT("Blunt: %.0f  Bladed: %.0f  Pierce: %.0f  Cold: %.0f"),
					Data.BluntArmor, Data.BladedArmor, Data.PiercingArmor, Data.ColdResistance);
			}

			ItemStatsText->SetText(FText::FromString(Stats));
		}

		OnSlotSelected(SelectedSlotIndex, Data);
	}
}

void USEEInventoryWidget::UseSelectedItem()
{
	if (!InventoryComp.IsValid() || SelectedSlotIndex < 0) return;
	InventoryComp->UseItem(SelectedSlotIndex);
}

void USEEInventoryWidget::DropSelectedItem()
{
	if (!InventoryComp.IsValid() || SelectedSlotIndex < 0) return;
	InventoryComp->DropItem(SelectedSlotIndex);
}

void USEEInventoryWidget::AssignToQuickSlot(int32 QuickSlotIndex)
{
	if (!InventoryComp.IsValid() || SelectedSlotIndex < 0) return;
	InventoryComp->SetQuickSlot(QuickSlotIndex, SelectedSlotIndex);
}

bool USEEInventoryWidget::GetSelectedItemData(FSEEItemData& OutData) const
{
	if (!InventoryComp.IsValid() || SelectedSlotIndex < 0) return false;

	TArray<FSEEInventorySlot> Slots = InventoryComp->GetAllSlots();
	if (!Slots.IsValidIndex(SelectedSlotIndex) || Slots[SelectedSlotIndex].IsEmpty()) return false;

	return InventoryComp->GetItemData(Slots[SelectedSlotIndex].ItemID, OutData);
}

void USEEInventoryWidget::UpdateWeightDisplay()
{
	if (!InventoryComp.IsValid()) return;

	float Current = InventoryComp->GetCurrentWeight();
	float Max = InventoryComp->GetMaxWeight();

	if (WeightText)
	{
		FString Text = FString::Printf(TEXT("Weight: %.1f / %.1f"), Current, Max);
		if (InventoryComp->IsOverweight()) Text += TEXT(" [OVERWEIGHT]");
		WeightText->SetText(FText::FromString(Text));
	}

	if (WeightBar)
	{
		float Percent = Max > 0.0f ? Current / Max : 0.0f;
		WeightBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
		FLinearColor Color = Percent > 1.0f
			? FLinearColor::Red
			: (Percent > 0.8f ? FLinearColor(1.0f, 0.6f, 0.0f) : FLinearColor(0.3f, 0.7f, 0.3f));
		WeightBar->SetFillColorAndOpacity(Color);
	}
}

void USEEInventoryWidget::SortSlots(TArray<FSEEInventorySlot>& Slots) const
{
	if (CurrentSortMode == ESEEInventorySortMode::Default || !InventoryComp.IsValid()) return;

	Slots.Sort([this](const FSEEInventorySlot& A, const FSEEInventorySlot& B)
	{
		if (A.IsEmpty() && B.IsEmpty()) return false;
		if (A.IsEmpty()) return false;
		if (B.IsEmpty()) return true;

		FSEEItemData DataA, DataB;
		bool bHasA = InventoryComp->GetItemData(A.ItemID, DataA);
		bool bHasB = InventoryComp->GetItemData(B.ItemID, DataB);
		if (!bHasA || !bHasB) return bHasA;

		switch (CurrentSortMode)
		{
		case ESEEInventorySortMode::ByName:
			return DataA.DisplayName.CompareTo(DataB.DisplayName) < 0;
		case ESEEInventorySortMode::ByCategory:
			return static_cast<uint8>(DataA.Category) < static_cast<uint8>(DataB.Category);
		case ESEEInventorySortMode::ByRarity:
			return static_cast<uint8>(DataA.Rarity) > static_cast<uint8>(DataB.Rarity);
		case ESEEInventorySortMode::ByWeight:
			return DataA.Weight > DataB.Weight;
		case ESEEInventorySortMode::ByValue:
			return DataA.Value > DataB.Value;
		default:
			return false;
		}
	});
}

FLinearColor USEEInventoryWidget::GetRarityColor(ESEEItemRarity Rarity) const
{
	switch (Rarity)
	{
	case ESEEItemRarity::Common: return FLinearColor(0.7f, 0.7f, 0.7f);
	case ESEEItemRarity::Uncommon: return FLinearColor(0.3f, 0.8f, 0.3f);
	case ESEEItemRarity::Rare: return FLinearColor(0.3f, 0.5f, 1.0f);
	case ESEEItemRarity::Legendary: return FLinearColor(1.0f, 0.7f, 0.0f);
	default: return FLinearColor::White;
	}
}

FText USEEInventoryWidget::GetCategoryName(ESEEItemCategory Category) const
{
	switch (Category)
	{
	case ESEEItemCategory::Weapon: return NSLOCTEXT("Inv", "CatWeapon", "Weapon");
	case ESEEItemCategory::Armor: return NSLOCTEXT("Inv", "CatArmor", "Armor");
	case ESEEItemCategory::Consumable: return NSLOCTEXT("Inv", "CatConsumable", "Consumable");
	case ESEEItemCategory::Crafting: return NSLOCTEXT("Inv", "CatCrafting", "Crafting Material");
	case ESEEItemCategory::Quest: return NSLOCTEXT("Inv", "CatQuest", "Quest Item");
	case ESEEItemCategory::Junk: return NSLOCTEXT("Inv", "CatJunk", "Junk");
	default: return NSLOCTEXT("Inv", "CatUnknown", "Unknown");
	}
}
