#include "SEEInventoryComponent.h"
#include "SEEStatsComponent.h"
#include "SEEHealthComponent.h"

USEEInventoryComponent::USEEInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	QuickSlots.Init(INDEX_NONE, 4);
}

void USEEInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	Slots.SetNum(MaxSlots);
}

bool USEEInventoryComponent::AddItem(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0) return false;

	const FSEEItemData* Data = GetItemDataPtr(ItemID);
	int32 MaxStack = Data ? Data->MaxStackSize : 1;
	float ItemWeight = Data ? Data->Weight : 1.0f;

	// Check weight capacity
	if (GetCurrentWeight() + ItemWeight * Quantity > GetMaxWeight())
	{
		return false;
	}

	int32 Remaining = Quantity;

	// Try stacking on existing slots
	if (MaxStack > 1)
	{
		for (FSEEInventorySlot& Slot : Slots)
		{
			if (Slot.ItemID == ItemID && Slot.Quantity < MaxStack)
			{
				int32 CanAdd = FMath::Min(Remaining, MaxStack - Slot.Quantity);
				Slot.Quantity += CanAdd;
				Remaining -= CanAdd;
				if (Remaining <= 0) break;
			}
		}
	}

	// Place remaining in empty slots
	while (Remaining > 0)
	{
		int32 EmptyIdx = FindEmptySlot();
		if (EmptyIdx == INDEX_NONE) return false;

		int32 CanAdd = FMath::Min(Remaining, MaxStack);
		Slots[EmptyIdx].ItemID = ItemID;
		Slots[EmptyIdx].Quantity = CanAdd;
		Remaining -= CanAdd;
	}

	OnItemAdded.Broadcast(ItemID, Quantity);
	OnInventoryChanged.Broadcast();
	return true;
}

bool USEEInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
	if (!HasItem(ItemID, Quantity)) return false;

	int32 Remaining = Quantity;
	for (FSEEInventorySlot& Slot : Slots)
	{
		if (Slot.ItemID == ItemID)
		{
			int32 CanRemove = FMath::Min(Remaining, Slot.Quantity);
			Slot.Quantity -= CanRemove;
			Remaining -= CanRemove;

			if (Slot.Quantity <= 0)
			{
				Slot.ItemID = NAME_None;
				Slot.Quantity = 0;
			}

			if (Remaining <= 0) break;
		}
	}

	OnItemRemoved.Broadcast(ItemID, Quantity);
	OnInventoryChanged.Broadcast();
	return true;
}

void USEEInventoryComponent::DropItem(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= Slots.Num()) return;
	if (Slots[SlotIndex].IsEmpty()) return;

	const FSEEItemData* Data = GetItemDataPtr(Slots[SlotIndex].ItemID);
	if (Data && Data->Category == ESEEItemCategory::Quest) return; // Can't drop quest items

	OnItemRemoved.Broadcast(Slots[SlotIndex].ItemID, Slots[SlotIndex].Quantity);
	Slots[SlotIndex].ItemID = NAME_None;
	Slots[SlotIndex].Quantity = 0;
	OnInventoryChanged.Broadcast();
}

bool USEEInventoryComponent::UseItem(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= Slots.Num()) return false;
	if (Slots[SlotIndex].IsEmpty()) return false;

	const FSEEItemData* Data = GetItemDataPtr(Slots[SlotIndex].ItemID);
	if (!Data) return false;

	if (Data->Category == ESEEItemCategory::Consumable)
	{
		AActor* Owner = GetOwner();
		if (!Owner) return false;

		if (Data->HealthRestore > 0.0f)
		{
			if (USEEHealthComponent* Health = Owner->FindComponentByClass<USEEHealthComponent>())
			{
				Health->Heal(Data->HealthRestore);
			}
		}

		// Hunger and stamina restore handled by their respective components
		// via delegates from OnItemUsed

		Slots[SlotIndex].Quantity--;
		if (Slots[SlotIndex].Quantity <= 0)
		{
			Slots[SlotIndex].ItemID = NAME_None;
			Slots[SlotIndex].Quantity = 0;
		}

		OnInventoryChanged.Broadcast();
		return true;
	}

	return false;
}

void USEEInventoryComponent::SetQuickSlot(int32 QuickSlotIndex, int32 InventorySlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= 4) return;
	QuickSlots[QuickSlotIndex] = InventorySlotIndex;
}

void USEEInventoryComponent::UseQuickSlot(int32 QuickSlotIndex)
{
	if (QuickSlotIndex < 0 || QuickSlotIndex >= 4) return;
	int32 SlotIdx = QuickSlots[QuickSlotIndex];
	if (SlotIdx != INDEX_NONE)
	{
		UseItem(SlotIdx);
	}
}

bool USEEInventoryComponent::HasItem(FName ItemID, int32 Quantity) const
{
	return GetItemCount(ItemID) >= Quantity;
}

int32 USEEInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Total = 0;
	for (const FSEEInventorySlot& Slot : Slots)
	{
		if (Slot.ItemID == ItemID)
		{
			Total += Slot.Quantity;
		}
	}
	return Total;
}

float USEEInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.0f;
	for (const FSEEInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			const FSEEItemData* Data = GetItemDataPtr(Slot.ItemID);
			if (Data)
			{
				Total += Data->Weight * Slot.Quantity;
			}
		}
	}
	return Total;
}

float USEEInventoryComponent::GetMaxWeight() const
{
	float Max = BaseCarryCapacity;
	if (AActor* Owner = GetOwner())
	{
		if (USEEStatsComponent* Stats = Owner->FindComponentByClass<USEEStatsComponent>())
		{
			Max += Stats->GetStat(ESEEStat::Strength) * StrengthWeightBonus;
		}
	}
	return Max;
}

const FSEEItemData* USEEInventoryComponent::GetItemDataPtr(FName ItemID) const
{
	if (!ItemDataTable || ItemID.IsNone()) return nullptr;
	return ItemDataTable->FindRow<FSEEItemData>(ItemID, TEXT(""));
}

bool USEEInventoryComponent::GetItemData(FName ItemID, FSEEItemData& OutData) const
{
	const FSEEItemData* Data = GetItemDataPtr(ItemID);
	if (Data)
	{
		OutData = *Data;
		return true;
	}
	return false;
}

int32 USEEInventoryComponent::FindSlotWithItem(FName ItemID) const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].ItemID == ItemID) return i;
	}
	return INDEX_NONE;
}

int32 USEEInventoryComponent::FindEmptySlot() const
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].IsEmpty()) return i;
	}
	return INDEX_NONE;
}
