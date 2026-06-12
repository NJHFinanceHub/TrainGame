// SSEEDeathScreen.h - Death / game over screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEEDeathScreen
 *
 * Death / game over overlay ("Eternal Engine" chrome):
 * - Deep red-black vignette layering
 * - Huge stencil header: "FROZEN" for cold deaths, "THE TAIL REMEMBERS"
 *   otherwise (picked from the death-cause string)
 * - Cause line, then industrial buttons (Reload Checkpoint / Quit to Menu)
 *
 * Displayed when the player character dies. Actions exposed via delegates.
 */

DECLARE_DELEGATE(FOnDeathScreenAction);

class SSEEDeathScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEDeathScreen) {}
		SLATE_EVENT(FOnDeathScreenAction, OnReloadCheckpoint)
		SLATE_EVENT(FOnDeathScreenAction, OnQuitToMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetDeathCause(const FText& InCause);

	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** Enter/Space reloads the checkpoint; every other key is swallowed so the
	 *  controller's screen-toggle bindings cannot replace the death screen. */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedRef<SWidget> MakeVignetteLayers();

	/** True when the cause string reads like death-by-cold (picks the header). */
	bool IsColdDeath() const;

	FText GetHeaderText() const;
	FSlateColor GetHeaderColor() const;
	FText GetCauseText() const;

	FOnDeathScreenAction OnReloadCheckpoint;
	FOnDeathScreenAction OnQuitToMenu;
	FText DeathCause;
};
