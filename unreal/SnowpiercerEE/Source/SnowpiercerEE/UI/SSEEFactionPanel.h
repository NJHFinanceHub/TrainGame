// SSEEFactionPanel.h - Fallout-style faction standing screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/UI/SEEHUDTypes.h"
#include "SnowpiercerEE/SEETypes.h"
#include "SnowpiercerEE/Factions/SEEFactionTypes.h"

class USEEFactionManager;

/**
 * SSEEFactionPanel
 *
 * The faction standing screen. One row card per faction:
 *  - faction name (faction color) + standing tier name (tier color) + raw rep
 *  - a lore one-liner
 *  - a segmented standing bar across the full -100..+100 axis with tier
 *    boundary ticks and a marker at the current value
 *  - a current-effects line driven by the manager's consequence queries
 *    (merchant prices, dialogue refusal, kill-on-sight)
 *
 * A detail pane on the right shows the selected faction's longer description
 * plus what raises and lowers standing. Browse with Up/Down (or W/S); click
 * a row to select it.
 *
 * Data source: the panel resolves USEEFactionManager from the running game
 * world and reads live values every frame (Slate attribute polling), so it
 * stays current while open. UpdateReputations() remains as the legacy push
 * API and acts as a fallback when no manager can be found.
 */
class SSEEFactionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEFactionPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Legacy push API (SEEUISubsystem / SEEGameHUD). Used as fallback data
	// only when the faction manager cannot be resolved from the engine.
	void UpdateReputations(const TArray<FFactionReputation>& InReputations);

	// Keyboard browsing
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	/** Static per-faction display data (enum, colors, lore). Defined in the cpp. */
	struct FFactionDisplayInfo;
	static TArrayView<const FFactionDisplayInfo> GetFactionInfo();

	USEEFactionManager* GetFactionManager() const;

	/** Live reputation for the Nth faction card (manager first, pushed values as fallback). */
	int32 GetRep(int32 InfoIndex) const;
	ESEEFactionStanding GetStandingAt(int32 InfoIndex) const;

	/** "Prices +25% • Civilians refuse to speak" style consequence summary. */
	FText GetEffectsText(int32 InfoIndex) const;

	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeFactionList();
	TSharedRef<SWidget> MakeFactionRow(int32 InfoIndex);
	TSharedRef<SWidget> MakeDetailPane();

	void SelectIndex(int32 NewIndex);

	int32 SelectedIndex = 0;

	/** Legacy pushed values, index-aligned with GetFactionInfo(). */
	TArray<FFactionReputation> Reputations;

	mutable TWeakObjectPtr<USEEFactionManager> CachedManager;
};
