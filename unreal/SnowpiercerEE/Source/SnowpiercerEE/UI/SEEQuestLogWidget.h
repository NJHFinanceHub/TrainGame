#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "../SEEQuestManager.h"

/**
 * SEEQuestLogWidget
 *
 * Full-screen quest log showing:
 * - Active quests (main + side) with objectives and progress
 * - Completed quests with rewards earned
 * - Quest tracking toggle (pins quest to HUD)
 * - Categories: Main Quest | Side Quests | Faction Quests | Companion Quests
 *
 * Reads from USEEQuestManager subsystem.
 */
class SSEEQuestLogWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEQuestLogWidget)
		: _QuestManager(nullptr)
	{}
		SLATE_ARGUMENT(USEEQuestManager*, QuestManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Refresh the quest display from the quest manager. */
	void RefreshQuests();

	/** Set the currently selected category filter. */
	void SetCategory(const FString& Category);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeCategoryTabs();
	TSharedRef<SWidget> MakeQuestList();
	TSharedRef<SWidget> MakeQuestEntry(const FSEEQuest& Quest);
	TSharedRef<SWidget> MakeObjectiveEntry(const FSEEQuestObjective& Objective);
	TSharedRef<SWidget> MakeQuestDetail();

	FText GetQuestStateText(ESEEQuestState State) const;
	FSlateColor GetQuestStateColor(ESEEQuestState State) const;
	FText GetObjectiveTypeIcon(ESEEObjectiveType Type) const;

	TWeakObjectPtr<USEEQuestManager> QuestManager;
	TArray<FSEEQuest> CachedQuests;
	FString ActiveCategory = TEXT("All");
	FName SelectedQuestID = NAME_None;

	static constexpr float PanelWidth = 800.0f;
	static constexpr float PanelHeight = 600.0f;
};

/**
 * SSEEQuestTrackerWidget
 *
 * Compact HUD overlay showing the currently tracked quest:
 * - Quest name
 * - Current objective with progress
 * - Waypoint direction indicator
 *
 * Displayed in the corner of the gameplay HUD.
 */
class SSEEQuestTrackerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEQuestTrackerWidget)
		: _QuestManager(nullptr)
	{}
		SLATE_ARGUMENT(USEEQuestManager*, QuestManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	/** Update the tracked quest display. Called per-tick or on quest events. */
	void UpdateTrackedQuest();

private:
	TWeakObjectPtr<USEEQuestManager> QuestManager;
	FName TrackedQuestID = NAME_None;
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
