// MerchantComponent.h - NPC merchant with inventory, faction pricing, and favor tiers
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyTypes.h"
#include "MerchantComponent.generated.h"

/**
 * UMerchantComponent
 *
 * Attached to NPC merchant actors. Manages merchant inventory, pricing,
 * favor tiers, and inventory refresh cycles.
 *
 * Merchants belong to factions and their prices are affected by the player's
 * standing with that faction. Repeat customers build favor, unlocking
 * exclusive inventory tiers and better prices.
 */
UCLASS(ClassGroup=(Economy), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UMerchantComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMerchantComponent();

	// --- Configuration ---

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	FName GetMerchantID() const { return MerchantID; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	EMerchantType GetMerchantType() const { return MerchantType; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	EFaction GetFaction() const { return Faction; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	ETrainZone GetHomeZone() const { return HomeZone; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	bool DoesBuyStolenGoods() const { return bBuysStolenGoods; }

	// --- Inventory ---

	/** Get all items available for sale (filtered by player's favor tier). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	TArray<FMerchantInventorySlot> GetAvailableInventory(int32 PlayerFavorTier = 0) const;

	/** Check if the merchant has a specific item in stock. */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	bool HasItemInStock(FName ItemID, int32 Quantity = 1) const;

	/** Remove items from merchant stock (called after trade completes). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RemoveFromStock(FName ItemID, int32 Quantity);

	/** Add items to merchant stock (called when merchant buys from player). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void AddToStock(FName ItemID, int32 Quantity);

	/** Force refresh merchant inventory (timer-based or event-triggered). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RefreshInventory();

	// --- Pricing ---

	/** Get buy price ratio (what fraction of value the merchant pays). */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	float GetBuyPriceRatio() const { return BuyPriceRatio; }

	/** Get sell price ratio (markup over calculated value). */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	float GetSellPriceRatio() const { return SellPriceRatio; }

	/** Get max haggle rounds. */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetMaxHaggleRounds() const { return MaxHaggleRounds; }

	// --- Favor ---

	/** Get the player's current favor tier with this merchant (0-3). */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetFavorTier() const { return FavorTier; }

	/** Increment completed tasks count. Recalculates favor tier. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void CompleteTask();

	/** Get number of completed trades (affects repeat customer bonus). */
	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetCompletedTrades() const { return CompletedTrades; }

	/** Record a completed trade. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RecordTrade();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	FName MerchantID;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	EMerchantType MerchantType = EMerchantType::Scavenger;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	EFaction Faction = EFaction::None;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	ETrainZone HomeZone = ETrainZone::Spine;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	float BuyPriceRatio = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	float SellPriceRatio = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	float InventoryRefreshDays = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	int32 MaxHaggleRounds = 3;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	bool bBuysStolenGoods = false;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	TArray<FMerchantInventorySlot> BaseInventory;

	// Runtime state
	UPROPERTY(VisibleAnywhere, Category = "Merchant|Runtime")
	TArray<FMerchantInventorySlot> CurrentInventory;

	UPROPERTY(VisibleAnywhere, Category = "Merchant|Runtime")
	int32 FavorTier = 0;

	UPROPERTY(VisibleAnywhere, Category = "Merchant|Runtime")
	int32 CompletedTasks = 0;

	UPROPERTY(VisibleAnywhere, Category = "Merchant|Runtime")
	int32 CompletedTrades = 0;

	UPROPERTY(VisibleAnywhere, Category = "Merchant|Runtime")
	float LastRefreshGameTime = 0.0f;

private:
	void RecalculateFavorTier();
};
