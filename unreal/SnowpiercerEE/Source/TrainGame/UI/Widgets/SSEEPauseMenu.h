// SSEEPauseMenu.h - In-game pause menu overlay
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEEPauseMenu
 *
 * Pause menu overlay with:
 * - Resume button
 * - Save Game button
 * - Load Game button
 * - Settings button
 * - Quit to Main Menu button
 *
 * Displayed when the game is paused. Actions are exposed via delegates.
 */

DECLARE_DELEGATE(FOnPauseMenuAction);

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
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, FOnPauseMenuAction Action);

	FOnPauseMenuAction OnResume;
	FOnPauseMenuAction OnSave;
	FOnPauseMenuAction OnLoad;
	FOnPauseMenuAction OnSettings;
	FOnPauseMenuAction OnQuitToMenu;
};
