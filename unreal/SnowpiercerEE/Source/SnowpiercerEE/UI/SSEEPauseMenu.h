// SSEEPauseMenu.h - In-game pause menu overlay
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEEPauseMenu
 *
 * Pause menu overlay ("Eternal Engine" chrome - riveted panel frame) with:
 * - Resume button
 * - Save Game button
 * - Load Game button
 * - Settings button
 * - Quit to Main Menu button
 * - Quit to Desktop button
 *
 * Displayed when the game is paused. Actions are exposed via delegates.
 * Handles Escape itself (the game is paused, so the controller's Escape
 * binding does not fire) and routes it to OnResume.
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
		SLATE_EVENT(FOnPauseMenuAction, OnQuitToDesktop)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedRef<SWidget> MakeMenuButton(const FText& Label, FOnPauseMenuAction Action, bool bDanger = false);

	FOnPauseMenuAction OnResume;
	FOnPauseMenuAction OnSave;
	FOnPauseMenuAction OnLoad;
	FOnPauseMenuAction OnSettings;
	FOnPauseMenuAction OnQuitToMenu;
	FOnPauseMenuAction OnQuitToDesktop;
};
