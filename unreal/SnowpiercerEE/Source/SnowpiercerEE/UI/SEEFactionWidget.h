// SEEFactionWidget.h - Faction screen: reputation bars, standing labels
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEETypes.h"
#include "SEEFactionManager.h"
#include "SEEFactionWidget.generated.h"

class UTextBlock;
class UProgressBar;

USTRUCT(BlueprintType)
struct FSEEFactionDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	ESEEFaction Faction = ESEEFaction::Neutral;

	UPROPERTY(BlueprintReadOnly)
	FText FactionName;

	UPROPERTY(BlueprintReadOnly)
	int32 Reputation = 0;

	UPROPERTY(BlueprintReadOnly)
	ESEEFactionStanding Standing = ESEEFactionStanding::Neutral;

	UPROPERTY(BlueprintReadOnly)
	FText StandingText;

	UPROPERTY(BlueprintReadOnly)
	FLinearColor StandingColor = FLinearColor::White;
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEFactionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Factions")
	void RefreshFactions();

	UFUNCTION(BlueprintPure, Category = "Factions")
	TArray<FSEEFactionDisplayEntry> GetFactionEntries() const { return FactionEntries; }

	UFUNCTION(BlueprintPure, Category = "Factions")
	FText GetFactionName(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Factions")
	FText GetStandingDisplayName(ESEEFactionStanding Standing) const;

	UFUNCTION(BlueprintPure, Category = "Factions")
	FLinearColor GetStandingColor(ESEEFactionStanding Standing) const;

	UFUNCTION(BlueprintPure, Category = "Factions")
	float GetReputationPercent(int32 Reputation) const;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Factions")
	void OnFactionsRefreshed(const TArray<FSEEFactionDisplayEntry>& Entries);

private:
	USEEFactionManager* GetFactionManager() const;
	TArray<FSEEFactionDisplayEntry> FactionEntries;
};
