// SSEEPauseMenu.h - In-game pause menu overlay
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE(FOnPauseMenuAction);

/**
 * SSEEPauseMenu
 *
 * Overlay pause menu shown when the game is paused:
 * - Resume
 * - Save Game
 * - Load Game
 * - Settings
 * - Quit to Main Menu
 *
 * Each button fires a delegate for the owning HUD/GameMode to handle.
 */
class SSEEPauseMenu : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEPauseMenu) {}
		SLATE_EVENT(FOnPauseMenuAction, OnResume)
		SLATE_EVENT(FOnPauseMenuAction, OnSave)
		SLATE_EVENT(FOnPauseMenuAction, OnLoad)
		SLATE_EVENT(FOnPauseMenuAction, OnSettings)
		SLATE_EVENT(FOnPauseMenuAction, OnQuitToMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, const FOnPauseMenuAction& Action);

	FOnPauseMenuAction OnResume;
	FOnPauseMenuAction OnSave;
	FOnPauseMenuAction OnLoad;
	FOnPauseMenuAction OnSettings;
	FOnPauseMenuAction OnQuitToMenu;
};
