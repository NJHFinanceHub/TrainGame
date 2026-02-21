// SEEDialogueWidget.cpp
#include "SEEDialogueWidget.h"
#include "SEEDialogueManager.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"

void USEEDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TimerBar)
	{
		TimerBar->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ContinuePromptText)
	{
		ContinuePromptText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USEEDialogueWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TimedResponseDuration > 0.0f && bWaitingForChoice)
	{
		TimedResponseTimer -= InDeltaTime;

		if (TimerBar)
		{
			float Percent = FMath::Clamp(TimedResponseTimer / TimedResponseDuration, 0.0f, 1.0f);
			TimerBar->SetPercent(Percent);

			FLinearColor Color = Percent > 0.3f
				? FLinearColor(0.3f, 0.7f, 1.0f)
				: FLinearColor(1.0f, 0.3f, 0.1f);
			TimerBar->SetFillColorAndOpacity(Color);
		}

		if (TimedResponseTimer <= 0.0f)
		{
			TimedResponseDuration = 0.0f;
			bWaitingForChoice = false;
			OnTimerExpired();

			// Auto-advance when timer expires (defaults to first choice or next node)
			USEEDialogueManager* DM = GetDialogueManager();
			if (DM)
			{
				DM->AdvanceDialogue();
			}
		}
	}
}

USEEDialogueManager* USEEDialogueWidget::GetDialogueManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USEEDialogueManager>() : nullptr;
}

void USEEDialogueWidget::ShowDialogueNode(const FSEEDialogueNode& Node)
{
	bWaitingForChoice = false;
	TimedResponseDuration = 0.0f;

	if (SpeakerNameText)
	{
		SpeakerNameText->SetText(Node.SpeakerName);
	}

	if (DialogueText)
	{
		DialogueText->SetText(Node.DialogueText);
	}

	// Handle node types
	switch (Node.NodeType)
	{
	case ESEEDialogueNodeType::NPCLine:
		if (ContinuePromptText)
		{
			ContinuePromptText->SetText(NSLOCTEXT("Dlg", "Continue", "[Space] Continue"));
			ContinuePromptText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		if (TimerBar) TimerBar->SetVisibility(ESlateVisibility::Collapsed);
		break;

	case ESEEDialogueNodeType::PlayerChoice:
	{
		if (ContinuePromptText) ContinuePromptText->SetVisibility(ESlateVisibility::Collapsed);

		USEEDialogueManager* DM = GetDialogueManager();
		if (DM)
		{
			ShowChoices(DM->GetAvailableChoices());
		}

		// Timed response
		if (Node.TimedResponseDuration > 0.0f)
		{
			TimedResponseDuration = Node.TimedResponseDuration;
			TimedResponseTimer = TimedResponseDuration;
			if (TimerBar)
			{
				TimerBar->SetPercent(1.0f);
				TimerBar->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
		else
		{
			if (TimerBar) TimerBar->SetVisibility(ESlateVisibility::Collapsed);
		}
		break;
	}

	case ESEEDialogueNodeType::SkillCheck:
		if (SkillCheckText)
		{
			SkillCheckText->SetVisibility(ESlateVisibility::HitTestInvisible);
			SkillCheckText->SetText(NSLOCTEXT("Dlg", "SkillCheck", "[Skill Check]"));
		}
		if (ContinuePromptText)
		{
			ContinuePromptText->SetText(NSLOCTEXT("Dlg", "Continue", "[Space] Continue"));
			ContinuePromptText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		break;

	default:
		if (ContinuePromptText) ContinuePromptText->SetVisibility(ESlateVisibility::Collapsed);
		if (TimerBar) TimerBar->SetVisibility(ESlateVisibility::Collapsed);
		break;
	}

	OnDialogueNodeShown(Node);
}

void USEEDialogueWidget::ShowChoices(const TArray<FSEEDialogueChoice>& Choices)
{
	bWaitingForChoice = true;
	OnChoicesShown(Choices);
}

void USEEDialogueWidget::SelectChoice(int32 ChoiceIndex)
{
	bWaitingForChoice = false;
	TimedResponseDuration = 0.0f;

	if (TimerBar) TimerBar->SetVisibility(ESlateVisibility::Collapsed);

	USEEDialogueManager* DM = GetDialogueManager();
	if (DM)
	{
		DM->SelectChoice(ChoiceIndex);
	}

	OnChoiceSelected(ChoiceIndex);
}

void USEEDialogueWidget::AdvanceDialogue()
{
	if (bWaitingForChoice) return;

	USEEDialogueManager* DM = GetDialogueManager();
	if (DM)
	{
		DM->AdvanceDialogue();
	}
}

void USEEDialogueWidget::EndDialogue()
{
	bWaitingForChoice = false;
	TimedResponseDuration = 0.0f;

	USEEDialogueManager* DM = GetDialogueManager();
	if (DM)
	{
		DM->EndConversation();
	}

	OnDialogueEnded();
}

void USEEDialogueWidget::SetSpeakerPortrait(UTexture2D* Portrait)
{
	if (!SpeakerPortraitImage) return;

	if (Portrait)
	{
		SpeakerPortraitImage->SetBrushFromTexture(Portrait);
		SpeakerPortraitImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		SpeakerPortraitImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
