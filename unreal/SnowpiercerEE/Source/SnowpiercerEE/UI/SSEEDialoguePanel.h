// SSEEDialoguePanel.h - Dialogue display with branching choice system
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/UI/SEEHUDTypes.h"

// Slate-level delegate for choice selection (not DYNAMIC - pure Slate callback)
DECLARE_DELEGATE_OneParam(FOnDialogueChoiceSelectedSlate, FName /*ChoiceID*/);

/**
 * SSEEDialoguePanel
 *
 * Displays dialogue at the bottom of the screen with:
 * - Speaker name (highlighted)
 * - Dialogue text (typewriter reveal effect)
 * - Choice buttons when the line includes FDialogueChoice entries
 * - "Continue" prompt when no choices (dismiss on input)
 *
 * The panel occupies the bottom ~30% of the screen with a dark gradient backdrop.
 * Unavailable choices are shown grayed out with tooltip explaining why.
 */
class SSEEDialoguePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEDialoguePanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// Keyboard driving: 1-4 select choices, E/Enter/Space continue, Esc closes
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	// Set a new dialogue line to display
	void SetDialogueLine(const FDialogueLine& Line);

	// Callbacks for HUD to connect
	void SetOnChoiceSelected(FOnDialogueChoiceSelectedSlate InDelegate) { OnChoiceSelected = InDelegate; }
	void SetOnDismissed(FSimpleDelegate InDelegate) { OnDismissed = InDelegate; }

	// Fired when the player wants out of the whole conversation (Esc)
	void SetOnCloseRequested(FSimpleDelegate InDelegate) { OnCloseRequested = InDelegate; }

private:
	TSharedRef<SWidget> MakeSpeakerName();
	TSharedRef<SWidget> MakeDialogueText();
	TSharedRef<SWidget> MakeChoiceButtons();
	TSharedRef<SWidget> MakeContinuePrompt();

	// Get the currently visible portion of dialogue text (typewriter effect)
	FText GetRevealedText() const;

	// Is the typewriter reveal complete?
	bool IsFullyRevealed() const;

	// Select an available choice by display index (0-based); true if handled
	bool SelectChoiceByIndex(int32 ChoiceIndex);

	FDialogueLine CurrentLine;
	FOnDialogueChoiceSelectedSlate OnChoiceSelected;
	FSimpleDelegate OnDismissed;
	FSimpleDelegate OnCloseRequested;

	// Typewriter state
	int32 RevealedCharCount = 0;
	float RevealTimer = 0.0f;
	float CharsPerSecond = 40.0f;

	// Choice button container (rebuilt when line changes)
	TSharedPtr<SVerticalBox> ChoiceContainer;
};
