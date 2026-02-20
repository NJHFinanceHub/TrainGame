#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEETypes.h"
#include "SEEFactionManager.generated.h"

UENUM(BlueprintType)
enum class ESEEFactionStanding : uint8
{
	Hostile		UMETA(DisplayName = "Hostile"),
	Unfriendly	UMETA(DisplayName = "Unfriendly"),
	Neutral		UMETA(DisplayName = "Neutral"),
	Friendly	UMETA(DisplayName = "Friendly"),
	Allied		UMETA(DisplayName = "Allied"),
	Devoted		UMETA(DisplayName = "Devoted")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionRepChanged, ESEEFaction, Faction, int32, NewRep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionStandingChanged, ESEEFaction, Faction, ESEEFactionStanding, NewStanding);

UCLASS()
class SNOWPIERCEREE_API USEEFactionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void ModifyReputation(ESEEFaction Faction, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetReputation(ESEEFaction Faction, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Faction")
	int32 GetReputation(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	ESEEFactionStanding GetStanding(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	bool IsHostile(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	bool IsFriendly(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	float GetPriceModifier(ESEEFaction Faction) const;

	UPROPERTY(BlueprintAssignable, Category = "Faction")
	FOnFactionRepChanged OnFactionRepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction")
	FOnFactionStandingChanged OnFactionStandingChanged;

private:
	TMap<ESEEFaction, int32> FactionReputations;

	void ApplyMutualExclusivity(ESEEFaction Faction, int32 Delta);
	ESEEFactionStanding RepToStanding(int32 Rep) const;
};
