// InventoryComponent.cpp - Inventory system implementation
#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

// --- Item Operations ---

int32 UInventoryComponent::AddItem(FName ItemID, int32 Count, float Durability)
{
	if (ItemID.IsNone() || Count <= 0) return 0;

	float ItemWeight = GetItemWeight(ItemID);
	int32 MaxStack = GetItemMaxStack(ItemID);
	float CurrentW = GetCurrentWeight();
	int32 Added = 0;

	for (int32 i = 0; i < Count; ++i)
	{
		// Check weight limit (quest items bypass weight)
		FItemDefinition Def;
		bool bIsQuest = GetItemDefinition(ItemID, Def) && Def.bIsQuestItem;
		if (!bIsQuest && (CurrentW + ItemWeight) > MaxCarryWeight)
		{
			break;
		}

		// Try to stack with existing
		FInventoryItem* Existing = FindStackableSlot(ItemID);
		if (Existing && Existing->StackCount < MaxStack)
		{
			Existing->StackCount++;
		}
		else
		{
			// Create new stack
			FInventoryItem NewItem;
			NewItem.ItemID = ItemID;
			NewItem.StackCount = 1;
			NewItem.CurrentDurability = Durability;
			Items.Add(NewItem);

			FInventoryItem& AddedItem = Items.Last();
			OnItemAdded.Broadcast(AddedItem);
		}

		CurrentW += ItemWeight;
		Added++;
	}

	if (Added > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Added;
}

int32 UInventoryComponent::RemoveItem(FName ItemID, int32 Count)
{
	if (ItemID.IsNone() || Count <= 0) return 0;

	int32 Removed = 0;

	for (int32 i = Items.Num() - 1; i >= 0 && Removed < Count; --i)
	{
		if (Items[i].ItemID != ItemID) continue;

		int32 ToRemove = FMath::Min(Items[i].StackCount, Count - Removed);
		Items[i].StackCount -= ToRemove;
		Removed += ToRemove;

		if (Items[i].StackCount <= 0)
		{
			FInventoryItem RemovedItem = Items[i];
			Items.RemoveAt(i);
			OnItemRemoved.Broadcast(RemovedItem);
		}
	}

	if (Removed > 0)
	{
		OnInventoryChanged.Broadcast();
	}

	return Removed;
}

bool UInventoryComponent::RemoveItemByInstance(const FGuid& InstanceID)
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].InstanceID == InstanceID)
		{
			FInventoryItem RemovedItem = Items[i];
			Items.RemoveAt(i);
			OnItemRemoved.Broadcast(RemovedItem);
			OnInventoryChanged.Broadcast();
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasItem(FName ItemID, int32 Count) const
{
	return GetItemCount(ItemID) >= Count;
}

int32 UInventoryComponent::GetItemCount(FName ItemID) const
{
	int32 Total = 0;
	for (const FInventoryItem& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			Total += Item.StackCount;
		}
	}
	return Total;
}

bool UInventoryComponent::FindItem(FName ItemID, FInventoryItem& OutItem) const
{
	for (const FInventoryItem& Item : Items)
	{
		if (Item.ItemID == ItemID)
		{
			OutItem = Item;
			return true;
		}
	}
	return false;
}

// --- Weight ---

float UInventoryComponent::GetCurrentWeight() const
{
	float Total = 0.0f;
	for (const FInventoryItem& Item : Items)
	{
		Total += GetItemWeight(Item.ItemID) * Item.StackCount;
	}
	return Total;
}

float UInventoryComponent::GetWeightPercent() const
{
	if (MaxCarryWeight <= 0.0f) return 0.0f;
	return GetCurrentWeight() / MaxCarryWeight;
}

bool UInventoryComponent::IsOverweight() const
{
	return GetCurrentWeight() > MaxCarryWeight;
}

// --- Durability ---

bool UInventoryComponent::DegradeDurability(const FGuid& InstanceID, float Amount)
{
	FInventoryItem* Item = FindItemInstance(InstanceID);
	if (!Item || Item->CurrentDurability < 0.0f) return false;

	Item->CurrentDurability = FMath::Max(0.0f, Item->CurrentDurability - Amount);

	if (Item->CurrentDurability <= 0.0f)
	{
		// Item broke
		RemoveItemByInstance(InstanceID);
		return true;
	}

	OnInventoryChanged.Broadcast();
	return false;
}

void UInventoryComponent::RepairItem(const FGuid& InstanceID, float Amount)
{
	FInventoryItem* Item = FindItemInstance(InstanceID);
	if (!Item || Item->CurrentDurability < 0.0f) return;

	FItemDefinition Def;
	if (GetItemDefinition(Item->ItemID, Def))
	{
		Item->CurrentDurability = FMath::Min(Item->CurrentDurability + Amount, Def.MaxDurability);
		OnInventoryChanged.Broadcast();
	}
}

// --- Special Resources ---

void UInventoryComponent::ModifyScrap(int32 Delta)
{
	Scrap = FMath::Max(0, Scrap + Delta);
	OnInventoryChanged.Broadcast();
}

void UInventoryComponent::ModifyInfluence(float Delta)
{
	Influence = FMath::Max(0.0f, Influence + Delta);
	OnInventoryChanged.Broadcast();
}

// --- Data Table ---

bool UInventoryComponent::GetItemDefinition(FName ItemID, FItemDefinition& OutDef) const
{
	if (!ItemDataTable) return false;

	FItemDefinition* Row = ItemDataTable->FindRow<FItemDefinition>(ItemID, TEXT("GetItemDefinition"));
	if (Row)
	{
		OutDef = *Row;
		return true;
	}
	return false;
}

// --- Private ---

float UInventoryComponent::GetItemWeight(FName ItemID) const
{
	FItemDefinition Def;
	if (GetItemDefinition(ItemID, Def))
	{
		return Def.Weight;
	}
	return 1.0f; // Fallback weight
}

int32 UInventoryComponent::GetItemMaxStack(FName ItemID) const
{
	FItemDefinition Def;
	if (GetItemDefinition(ItemID, Def))
	{
		return Def.MaxStackSize;
	}
	return 1;
}

FInventoryItem* UInventoryComponent::FindItemInstance(const FGuid& InstanceID)
{
	for (FInventoryItem& Item : Items)
	{
		if (Item.InstanceID == InstanceID)
		{
			return &Item;
		}
	}
	return nullptr;
}

FInventoryItem* UInventoryComponent::FindStackableSlot(FName ItemID)
{
	int32 MaxStack = GetItemMaxStack(ItemID);
	for (FInventoryItem& Item : Items)
	{
		if (Item.ItemID == ItemID && Item.StackCount < MaxStack)
		{
			return &Item;
		}
	}
	return nullptr;
}
