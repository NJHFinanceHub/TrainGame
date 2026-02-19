// BarterComponent.h - Barter trading system with zone-relative pricing and haggling
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EconomyTypes.h"
#include "BarterComponent.generated.h"

class UInventoryComponent;
class UMerchantComponent;

/**
 * UBarterComponent
 *
 * Attached to the player character. Handles trade negotiations with merchants,
 * calculates zone-relative item values, and executes barter transactions.
 *
 * Trade flow:
 * 1. Player interacts with merchant actor -> BeginTrade()
 * 2. Player builds offer -> ProposeTradeOffer()
 * 3. Merchant evaluates and responds (accept/counter/reject)
 * 4. Player can haggle (up to merchant's max rounds)
 * 5. Trade executes or fails -> EndTrade()
 */
UCLASS(ClassGroup=(Economy), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBarterComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBarterComponent();

	// --- Trade Session ---

	/** Begin a trade session with a merchant. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	bool BeginTrade(UMerchantComponent* Merchant);

	/** End the current trade session without completing a deal. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void EndTrade();

	/** Check if currently in a trade session. */
	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	bool IsTrading() const { return ActiveMerchant != nullptr; }

	// --- Value Calculation ---

	/** Calculate the trade value of an item in a specific zone. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	float CalculateItemValue(FName ItemID, ETrainZone Zone, float Condition = 1.0f) const;

	/** Calculate value with faction modifier applied. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	float CalculateItemValueWithFaction(FName ItemID, ETrainZone Zone, EFaction MerchantFaction, float Condition = 1.0f) const;

	/** Get the current zone the player is in (affects all value calculations). */
	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	ETrainZone GetCurrentZone() const { return CurrentZone; }

	/** Set the player's current zone (called by zone triggers). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void SetCurrentZone(ETrainZone Zone);

	// --- Trade Proposals ---

	/** Propose a trade: what the player offers vs. what they want from the merchant. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	EHaggleResult ProposeTradeOffer(const FTradeProposal& Proposal);

	/** Attempt to haggle for a better deal. Returns the merchant's response. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	EHaggleResult Haggle();

	/** Accept the current counter-offer from the merchant. Executes the trade. */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	bool AcceptCurrentOffer();

	/** Get the current trade proposal (with any haggle modifications). */
	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	const FTradeProposal& GetCurrentProposal() const { return CurrentProposal; }

	// --- Stolen Goods ---

	/** Check if an item instance is flagged as stolen. */
	UFUNCTION(BlueprintPure, Category = "Economy|Barter")
	bool IsItemStolen(const FGuid& InstanceID) const;

	/** Flag an item as stolen (called when looting NPCs). */
	UFUNCTION(BlueprintCallable, Category = "Economy|Barter")
	void MarkItemStolen(const FGuid& InstanceID);

	/** Clear stolen flag (time-based or quest-based). */
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
	ETrainZone CurrentZone = ETrainZone::Tail;

	UPROPERTY()
	UMerchantComponent* ActiveMerchant = nullptr;

	UPROPERTY()
	UInventoryComponent* PlayerInventory = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Economy|Runtime")
	FTradeProposal CurrentProposal;

	// Item instances flagged as stolen (InstanceID -> game-time when stolen)
	UPROPERTY(VisibleAnywhere, Category = "Economy|Runtime")
	TMap<FGuid, float> StolenItems;

	// Days before stolen flag auto-clears
	UPROPERTY(EditAnywhere, Category = "Economy|Config")
	float StolenClearanceDays = 7.0f;

private:
	float GetFactionPriceModifier(EFaction Faction) const;
	float CalculateHaggleModifier(int32 Round, float SocialStat) const;
	bool ValidateTradeItems(const FTradeOffer& Offer, UInventoryComponent* Source) const;
};
