// SSEESurvivalBars.h - Slate widget displaying survival stat meters
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowyEngine/Survival/SurvivalTypes.h"

class USurvivalComponent;
class UKronoleComponent;

/**
 * SSEESurvivalBars
 *
 * Displays the five core survival stats as vertical or horizontal bars:
 * Health (red), Stamina (yellow), Hunger (orange), Cold (cyan), Morale (purple).
 *
 * Positioned in the bottom-left of the viewport. Bars pulse when crossing
 * into critical thresholds. Also shows Kronole addiction/withdrawal indicators
 * when applicable.
 *
 * Polls component data each frame via Tick (components are polled, not delegate-bound,
 * for Slate simplicity - the survival stats change frequently enough that polling
 * is more efficient than delegate registration for UI).
 */
class SSEESurvivalBars : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEESurvivalBars)
		: _SurvivalComponent(nullptr)
		, _KronoleComponent(nullptr)
	{}
		SLATE_ARGUMENT(USurvivalComponent*, SurvivalComponent)
		SLATE_ARGUMENT(UKronoleComponent*, KronoleComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// Build a single stat bar row (label + colored bar)
	TSharedRef<SWidget> MakeStatBar(ESurvivalStatType StatType, const FText& Label, const FLinearColor& BarColor);

	// Build the Kronole status indicator
	TSharedRef<SWidget> MakeKronoleIndicator();

	// Get the fill percent for a stat (0-1)
	float GetStatPercent(ESurvivalStatType StatType) const;

	// Get bar opacity (pulses when critical)
	float GetBarOpacity(ESurvivalStatType StatType) const;

	// Get the kronole status text
	FText GetKronoleStatusText() const;
	FSlateColor GetKronoleStatusColor() const;

	TWeakObjectPtr<USurvivalComponent> SurvivalComp;
	TWeakObjectPtr<UKronoleComponent> KronoleComp;

	// Cached values for bar display
	TMap<ESurvivalStatType, float> CachedPercents;
	float PulseTimer = 0.0f;
};
