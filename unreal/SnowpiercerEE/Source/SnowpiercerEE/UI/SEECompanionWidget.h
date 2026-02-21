// SEECompanionWidget.h - Companion screen: loyalty, equipment, personal quest, party management
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TrainGame/Companions/CompanionTypes.h"
#include "SEECompanionWidget.generated.h"

class UCompanionComponent;
class UCompanionRosterSubsystem;
class UTextBlock;
class UProgressBar;

USTRUCT(BlueprintType)
struct FSEECompanionDisplayEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName CompanionID;

	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	FText Title;

	UPROPERTY(BlueprintReadOnly)
	ECompanionRole Role = ECompanionRole::Support;

	UPROPERTY(BlueprintReadOnly)
	ELoyaltyState LoyaltyState = ELoyaltyState::Neutral;

	UPROPERTY(BlueprintReadOnly)
	ECompanionBehavior Behavior = ECompanionBehavior::Defensive;

	UPROPERTY(BlueprintReadOnly)
	FCompanionStats Stats;

	UPROPERTY(BlueprintReadOnly)
	bool bInActiveParty = false;

	UPROPERTY(BlueprintReadOnly)
	bool bAlive = true;

	UPROPERTY(BlueprintReadOnly)
	bool bRomanceActive = false;

	UPROPERTY(BlueprintReadOnly)
	FCompanionQuestStep CurrentQuest;
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEECompanionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Companion")
	void RefreshCompanions();

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SelectCompanion(FName CompanionID);

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SetBehavior(FName CompanionID, ECompanionBehavior Behavior);

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void AddToParty(FName CompanionID);

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void RemoveFromParty(FName CompanionID);

	UFUNCTION(BlueprintPure, Category = "Companion")
	TArray<FSEECompanionDisplayEntry> GetRosterEntries() const { return RosterEntries; }

	UFUNCTION(BlueprintPure, Category = "Companion")
	FText GetLoyaltyDisplayName(ELoyaltyState State) const;

	UFUNCTION(BlueprintPure, Category = "Companion")
	FLinearColor GetLoyaltyColor(ELoyaltyState State) const;

	UFUNCTION(BlueprintPure, Category = "Companion")
	FText GetRoleDisplayName(ECompanionRole Role) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CompanionNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CompanionTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LoyaltyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RoleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StatsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PartyStatusText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Companion")
	void OnRosterRefreshed(const TArray<FSEECompanionDisplayEntry>& Entries);

	UFUNCTION(BlueprintImplementableEvent, Category = "Companion")
	void OnCompanionSelected(const FSEECompanionDisplayEntry& Entry);

private:
	UCompanionComponent* FindCompanion(FName CompanionID) const;
	UCompanionRosterSubsystem* GetRosterSubsystem() const;

	TArray<FSEECompanionDisplayEntry> RosterEntries;
	FName SelectedCompanionID;
};
