// EconomyTypes.h - Economy, barter, and merchant type definitions
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SnowyEngine/Faction/FactionTypes.h"
#include "EconomyTypes.generated.h"

// Train zone for zone-relative trade value calculation
UENUM(BlueprintType)
enum class ETrainZone : uint8
{
	Tail			UMETA(DisplayName = "Tail Section"),
	ThirdClass		UMETA(DisplayName = "Third Class"),
	Spine			UMETA(DisplayName = "Spine"),
	SecondClass		UMETA(DisplayName = "Second Class"),
	FirstClass		UMETA(DisplayName = "First Class"),
	Engine			UMETA(DisplayName = "Engine Section")
};

// Merchant archetype — determines inventory pool and behavior
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
	ETrainZone Zone = ETrainZone::Spine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ValueMultiplier = 1.0f;
};

// Resource definition — extends item system with economy data
USTRUCT(BlueprintType)
struct FResourceEconomyData : public FTableRowBase
{
	GENERATED_BODY()

	// Links to FItemDefinition::ItemID
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// Base trade value (in abstract "value units" for barter comparison)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseTradeValue = 1.0f;

	// Zone-specific value multipliers
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FZoneValueEntry> ZoneMultipliers;

	// Degradation behavior
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDegradationCategory DegradationCategory = EDegradationCategory::Durable;

	// Degradation rate per in-game day (0 for durable items)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DegradationRatePerDay = 0.0f;

	// Whether this item can be flagged as stolen
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeStolen = true;

	// Whether legitimate merchants refuse to buy this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bContraband = false;
};

// A single trade offer (one side of a barter)
USTRUCT(BlueprintType)
struct FTradeOffer
{
	GENERATED_BODY()

	// Items offered (ItemID -> count)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> Items;

	// Total calculated value of this offer in the current zone
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

	// Haggle modifier applied (1.0 = no haggle effect)
	UPROPERTY(BlueprintReadOnly)
	float HaggleModifier = 1.0f;

	// Number of haggle rounds used
	UPROPERTY(BlueprintReadOnly)
	int32 HaggleRoundsUsed = 0;
};

// Merchant inventory slot — what a merchant has for sale
USTRUCT(BlueprintType)
struct FMerchantInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	// Merchant's asking price multiplier (1.0 = standard markup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PriceMultiplier = 1.0f;

	// Favor tier required to see this item (0 = always visible)
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
	EFaction Faction = EFaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrainZone HomeZone = ETrainZone::Spine;

	// Buy/sell spread: merchants buy at this fraction of value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BuyPriceRatio = 0.7f;

	// Sell markup over calculated value
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SellPriceRatio = 1.2f;

	// Days between inventory refresh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InventoryRefreshDays = 3.0f;

	// Maximum haggle rounds before merchant locks in
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHaggleRounds = 3;

	// Whether this merchant buys stolen goods
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBuysStolenGoods = false;

	// Whether this merchant moves locations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoaming = false;

	// Base inventory pool
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMerchantInventorySlot> BaseInventory;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTradeCompleted, const FTradeProposal&, Trade, FName, MerchantID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHaggleRound, int32, Round, EHaggleResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemDegraded, FName, ItemID, float, NewCondition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemDestroyed, FName, ItemID);
