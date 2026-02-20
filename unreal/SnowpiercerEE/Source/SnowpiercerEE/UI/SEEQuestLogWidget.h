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
};
