// SSEECodexPanel.h - Codex/journal for lore and collectibles tracking
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowpiercerEE/Exploration/CollectibleTypes.h"

class UCollectibleJournalSubsystem;

/**
 * SSEECodexPanel
 *
 * Full-screen codex/journal showing:
 * - Category filter tabs (All, Train Logs, Audio, Artifacts, Blueprints, Intel, Manifest)
 * - Collection completion percentages per category
 * - Overall completion with exploration title
 * - Unviewed indicator for new entries
 * - Web of Power faction intel progress
 *
 * Reads from UCollectibleJournalSubsystem (GameInstance subsystem).
 * Toggled via ASEEGameHUD::ToggleCodex().
 */
class SSEECodexPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECodexPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetJournalSubsystem(UCollectibleJournalSubsystem* InSubsystem);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeCategoryTabs();
	TSharedRef<SWidget> MakeCollectibleList();
	TSharedRef<SWidget> MakeProgressSidebar();

	FText GetTypeName(ECollectibleType Type) const;

	TWeakObjectPtr<UCollectibleJournalSubsystem> JournalSubsystem;

	// 255 = All, otherwise cast to ECollectibleType
	uint8 ActiveCategory = 255;
};
