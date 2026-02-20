// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// BarterComponent.h - Barter trading system with zone-relative pricing and haggling

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyTypes.h"
#include "BarterComponent.generated.h"

class USEEInventoryComponent;
class UMerchantComponent;

/**
 * UBarterComponent
 *
 * Attached to the player character. Handles trade negotiations with merchants,
 * calculates zone-relative item values, and executes barter transactions.
 */
UCLASS(ClassGroup=(Economy), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UBarterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBarterComponent();

	// --- Trade Session ---

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	bool BeginTrade(UMerchantComponent* Merchant);

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void EndTrade();

	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	bool IsTrading() const { return ActiveMerchant != nullptr; }

	// --- Value Calculation ---

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	float CalculateItemValue(FName ItemID, ESEETrainZone Zone, float Condition = 1.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	float CalculateItemValueWithFaction(FName ItemID, ESEETrainZone Zone, ESEEFaction MerchantFaction, float Condition = 1.0f) const;

	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	ESEETrainZone GetCurrentZone() const { return CurrentZone; }

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void SetCurrentZone(ESEETrainZone Zone);

	// --- Trade Proposals ---

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	EHaggleResult ProposeTradeOffer(const FTradeProposal& Proposal);

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	EHaggleResult Haggle();

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	bool AcceptCurrentOffer();

	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	const FTradeProposal& GetCurrentProposal() const { return CurrentProposal; }

	// --- Stolen Goods ---

	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	bool IsItemStolen(const FGuid& InstanceID) const;

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void MarkItemStolen(const FGuid& InstanceID);

	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void ClearStolenFlag(const FGuid& InstanceID);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Economy|Barter")
	FOnTradeCompleted OnTradeCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Economy|Barter")
	FOnHaggleRound OnHaggleRound;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Economy|Config")
	UDataTable* ResourceEconomyDataTable = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Economy|Runtime")
	ESEETrainZone CurrentZone = ESEETrainZone::Tail;

	UPROPERTY()
	UMerchantComponent* ActiveMerchant = nullptr;

	UPROPERTY()
	USEEInventoryComponent* PlayerInventory = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Economy|Runtime")
	FTradeProposal CurrentProposal;

	UPROPERTY(VisibleAnywhere, Category = "Economy|Runtime")
	TMap<FGuid, float> StolenItems;

	UPROPERTY(EditAnywhere, Category = "Economy|Config")
	float StolenClearanceDays = 7.0f;

private:
	float GetFactionPriceModifier(ESEEFaction Faction) const;
	float CalculateHaggleModifier(int32 Round, float SocialStat) const;
	bool ValidateTradeItems(const FTradeOffer& Offer, USEEInventoryComponent* Source) const;
};
