// SSEECodexPanel.h - Lore codex and collectibles tracker
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEECodexPanel
 *
 * Full-screen codex/journal showing:
 * - Category tabs: Lore / Characters / Locations / Collectibles
 * - Entry list with read/unread indicators
 * - Entry detail text
 * - Collectibles progress tracker (found / total)
 *
 * Data is driven externally; entries are added via AddEntry().
 */

/** Category filter for codex entries */
enum class ECodexCategory : uint8
{
	Lore,
	Characters,
	Locations,
	Collectibles
};

struct FCodexEntry
{
	FName EntryID;
	FText Title;
	FText Content;
	ECodexCategory Category = ECodexCategory::Lore;
	bool bRead = false;
};

class SSEECodexPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECodexPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void AddEntry(const FCodexEntry& Entry);
	void MarkRead(FName EntryID);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeCategoryTabs();
	TSharedRef<SWidget> MakeEntryList();
	TSharedRef<SWidget> MakeEntryDetail();

	TArray<FCodexEntry> GetFilteredEntries() const;

	TArray<FCodexEntry> AllEntries;
	ECodexCategory ActiveCategory = ECodexCategory::Lore;
	FName SelectedEntryID;
};
