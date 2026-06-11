// SSEESettingsPanel.h - Minimal settings overlay (mouse sensitivity, window mode)
#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericWindow.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class APlayerController;

/**
 * SSEESettingsPanel
 *
 * Overlay panel shared by the main menu and pause menu:
 * - Mouse sensitivity slider (writes through to the local player's UPlayerInput)
 * - Window mode selection (Fullscreen / Borderless / Windowed via UGameUserSettings)
 * - Back button (and Esc) closes the overlay via the OnClose delegate
 */
class SSEESettingsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEESettingsPanel)
		: _PlayerController(nullptr)
	{}
		SLATE_ARGUMENT(APlayerController*, PlayerController)
		SLATE_EVENT(FSimpleDelegate, OnClose)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

private:
	TSharedRef<SWidget> MakeSensitivityRow();
	TSharedRef<SWidget> MakeWindowModeRow();
	TSharedRef<SWidget> MakeWindowModeButton(const FText& Label, EWindowMode::Type Mode);

	/** Write the new sensitivity to the local player's input. */
	void ApplySensitivity(float NewValue);

	/** Read the current MouseX axis sensitivity from player input (fallback 0.07). */
	float ReadCurrentSensitivity() const;

	TWeakObjectPtr<APlayerController> OwnerPC;
	FSimpleDelegate OnClose;
	float SensitivityValue = 0.07f;
};
