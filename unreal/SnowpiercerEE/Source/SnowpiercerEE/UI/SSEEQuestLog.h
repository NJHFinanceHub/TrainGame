// SSEEQuestLog.h - Quest log display with filtering and tracking
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "../SEEQuestManager.h"

class USEEQuestManager;

/**
 * SSEEQuestLog
 *
 * Full-screen quest log showing:
 * - Tab filters: Active / Completed / Failed
 * - Quest list with main/side quest indicators
 * - Objective checklist per quest
 * - Tracking toggle (pin quest to HUD)
 * - Quest rewards preview
 *
 * Reads from USEEQuestManager (GameInstanceSubsystem).
 */
class SSEEQuestLog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEQuestLog) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetQuestManager(USEEQuestManager* InManager) { QuestManager = InManager; }

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeFilterTabs();
	TSharedRef<SWidget> MakeQuestList();
	TSharedRef<SWidget> MakeQuestDetail();

	TArray<FSEEQuest> GetFilteredQuests() const;
	FText GetQuestStateLabel(ESEEQuestState State) const;
	FLinearColor GetQuestStateColor(ESEEQuestState State) const;

	TWeakObjectPtr<USEEQuestManager> QuestManager;
	ESEEQuestState ActiveFilter = ESEEQuestState::Active;
	FName SelectedQuestID;
};
