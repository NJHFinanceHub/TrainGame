// SSEEQuestLog.h - Quest log with active/completed/failed tabs and tracking
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USEEQuestManager;

/**
 * SSEEQuestLog
 *
 * Full-screen quest journal showing:
 * - Tab filters: Active / Completed / Failed
 * - Quest list with main/side indicators
 * - Quest detail panel with objectives and progress
 * - Track/untrack toggle for active quests
 * - Reward preview (XP, items, faction rep)
 *
 * Reads from USEEQuestManager (GameInstance subsystem).
 * Toggled via ASEEGameHUD::ToggleQuestLog().
 */
class SSEEQuestLog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEQuestLog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetQuestManager(USEEQuestManager* InManager);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeFilterTabs();
	TSharedRef<SWidget> MakeQuestList();
	TSharedRef<SWidget> MakeDetailPanel();

	TWeakObjectPtr<USEEQuestManager> QuestManager;

	// 0 = Active, 1 = Completed, 2 = Failed
	int32 ActiveFilter = 0;
};
