// SEEQuestLogWidget.h - Quest log: active/completed/failed, tracking toggle, objectives
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEQuestManager.h"
#include "SEEQuestLogWidget.generated.h"

class UTextBlock;
class UScrollBox;

UENUM(BlueprintType)
enum class ESEEQuestFilter : uint8
{
	Active		UMETA(DisplayName = "Active"),
	Completed	UMETA(DisplayName = "Completed"),
	Failed		UMETA(DisplayName = "Failed"),
	All			UMETA(DisplayName = "All")
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEQuestLogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RefreshQuests();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetFilter(ESEEQuestFilter NewFilter);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SelectQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ToggleTracking(FName QuestID);

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetFilteredQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName GetSelectedQuestID() const { return SelectedQuestID; }

	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsTracked(FName QuestID) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuestDescText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectivesText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FilterText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TrackingText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void OnQuestsRefreshed(const TArray<FSEEQuest>& Quests);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest")
	void OnQuestSelected(const FSEEQuest& Quest);

private:
	void UpdateQuestDetails();
	USEEQuestManager* GetQuestManager() const;

	ESEEQuestFilter CurrentFilter = ESEEQuestFilter::Active;
	FName SelectedQuestID;
};
