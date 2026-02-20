// SSEEDeathScreen.h - Death / game over screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEEDeathScreen
 *
 * Death / game over overlay showing:
 * - "YOU DIED" text with cause of death
 * - Reload Checkpoint button
 * - Quit to Main Menu button
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

private:
	FOnDeathScreenAction OnReloadCheckpoint;
	FOnDeathScreenAction OnQuitToMenu;
	FText DeathCause;
};
