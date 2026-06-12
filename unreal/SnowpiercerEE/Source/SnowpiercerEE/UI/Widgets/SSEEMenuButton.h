// SSEEMenuButton.h - Industrial menu button shared by all SEE menu screens.
// Left amber tick bar that expands on hover/focus, spaced-caps label,
// hover lightens the panel, press flashes amber, disabled renders at 40%.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Animation/CurveSequence.h"
#include "Styling/SlateColor.h"
#include "Types/SlateStructs.h"

class SButton;

/**
 * SSEEMenuButton
 *
 * Fixed-height industrial button used by the main menu, pause menu,
 * death screen and settings panel:
 * - Left accent "tick" bar (engine amber by default) that grows on hover
 * - Label in letter-spaced caps
 * - Hover: panel lightens + tick expands (animated via FCurveSequence)
 * - Pressed: amber flash
 * - Disabled: 40% opacity, no interaction
 * - Keyboard focus shows the same highlight as hover
 */
class SSEEMenuButton : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEMenuButton)
		: _Text()
		, _bEnabled(true)
		, _Width(320.0f)
		, _Height(50.0f)
		, _TickColor(FLinearColor::White) // resolved to EngineAmber in Construct
		, _bUseDefaultTickColor(true)
	{}
		SLATE_ARGUMENT(FText, Text)
		SLATE_ARGUMENT(bool, bEnabled)
		SLATE_ARGUMENT(float, Width)
		SLATE_ARGUMENT(float, Height)
		/** Accent color of the tick bar / press flash (default: engine amber). */
		SLATE_ARGUMENT(FLinearColor, TickColor)
		SLATE_ARGUMENT(bool, bUseDefaultTickColor)
		SLATE_EVENT(FSimpleDelegate, OnClicked)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	FReply HandleClicked();

	/** True while hovered or keyboard/gamepad focused (and enabled). */
	bool IsHighlighted() const;

	FSlateColor GetPanelColor() const;
	FSlateColor GetLabelColor() const;
	FSlateColor GetTickColor() const;
	FOptionalSize GetTickWidth() const;

	TSharedPtr<SButton> Button;
	FSimpleDelegate OnClicked;
	FLinearColor TickAccent;

	FCurveSequence HoverSequence;
	bool bHoverAnimTarget = false;
};
