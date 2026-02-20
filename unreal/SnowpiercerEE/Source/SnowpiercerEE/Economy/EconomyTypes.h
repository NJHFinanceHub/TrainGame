// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// EconomyTypes.h - Economy, barter, and merchant type definitions

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SnowpiercerEE/SEETypes.h"
#include "EconomyTypes.generated.h"

// NOTE: Uses ESEETrainZone from SEETypes.h for zone references.
// NOTE: Uses ESEEFaction from SEETypes.h for faction references.

// Merchant archetype -- determines inventory pool and behavior
UENUM(BlueprintType)
enum class EMerchantType : uint8
{
	Scavenger		UMETA(DisplayName = "Scavenger"),
	WorkshopVendor	UMETA(DisplayName = "Workshop Vendor"),
	MedicTrader		UMETA(DisplayName = "Medic Trader"),
	KronoleDealer	UMETA(DisplayName = "Kronole Dealer"),
	BlackMarketeer	UMETA(DisplayName = "Black Marketeer"),
	ArmsDealer		UMETA(DisplayName = "Arms Dealer"),
	FirstClassPurveyor UMETA(DisplayName = "First Class Purveyor"),
	EngineSpecialist UMETA(DisplayName = "Engine Specialist")
};

// Result of a haggling round
UENUM(BlueprintType)
enum class EHaggleResult : uint8
{
	Accepted		UMETA(DisplayName = "Deal Accepted"),
	CounterOffer	UMETA(DisplayName = "Counter Offer"),
	Rejected		UMETA(DisplayName = "Offer Rejected"),
	WalkedAway		UMETA(DisplayName = "Player Walked Away"),
	MerchantAngry	UMETA(DisplayName = "Merchant Angry (deal off)")
};

// Degradation category for perishable items
UENUM(BlueprintType)
enum class EDegradationCategory : uint8
{
	Durable			UMETA(DisplayName = "Durable (no decay)"),
	PerishableFast	UMETA(DisplayName = "Perishable (fast)"),
	PerishableSlow	UMETA(DisplayName = "Perishable (slow)"),
	MoistureSensitive UMETA(DisplayName = "Moisture Sensitive"),
	Volatile		UMETA(DisplayName = "Volatile"),
	Expiring		UMETA(DisplayName = "Expiring")
};

// Zone-specific value multiplier entry
USTRUCT(BlueprintType)
struct FZoneValueEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEETrainZone Zone = ESEETrainZone::WorkingSpine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ValueMultiplier = 1.0f;
};

// Resource definition -- extends item system with economy data
USTRUCT(BlueprintType)
struct FResourceEconomyData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseTradeValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FZoneValueEntry> ZoneMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDegradationCategory DegradationCategory = EDegradationCategory::Durable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DegradationRatePerDay = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeStolen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bContraband = false;
};

// A single trade offer (one side of a barter)
USTRUCT(BlueprintType)
struct FTradeOffer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> Items;

	UPROPERTY(BlueprintReadOnly)
	float TotalValue = 0.0f;
};

// Complete trade proposal: what player gives vs. what player receives
USTRUCT(BlueprintType)
struct FTradeProposal
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTradeOffer PlayerOffer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTradeOffer MerchantOffer;

	UPROPERTY(BlueprintReadOnly)
	float HaggleModifier = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 HaggleRoundsUsed = 0;
};

// Merchant inventory slot -- what a merchant has for sale
USTRUCT(BlueprintType)
struct FMerchantInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredFavorTier = 0;
};

// Merchant configuration loaded from data tables
USTRUCT(BlueprintType)
struct FMerchantConfig : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MerchantID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMerchantType MerchantType = EMerchantType::Scavenger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEFaction Faction = ESEEFaction::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEETrainZone HomeZone = ESEETrainZone::WorkingSpine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuyPriceRatio = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SellPriceRatio = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InventoryRefreshDays = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHaggleRounds = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBuysStolenGoods = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoaming = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMerchantInventorySlot> BaseInventory;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, const FTradeProposal&, Trade, FName, MerchantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHaggleRound, int32, Round, EHaggleResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDegraded, FName, ItemID, float, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDestroyed, FName, ItemID);
