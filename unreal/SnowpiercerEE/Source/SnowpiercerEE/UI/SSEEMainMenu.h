// SSEEMainMenu.h - Main menu screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Animation/CurveSequence.h"

struct FSlateBrush;

/**
 * SSEEMainMenu
 *
 * Title screen / main menu ("Eternal Engine" chrome):
 * - Near-black steel backdrop with a cold-blue gradient and faint metal wash
 * - Centered title block: amber overline, massive SNOWPIERCER masthead,
 *   letter-spaced ETERNAL ENGINE sub-line, riveted rule with a pulsing
 *   amber underline (FCurveSequence, no assets)
 * - Left-aligned vertical menu of industrial buttons
 * - Bottom strip: version left, tagline right
 *
 * Button actions are exposed via delegates set by the owning game mode.
 */

DECLARE_DELEGATE(FOnMenuAction);

class SSEEMainMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEMainMenu)
		: _bHasSaveGame(false)
		, _bShowCredits(true)
	{}
		SLATE_ARGUMENT(bool, bHasSaveGame)
		/** Hide the credits entry when no credits screen is wired up. */
		SLATE_ARGUMENT(bool, bShowCredits)
		SLATE_EVENT(FOnMenuAction, OnNewGame)
		SLATE_EVENT(FOnMenuAction, OnContinue)
		SLATE_EVENT(FOnMenuAction, OnSettings)
		SLATE_EVENT(FOnMenuAction, OnCredits)
		SLATE_EVENT(FOnMenuAction, OnQuit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** Swallow all keys: the world is paused behind the menu, and stray toggle
	 *  keys must not reach the player controller's screen bindings. */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedRef<SWidget> MakeBackdropLayers();
	TSharedRef<SWidget> MakeTitle();
	TSharedRef<SWidget> MakeRivetRule();
	TSharedRef<SWidget> MakeMenuButtons();
	TSharedRef<SWidget> MakeBottomStrip();

	/** Pulsing opacity for the amber underline beneath the masthead. */
	FSlateColor GetPulseUnderlineColor() const;

	bool bHasSaveGame = false;
	bool bShowCredits = true;
	FOnMenuAction OnNewGame;
	FOnMenuAction OnContinue;
	FOnMenuAction OnSettings;
	FOnMenuAction OnCredits;
	FOnMenuAction OnQuit;

	/** Owns the backdrop texture wash brush for the widget's lifetime. */
	TSharedPtr<FSlateBrush> BackdropTextureBrush;

	/** Idle pulse on the amber underline (loops; Slate-native, no assets). */
	FCurveSequence PulseSequence;
	FCurveHandle PulseUpCurve;
	FCurveHandle PulseDownCurve;
};
