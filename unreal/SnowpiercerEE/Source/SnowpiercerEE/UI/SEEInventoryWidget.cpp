#include "SEEInventoryWidget.h"

static const FSEEInventorySlot EmptySlot;

void USEEInventoryWidget::SetGridSize(int32 Columns, int32 Rows)
{
	GridColumns = FMath::Max(1, Columns);
	GridRows = FMath::Max(1, Rows);
	const int32 TotalSlots = GridColumns * GridRows;
	InventorySlots.SetNum(TotalSlots);
	OnGridChanged();
}

void USEEInventoryWidget::SetSlot(int32 Index, const FSEEInventorySlot& Slot)
{
	if (InventorySlots.IsValidIndex(Index))
	{
		InventorySlots[Index] = Slot;
		OnSlotUpdated(Index);
	}
}

void USEEInventoryWidget::ClearSlot(int32 Index)
{
	if (InventorySlots.IsValidIndex(Index))
	{
		InventorySlots[Index] = FSEEInventorySlot();
		OnSlotUpdated(Index);
	}
}

void USEEInventoryWidget::SetEquipmentSlot(ESEEEquipmentSlot Slot, const FSEEInventorySlot& Item)
{
	EquipmentSlots.Add(Slot, Item);
	OnEquipmentSlotUpdated(Slot);
}

void USEEInventoryWidget::ClearEquipmentSlot(ESEEEquipmentSlot Slot)
{
	EquipmentSlots.Remove(Slot);
	OnEquipmentSlotUpdated(Slot);
}

const FSEEInventorySlot& USEEInventoryWidget::GetSlot(int32 Index) const
{
	if (InventorySlots.IsValidIndex(Index))
	{
		return InventorySlots[Index];
	}
	return EmptySlot;
}
