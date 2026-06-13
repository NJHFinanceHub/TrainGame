#include "SEEInventoryComponent.h"
#include "SEEStatsComponent.h"
#include "SEEHealthComponent.h"
#include "SEEHungerComponent.h"
#include "Engine/DataTable.h"

namespace
{
	// Canonical item DataTable created by Scripts/create_datatables.py.
	const TCHAR* GSEEItemDataTablePath = TEXT("/Game/DataTables/DT_Items.DT_Items");
}

USEEInventoryComponent::USEEInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	QuickSlots.Init(INDEX_NONE, 4);
}

void USEEInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// BUG FIX: nothing (C++ or content) ever assigned ItemDataTable, so every
	// GetItemDataPtr() lookup failed: weight was always 0, UseItem() refused to
	// run, the UI fell back to raw ItemID names and Junk categorisation.
	// Resolve the project's canonical DT_Items as a default when unset.
	if (!ItemDataTable)
	{
		ItemDataTable = LoadObject<UDataTable>(nullptr, GSEEItemDataTablePath);
	}

	// AddItem may legitimately run before BeginPlay (quest rewards during world
	// init); EnsureSlots() may already have sized the array - don't shrink it.
	EnsureSlots();
}

void USEEInventoryComponent::EnsureSlots()
{
	if (Slots.Num() < MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}
}

bool USEEInventoryComponent::AddItem(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0) return false;

	// BUG FIX: items granted before BeginPlay hit an empty slot array and were
	// silently rejected (Slots was only sized in BeginPlay).
	EnsureSlots();

	const FSEEItemData* Data = GetItemDataPtr(ItemID);
	const int32 MaxStack = FMath::Max(1, Data ? Data->MaxStackSize : 1);
	const float ItemWeight = Data ? Data->Weight : 1.0f;

	// Check weight capacity
	if (GetCurrentWeight() + ItemWeight * Quantity > GetMaxWeight())
	{
		return false;
	}

	// BUG FIX: the old code mutated slots first and bailed out mid-add when it
	// ran out of empty slots - items vanished into slots with no OnItemAdded /
	// OnInventoryChanged broadcast and the pickup actor stayed in the world.
	// Pre-check slot capacity so the add is all-or-nothing.
	int32 Capacity = 0;
	for (const FSEEInventorySlot& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			Capacity += MaxStack;
		}
		else if (MaxStack > 1 && Slot.ItemID == ItemID && Slot.Quantity < MaxStack)
		{
			Capacity += MaxStack - Slot.Quantity;
		}
		if (Capacity >= Quantity) break;
	}
	if (Capacity < Quantity)
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
		if (EmptyIdx == INDEX_NONE) return false; // unreachable after pre-check

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

		// BUG FIX: the old comment claimed hunger restore was "handled via
		// delegates from OnItemUsed" - no such delegate exists, so food items
		// never fed the player. Apply HungerRestore directly.
		if (Data->HungerRestore > 0.0f)
		{
			if (USEEHungerComponent* Hunger = Owner->FindComponentByClass<USEEHungerComponent>())
			{
				Hunger->Eat(Data->HungerRestore);
			}
		}

		// StaminaRestore: no stamina pool component exposes a restore API yet;
		// intentionally left until one exists.

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

	if (const FSEEItemData* Row = ItemDataTable->FindRow<FSEEItemData>(ItemID, TEXT(""), /*bWarnIfRowMissing*/ false))
	{
		return Row;
	}

	// BUG FIX: world pickups / legacy saves carry short IDs ("Bandage",
	// "ScrapMetal") while DT_Items rows are prefixed ("Item_Bandage",
	// "Item_ScrapMetal"). Try the canonical prefixed row before giving up.
	if (!ItemID.ToString().StartsWith(TEXT("Item_")))
	{
		const FName PrefixedID(*(TEXT("Item_") + ItemID.ToString()));
		return ItemDataTable->FindRow<FSEEItemData>(PrefixedID, TEXT(""), /*bWarnIfRowMissing*/ false);
	}

	return nullptr;
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

TArray<FSEEItemSaveEntry> USEEInventoryComponent::GetSaveState() const
{
	TArray<FSEEItemSaveEntry> Out;
	for (const FSEEInventorySlot& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			FSEEItemSaveEntry& Entry = Out.AddDefaulted_GetRef();
			Entry.ItemID = Slot.ItemID;
			Entry.Count = Slot.Quantity;
			Entry.Durability = 0.0f; // slots don't track durability today
		}
	}
	return Out;
}

void USEEInventoryComponent::SetSaveState(const TArray<FSEEItemSaveEntry>& Entries)
{
	EnsureSlots();

	// Clear current contents (keep the array sized so the UI's slot grid is stable).
	for (FSEEInventorySlot& Slot : Slots)
	{
		Slot.ItemID = NAME_None;
		Slot.Quantity = 0;
	}

	// Restore preserving slot order; grow the array if a save carries more
	// stacks than the default MaxSlots (e.g. capacity changed between versions).
	int32 Index = 0;
	for (const FSEEItemSaveEntry& Entry : Entries)
	{
		if (Entry.ItemID.IsNone() || Entry.Count <= 0)
		{
			continue;
		}
		if (Index >= Slots.Num())
		{
			Slots.SetNum(Index + 1);
		}
		Slots[Index].ItemID = Entry.ItemID;
		Slots[Index].Quantity = Entry.Count;
		++Index;
	}

	OnInventoryChanged.Broadcast();
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
