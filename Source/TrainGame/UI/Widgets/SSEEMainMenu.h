// SSEEMainMenu.h - Main menu: new game, continue, settings, credits
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE(FOnMainMenuAction);

/**
 * SSEEMainMenu
 *
 * Full-screen main menu displayed at game start:
 * - Game title and atmospheric background
 * - New Game button
 * - Continue button (grayed if no save exists)
 * - Settings button
 * - Credits button
 * - Quit button
 *
 * Each button fires a delegate that the owning HUD or GameMode handles.
 */
class SSEEMainMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEMainMenu)
		: _bHasSaveGame(false)
	{}
		SLATE_ARGUMENT(bool, bHasSaveGame)
		SLATE_EVENT(FOnMainMenuAction, OnNewGame)
		SLATE_EVENT(FOnMainMenuAction, OnContinue)
		SLATE_EVENT(FOnMainMenuAction, OnSettings)
		SLATE_EVENT(FOnMainMenuAction, OnCredits)
		SLATE_EVENT(FOnMainMenuAction, OnQuit)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetHasSaveGame(bool bHasSave);

private:
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, const FOnMainMenuAction& Action, bool bEnabled = true);

	bool bHasSaveGame = false;
	FOnMainMenuAction OnNewGame;
	FOnMainMenuAction OnContinue;
	FOnMainMenuAction OnSettings;
	FOnMainMenuAction OnCredits;
	FOnMainMenuAction OnQuit;
};
