#include "SEEInventoryComponent.h"
#include "SEEStatsComponent.h"
#include "SEEHealthComponent.h"
#include "SEEHungerComponent.h"
#include "Actors/SEEPickupActor.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

namespace
{
	// Canonical item DataTable created by Scripts/create_datatables.py.
	const TCHAR* GSEEItemDataTablePath = TEXT("/Game/DataTables/DT_Items.DT_Items");
}

USEEInventoryComponent::USEEInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	QuickSlots.Init(INDEX_NONE, 4);

	// Server-authoritative inventory; replicates the Slots array to the owning
	// client so co-op pickups show up in that player's UI (see Slots/OnRep_Slots).
	SetIsReplicatedByDefault(true);
}

void USEEInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// COND_OwnerOnly: a player's bag replicates only to the client that owns the
	// pawn, never to the other co-op players. The server (and standalone) already
	// has the authoritative array; this pushes server-side AddItem results down to
	// the right player's owning client.
	DOREPLIFETIME_CONDITION(USEEInventoryComponent, Slots, COND_OwnerOnly);
}

void USEEInventoryComponent::OnRep_Slots()
{
	// Clients learned the server mutated the bag (a pickup grant, a use, a drop):
	// refresh the inventory UI. Runs only on the owning client; authority drives
	// the same broadcast inline from each mutating call so single-player is intact.
	OnInventoryChanged.Broadcast();
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

bool USEEInventoryComponent::IsOwnerAuthority() const
{
	const AActor* Owner = GetOwner();
	// No owner (test stub) or actual authority both run the mutation directly.
	return !Owner || Owner->HasAuthority();
}

void USEEInventoryComponent::DropItem(int32 SlotIndex)
{
	// CO-OP: on a client, ask the server to perform the authoritative drop; the
	// replicated Slots array + OnRep_Slots refresh this client's UI, and the
	// server-spawned ASEEPickupActor replicates to everyone. Authority (host /
	// standalone) drops directly — single-player path unchanged.
	if (!IsOwnerAuthority())
	{
		ServerDropItem(SlotIndex);
		return;
	}

	if (SlotIndex < 0 || SlotIndex >= Slots.Num()) return;
	if (Slots[SlotIndex].IsEmpty()) return;

	const FSEEItemData* Data = GetItemDataPtr(Slots[SlotIndex].ItemID);
	if (Data && Data->Category == ESEEItemCategory::Quest) return; // Can't drop quest items

	const FName DroppedItemID = Slots[SlotIndex].ItemID;
	const int32 DroppedQuantity = Slots[SlotIndex].Quantity;

	OnItemRemoved.Broadcast(DroppedItemID, DroppedQuantity);
	Slots[SlotIndex].ItemID = NAME_None;
	Slots[SlotIndex].Quantity = 0;
	OnInventoryChanged.Broadcast();

	// Spawn the dropped stack as a world pickup so it can be re-collected. Authority
	// only (the pickup actor replicates down to clients); standalone runs it inline.
	SpawnDroppedPickup(DroppedItemID, DroppedQuantity);
}

bool USEEInventoryComponent::ServerDropItem_Validate(int32 SlotIndex) { return true; }
void USEEInventoryComponent::ServerDropItem_Implementation(int32 SlotIndex)
{
	// Runs on the server with authority — the wrapper's authority branch performs
	// the real mutation + pickup spawn.
	DropItem(SlotIndex);
}

bool USEEInventoryComponent::UseItem(int32 SlotIndex)
{
	// CO-OP: clients forward to the server; the heal/eat applies through the
	// server-authoritative health/hunger components and replicates, and the
	// consumed slot replicates back to refresh this client's UI.
	if (!IsOwnerAuthority())
	{
		ServerUseItem(SlotIndex);
		return true; // optimistic: the authoritative result arrives via replication
	}

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

	// The quickslot -> inventory-slot mapping (QuickSlots) is a per-client binding
	// set locally via SetQuickSlot, so the server doesn't share it. Resolve to the
	// concrete inventory slot index HERE, then route through UseItem — which itself
	// branches to ServerUseItem on a client. This keeps the server authoritative on
	// the actual consume while honoring the local quickslot binding.
	const int32 SlotIdx = QuickSlots[QuickSlotIndex];
	if (SlotIdx != INDEX_NONE)
	{
		UseItem(SlotIdx);
	}
}

bool USEEInventoryComponent::ServerUseItem_Validate(int32 SlotIndex) { return true; }
void USEEInventoryComponent::ServerUseItem_Implementation(int32 SlotIndex)
{
	// Authority: run the real consume (heal/eat + slot decrement). The wrapper's
	// authority branch does exactly this.
	UseItem(SlotIndex);
}

bool USEEInventoryComponent::ServerUseQuickSlot_Validate(int32 QuickSlotIndex) { return true; }
void USEEInventoryComponent::ServerUseQuickSlot_Implementation(int32 QuickSlotIndex)
{
	// Provided for completeness / Blueprint callers. The server's QuickSlots
	// mapping may differ from the client's local binding, so prefer the
	// UseQuickSlot path (which resolves the slot client-side then calls ServerUseItem).
	UseQuickSlot(QuickSlotIndex);
}

void USEEInventoryComponent::SpawnDroppedPickup(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0) return;

	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World) return;

	// Drop in front of and below the owner so it lands ahead of their feet. Far
	// enough (forward 180cm vs the pickup's 100cm overlap sphere) that the dropper
	// isn't standing inside it — otherwise BeginPlay's initial-overlap sweep would
	// instantly re-grant the just-dropped stack.
	const FVector DropLoc = Owner->GetActorLocation()
		+ Owner->GetActorForwardVector() * 180.0f
		+ FVector(0.0f, 0.0f, -40.0f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASEEPickupActor* Pickup = World->SpawnActor<ASEEPickupActor>(
		ASEEPickupActor::StaticClass(), DropLoc, FRotator::ZeroRotator, Params);
	if (Pickup)
	{
		Pickup->InitPickup(ItemID, Quantity);
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
