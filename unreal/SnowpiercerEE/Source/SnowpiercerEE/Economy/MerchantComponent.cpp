// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// MerchantComponent.cpp - NPC merchant implementation

#include "MerchantComponent.h"

UMerchantComponent::UMerchantComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMerchantComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshInventory();
}

TArray<FMerchantInventorySlot> UMerchantComponent::GetAvailableInventory(int32 PlayerFavorTier) const
{
	TArray<FMerchantInventorySlot> Available;
	for (const FMerchantInventorySlot& Slot : CurrentInventory)
	{
		if (Slot.Quantity > 0 && Slot.RequiredFavorTier <= PlayerFavorTier)
		{
			Available.Add(Slot);
		}
	}
	return Available;
}

bool UMerchantComponent::HasItemInStock(FName ItemID, int32 Quantity) const
{
	for (const FMerchantInventorySlot& Slot : CurrentInventory)
	{
		if (Slot.ItemID == ItemID && Slot.Quantity >= Quantity)
		{
			return true;
		}
	}
	return false;
}

void UMerchantComponent::RemoveFromStock(FName ItemID, int32 Quantity)
{
	for (FMerchantInventorySlot& Slot : CurrentInventory)
	{
		if (Slot.ItemID == ItemID)
		{
			Slot.Quantity = FMath::Max(0, Slot.Quantity - Quantity);
			return;
		}
	}
}

void UMerchantComponent::AddToStock(FName ItemID, int32 Quantity)
{
	for (FMerchantInventorySlot& Slot : CurrentInventory)
	{
		if (Slot.ItemID == ItemID)
		{
			Slot.Quantity += Quantity;
			return;
		}
	}

	FMerchantInventorySlot NewSlot;
	NewSlot.ItemID = ItemID;
	NewSlot.Quantity = Quantity;
	NewSlot.PriceMultiplier = 1.0f;
	CurrentInventory.Add(NewSlot);
}

void UMerchantComponent::RefreshInventory()
{
	CurrentInventory = BaseInventory;
	LastRefreshGameTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UMerchantComponent::CompleteTask()
{
	CompletedTasks++;
	RecalculateFavorTier();
}

void UMerchantComponent::RecordTrade()
{
	CompletedTrades++;
}

void UMerchantComponent::RecalculateFavorTier()
{
	if (CompletedTasks >= 10)
	{
		FavorTier = 3;
	}
	else if (CompletedTasks >= 5)
	{
		FavorTier = 2;
	}
	else if (CompletedTasks >= 2)
	{
		FavorTier = 1;
	}
	else
	{
		FavorTier = 0;
	}
}
