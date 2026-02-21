// SSEEFactionPanel.h - Faction reputation display
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/UI/SEEHUDTypes.h"

/**
 * SSEEFactionPanel
 *
 * Displays the player's reputation with each faction:
 * - Tailies, Third Class Union, Jackboots, Bureaucracy,
 *   First Class Elite, Order of the Engine, Kronole Network, The Thaw
 *
 * Each faction shows:
 * - Faction name and color
 * - Reputation bar (-100 to +100, centered at 0)
 * - Standing label (Hostile / Unfriendly / Neutral / Friendly / Allied)
 *
 * Reputation values are set externally via UpdateReputations().
 */
class SSEEFactionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEFactionPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Update the displayed reputation values
	void UpdateReputations(const TArray<FFactionReputation>& InReputations);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeReputationList();

	FText GetStandingText(float Reputation) const;
	FSlateColor GetStandingColor(float Reputation) const;

	TArray<FFactionReputation> Reputations;
};
