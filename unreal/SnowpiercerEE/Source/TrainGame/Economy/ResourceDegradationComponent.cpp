// ResourceDegradationComponent.cpp - Perishable item degradation implementation
#include "ResourceDegradationComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"

UResourceDegradationComponent::UResourceDegradationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Tick once per second
}

void UResourceDegradationComponent::BeginPlay()
{
	Super::BeginPlay();
	LinkedInventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
}

void UResourceDegradationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (SecondsPerGameDay <= 0.0f)
	{
		return;
	}

	TArray<FGuid> ToRemove;

	for (auto& Pair : TrackedItems)
	{
		FDegradationEntry& Entry = Pair.Value;
		if (Entry.bPaused || Entry.DegradationRatePerDay <= 0.0f)
		{
			continue;
		}

		float OldCondition = Entry.Condition;
		float DecayThisTick = (Entry.DegradationRatePerDay / SecondsPerGameDay) * DeltaTime;
		Entry.Condition = FMath::Max(0.0f, Entry.Condition - DecayThisTick);

		// Broadcast on threshold crossings (every 25%)
		int32 OldBucket = FMath::FloorToInt(OldCondition * 4.0f);
		int32 NewBucket = FMath::FloorToInt(Entry.Condition * 4.0f);
		if (OldBucket != NewBucket)
		{
			OnItemDegraded.Broadcast(Entry.ItemID, Entry.Condition);
		}

		if (Entry.Condition <= 0.0f)
		{
			OnItemDestroyed.Broadcast(Entry.ItemID);
			if (LinkedInventory)
			{
				LinkedInventory->RemoveItemByInstance(Pair.Key);
			}
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FGuid& ID : ToRemove)
	{
		TrackedItems.Remove(ID);
	}
}

float UResourceDegradationComponent::GetItemCondition(const FGuid& InstanceID) const
{
	const FDegradationEntry* Entry = TrackedItems.Find(InstanceID);
	return Entry ? Entry->Condition : 1.0f;
}

FText UResourceDegradationComponent::GetConditionLabel(const FGuid& InstanceID) const
{
	float Condition = GetItemCondition(InstanceID);
	if (Condition > 0.75f) return FText::FromString(TEXT("Fresh"));
	if (Condition > 0.50f) return FText::FromString(TEXT("Worn"));
	if (Condition > 0.25f) return FText::FromString(TEXT("Degraded"));
	return FText::FromString(TEXT("Ruined"));
}

float UResourceDegradationComponent::GetEffectivenessMultiplier(const FGuid& InstanceID) const
{
	float Condition = GetItemCondition(InstanceID);
	if (Condition > 0.75f) return 1.0f;
	if (Condition > 0.50f) return 0.75f;
	if (Condition > 0.25f) return 0.50f;
	return 0.25f;
}

void UResourceDegradationComponent::PauseItemDegradation(const FGuid& InstanceID)
{
	FDegradationEntry* Entry = TrackedItems.Find(InstanceID);
	if (Entry)
	{
		Entry->bPaused = true;
	}
}

void UResourceDegradationComponent::ResumeItemDegradation(const FGuid& InstanceID)
{
	FDegradationEntry* Entry = TrackedItems.Find(InstanceID);
	if (Entry)
	{
		Entry->bPaused = false;
	}
}

void UResourceDegradationComponent::SetItemCondition(const FGuid& InstanceID, float Condition)
{
	FDegradationEntry* Entry = TrackedItems.Find(InstanceID);
	if (Entry)
	{
		Entry->Condition = FMath::Clamp(Condition, 0.0f, 1.0f);
	}
}

void UResourceDegradationComponent::RegisterItem(const FGuid& InstanceID, FName ItemID)
{
	float Rate = GetDegradationRate(ItemID);

	FDegradationEntry Entry;
	Entry.ItemID = ItemID;
	Entry.Condition = 1.0f;
	Entry.DegradationRatePerDay = Rate;
	Entry.bPaused = false;

	TrackedItems.Add(InstanceID, Entry);
}

void UResourceDegradationComponent::UnregisterItem(const FGuid& InstanceID)
{
	TrackedItems.Remove(InstanceID);
}

float UResourceDegradationComponent::GetDegradationRate(FName ItemID) const
{
	if (!ResourceEconomyDataTable)
	{
		return 0.0f;
	}

	static const FString ContextString(TEXT("DegradationRateLookup"));
	FResourceEconomyData* Data = ResourceEconomyDataTable->FindRow<FResourceEconomyData>(ItemID, ContextString);
	return Data ? Data->DegradationRatePerDay : 0.0f;
}
