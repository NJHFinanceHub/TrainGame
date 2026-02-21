// BarterComponent.cpp - Barter trading system implementation
#include "BarterComponent.h"
#include "MerchantComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"

UBarterComponent::UBarterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UBarterComponent::BeginPlay()
{
	Super::BeginPlay();
	PlayerInventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
}

bool UBarterComponent::BeginTrade(UMerchantComponent* Merchant)
{
	if (!Merchant || ActiveMerchant)
	{
		return false;
	}

	ActiveMerchant = Merchant;
	CurrentProposal = FTradeProposal();
	return true;
}

void UBarterComponent::EndTrade()
{
	ActiveMerchant = nullptr;
	CurrentProposal = FTradeProposal();
}

void UBarterComponent::SetCurrentZone(ETrainZone Zone)
{
	CurrentZone = Zone;
}

float UBarterComponent::CalculateItemValue(FName ItemID, ETrainZone Zone, float Condition) const
{
	if (!ResourceEconomyDataTable)
	{
		return 0.0f;
	}

	static const FString ContextString(TEXT("BarterValueCalc"));
	FResourceEconomyData* Data = ResourceEconomyDataTable->FindRow<FResourceEconomyData>(ItemID, ContextString);
	if (!Data)
	{
		return 0.0f;
	}

	float ZoneMultiplier = 1.0f;
	for (const FZoneValueEntry& Entry : Data->ZoneMultipliers)
	{
		if (Entry.Zone == Zone)
		{
			ZoneMultiplier = Entry.ValueMultiplier;
			break;
		}
	}

	float ConditionFactor = FMath::Clamp(Condition, 0.0f, 1.0f);
	return Data->BaseTradeValue * ZoneMultiplier * ConditionFactor;
}

float UBarterComponent::CalculateItemValueWithFaction(FName ItemID, ETrainZone Zone, EFaction MerchantFaction, float Condition) const
{
	float BaseValue = CalculateItemValue(ItemID, Zone, Condition);
	return BaseValue * GetFactionPriceModifier(MerchantFaction);
}

EHaggleResult UBarterComponent::ProposeTradeOffer(const FTradeProposal& Proposal)
{
	if (!ActiveMerchant)
	{
		return EHaggleResult::Rejected;
	}

	CurrentProposal = Proposal;
	CurrentProposal.HaggleRoundsUsed = 0;
	CurrentProposal.HaggleModifier = 1.0f;

	// Calculate offer values
	float PlayerValue = 0.0f;
	for (const auto& Pair : Proposal.PlayerOffer.Items)
	{
		PlayerValue += CalculateItemValue(Pair.Key, CurrentZone) * Pair.Value;
	}

	float MerchantValue = 0.0f;
	for (const auto& Pair : Proposal.MerchantOffer.Items)
	{
		MerchantValue += CalculateItemValue(Pair.Key, CurrentZone) * Pair.Value;
	}

	CurrentProposal.PlayerOffer.TotalValue = PlayerValue;
	CurrentProposal.MerchantOffer.TotalValue = MerchantValue;

	// Merchant accepts if player offers >= merchant's sell value
	if (PlayerValue >= MerchantValue)
	{
		return EHaggleResult::Accepted;
	}

	// Too low — merchant may counter or reject
	float Ratio = PlayerValue / FMath::Max(MerchantValue, 0.01f);
	if (Ratio < 0.5f)
	{
		return EHaggleResult::MerchantAngry;
	}

	return EHaggleResult::CounterOffer;
}

EHaggleResult UBarterComponent::Haggle()
{
	if (!ActiveMerchant)
	{
		return EHaggleResult::Rejected;
	}

	CurrentProposal.HaggleRoundsUsed++;

	// TODO: Read merchant's MaxHaggleRounds from config
	if (CurrentProposal.HaggleRoundsUsed > 3)
	{
		EHaggleResult Result = EHaggleResult::Rejected;
		OnHaggleRound.Broadcast(CurrentProposal.HaggleRoundsUsed, Result);
		return Result;
	}

	// TODO: Read player's Social stat for haggle modifier calculation
	float SocialStat = 5.0f; // Placeholder
	CurrentProposal.HaggleModifier = CalculateHaggleModifier(CurrentProposal.HaggleRoundsUsed, SocialStat);

	EHaggleResult Result = EHaggleResult::CounterOffer;
	OnHaggleRound.Broadcast(CurrentProposal.HaggleRoundsUsed, Result);
	return Result;
}

bool UBarterComponent::AcceptCurrentOffer()
{
	if (!ActiveMerchant || !PlayerInventory)
	{
		return false;
	}

	// Validate both sides have the items
	if (!ValidateTradeItems(CurrentProposal.PlayerOffer, PlayerInventory))
	{
		return false;
	}

	// Remove player's items
	for (const auto& Pair : CurrentProposal.PlayerOffer.Items)
	{
		PlayerInventory->RemoveItem(Pair.Key, Pair.Value);
	}

	// Add merchant's items to player
	for (const auto& Pair : CurrentProposal.MerchantOffer.Items)
	{
		PlayerInventory->AddItem(Pair.Key, Pair.Value);
	}

	OnTradeCompleted.Broadcast(CurrentProposal, NAME_None);
	EndTrade();
	return true;
}

bool UBarterComponent::IsItemStolen(const FGuid& InstanceID) const
{
	return StolenItems.Contains(InstanceID);
}

void UBarterComponent::MarkItemStolen(const FGuid& InstanceID)
{
	// Store the game time when the item was stolen
	StolenItems.Add(InstanceID, GetWorld()->GetTimeSeconds());
}

void UBarterComponent::ClearStolenFlag(const FGuid& InstanceID)
{
	StolenItems.Remove(InstanceID);
}

float UBarterComponent::GetFactionPriceModifier(EFaction Faction) const
{
	// TODO: Query FactionReputationComponent for standing with this faction
	// Hostile: cannot trade, Unfriendly: 1.3, Neutral: 1.0, Friendly: 0.9, Allied: 0.8
	return 1.0f;
}

float UBarterComponent::CalculateHaggleModifier(int32 Round, float SocialStat) const
{
	// Higher social stat = better haggle outcomes
	// Base improvement per round: 5% + (SocialStat * 1%)
	float Improvement = 0.05f + (SocialStat * 0.01f);
	return 1.0f + (Improvement * Round);
}

bool UBarterComponent::ValidateTradeItems(const FTradeOffer& Offer, UInventoryComponent* Source) const
{
	if (!Source)
	{
		return false;
	}

	for (const auto& Pair : Offer.Items)
	{
		if (!Source->HasItem(Pair.Key, Pair.Value))
		{
			return false;
		}
	}

	return true;
}
