// SSEEDeathScreen.h - Death/game over screen
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

DECLARE_DELEGATE(FOnDeathScreenAction);

/**
 * SSEEDeathScreen
 *
 * Full-screen death overlay shown when the player dies:
 * - Death message
 * - Reload Last Checkpoint button
 * - Quit to Main Menu button
 *
 * Displayed by ASEEGameHUD::ShowDeathScreen(), hidden on action.
 */
class SSEEDeathScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEDeathScreen) {}
		SLATE_EVENT(FOnDeathScreenAction, OnReloadCheckpoint)
		SLATE_EVENT(FOnDeathScreenAction, OnQuitToMenu)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetDeathMessage(const FText& Message);

private:
	FText DeathMessage;
	FOnDeathScreenAction OnReloadCheckpoint;
	FOnDeathScreenAction OnQuitToMenu;
};
