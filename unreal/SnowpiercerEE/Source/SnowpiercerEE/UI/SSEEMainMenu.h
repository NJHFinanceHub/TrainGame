// SSEEMainMenu.h - Main menu screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEEMainMenu
 *
 * Title screen / main menu:
 * - Game title and version
 * - New Game button
 * - Continue button (greyed out if no save)
 * - Settings button
 * - Credits button
 * - Quit button
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
	TSharedRef<SWidget> MakeTitle();
	TSharedRef<SWidget> MakeMenuButtons();
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, FOnMenuAction Action, bool bEnabled = true);

	bool bHasSaveGame = false;
	bool bShowCredits = true;
	FOnMenuAction OnNewGame;
	FOnMenuAction OnContinue;
	FOnMenuAction OnSettings;
	FOnMenuAction OnCredits;
	FOnMenuAction OnQuit;
};
