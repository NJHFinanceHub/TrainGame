// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
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
 */
UCLASS(ClassGroup=(Economy), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UMerchantComponent : public UActorComponent
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
	ESEEFaction GetFaction() const { return Faction; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	ESEETrainZone GetHomeZone() const { return HomeZone; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	bool DoesBuyStolenGoods() const { return bBuysStolenGoods; }

	// --- Inventory ---

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	TArray<FMerchantInventorySlot> GetAvailableInventory(int32 PlayerFavorTier = 0) const;

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	bool HasItemInStock(FName ItemID, int32 Quantity = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RemoveFromStock(FName ItemID, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void AddToStock(FName ItemID, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RefreshInventory();

	// --- Pricing ---

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	float GetBuyPriceRatio() const { return BuyPriceRatio; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	float GetSellPriceRatio() const { return SellPriceRatio; }

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetMaxHaggleRounds() const { return MaxHaggleRounds; }

	// --- Favor ---

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetFavorTier() const { return FavorTier; }

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void CompleteTask();

	UFUNCTION(BlueprintPure, Category = "Economy|Merchant")
	int32 GetCompletedTrades() const { return CompletedTrades; }

	UFUNCTION(BlueprintCallable, Category = "Economy|Merchant")
	void RecordTrade();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	FName MerchantID;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	EMerchantType MerchantType = EMerchantType::Scavenger;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	ESEEFaction Faction = ESEEFaction::Neutral;

	UPROPERTY(EditAnywhere, Category = "Merchant|Config")
	ESEETrainZone HomeZone = ESEETrainZone::WorkingSpine;

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
