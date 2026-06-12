// SSEEDialoguePanel.h - Cinematic dialogue presentation (Fallout 4 x Skyrim hybrid)
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Animation/CurveSequence.h"
#include "TrainGame/UI/SEEHUDTypes.h"

class SVerticalBox;
class SUniformGridPanel;

// Slate-level delegate for choice selection (not DYNAMIC - pure Slate callback)
DECLARE_DELEGATE_OneParam(FOnDialogueChoiceSelectedSlate, FName /*ChoiceID*/);

/**
 * SSEEDialoguePanel
 *
 * Cinematic conversation presentation:
 * - Subtle letterbox: top and bottom 8% darkened bands
 * - Speaker NAME PLATE left-aligned above the text box - spaced caps,
 *   faction-tinted name over a thin engine-amber underline; slides/fades in
 *   whenever the speaker changes
 * - Wide bottom dialogue box in riveted-steel chrome; bone text revealed by a
 *   typewriter (~52 chars/s; click or E completes instantly)
 * - Choices Fallout-style: up to 4 in a 2x2 grid bottom-center with amber
 *   number badges (1-4), hover/keyboard highlight, single-line truncation
 *   with a full-text tooltip, and a dim "[PER 5]" stat-gate prefix when the
 *   line data provides one; unavailable choices render dimmed with a reason
 * - "Continue >" affordance when the line has no choices
 * - The whole panel fades in on open and fades out before Esc-close fires
 *
 * Keyboard: 1-4 select choices, E/Enter/Space complete text then continue,
 * Esc requests conversation close. Public API unchanged - the UI subsystem
 * and the game HUD drive it exactly as before.
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

	// Click anywhere completes the typewriter reveal (Fallout-style skip)
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	// Set a new dialogue line to display
	void SetDialogueLine(const FDialogueLine& Line);

	// Callbacks for HUD to connect
	void SetOnChoiceSelected(FOnDialogueChoiceSelectedSlate InDelegate) { OnChoiceSelected = InDelegate; }
	void SetOnDismissed(FSimpleDelegate InDelegate) { OnDismissed = InDelegate; }

	// Fired when the player wants out of the whole conversation (Esc)
	void SetOnCloseRequested(FSimpleDelegate InDelegate) { OnCloseRequested = InDelegate; }

private:
	TSharedRef<SWidget> MakeLetterboxBand();
	TSharedRef<SWidget> MakeNamePlate();
	TSharedRef<SWidget> MakeDialogueBox();
	TSharedRef<SWidget> MakeContinuePrompt();
	TSharedRef<SWidget> MakeChoiceTile(const FDialogueChoice& Choice, int32 DisplayIndex);

	// Get the currently visible portion of dialogue text (typewriter effect)
	FText GetRevealedText() const { return CachedRevealedText; }

	// Is the typewriter reveal complete?
	bool IsFullyRevealed() const;

	// Jump the typewriter to the end of the line
	void CompleteReveal();

	// Select an available choice by display index (0-based); true if handled
	bool SelectChoiceByIndex(int32 ChoiceIndex);

	// Start the fade-out; fires OnCloseRequested when it finishes (Tick)
	void RequestClose();

	// Faction tint for the current speaker name (computed once per line)
	FLinearColor GetSpeakerColor() const;

	// "[PER 5]"-style prefix for a stat-gated choice; empty when ungated
	static FText MakeGatePrefix(const FDialogueChoice& Choice);

	// The subsystem prepends "1. " numbering to choice text; the number badge
	// replaces it, so strip any leading "<digits>. " once at line-set time.
	static FText StripLeadingNumber(const FText& ChoiceText);

	FDialogueLine CurrentLine;
	FOnDialogueChoiceSelectedSlate OnChoiceSelected;
	FSimpleDelegate OnDismissed;
	FSimpleDelegate OnCloseRequested;

	// Typewriter state
	int32 RevealedCharCount = 0;
	float RevealTimer = 0.0f;
	float CharsPerSecond = 52.0f;
	FText CachedRevealedText;   // rebuilt only when RevealedCharCount changes
	FString CachedFullText;     // CurrentLine.DialogueText, cached once per line
	FLinearColor CachedSpeakerColor = FLinearColor::White; // computed once per line

	// Choice grid (rebuilt when line changes)
	TSharedPtr<SUniformGridPanel> ChoiceGrid;

	// Animations
	FCurveSequence FadeSequence;      // panel fade in/out
	FCurveSequence NamePlateSequence; // name plate slide/fade on speaker change
	bool bClosing = false;            // fade-out in flight; fire close at end
	FString LastSpeakerString;        // detect speaker changes
};
