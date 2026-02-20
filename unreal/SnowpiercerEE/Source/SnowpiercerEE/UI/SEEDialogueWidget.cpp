#include "SEEDialogueWidget.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void USEEDialogueWidget::ShowDialogue(const FText& SpeakerName, const FText& DialogueText, const TArray<FSEEDialogueOption>& Options)
{
	bDialogueActive = true;

	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(SpeakerName);
	}
	if (DialogueBodyText)
	{
		DialogueBodyText->SetText(DialogueText);
	}

	// Limit to max 4 options
	CurrentOptions.Empty();
	const int32 NumOptions = FMath::Min(Options.Num(), MaxDialogueOptions);
	for (int32 i = 0; i < NumOptions; ++i)
	{
		CurrentOptions.Add(Options[i]);
	}

	if (DialoguePanel)
	{
		DialoguePanel->SetVisibility(ESlateVisibility::Visible);
	}

	OnDialogueShown();
}

void USEEDialogueWidget::HideDialogue()
{
	bDialogueActive = false;
	CurrentOptions.Empty();

	if (DialoguePanel)
	{
		DialoguePanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnDialogueHidden();
}

void USEEDialogueWidget::SelectOption(int32 OptionIndex)
{
	if (!bDialogueActive || !CurrentOptions.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FSEEDialogueOption& Option = CurrentOptions[OptionIndex];

	// Don't allow selecting locked options
	if (Option.bLocked)
	{
		return;
	}

	OnOptionSelected.Broadcast(OptionIndex, Option.OptionID);
}
