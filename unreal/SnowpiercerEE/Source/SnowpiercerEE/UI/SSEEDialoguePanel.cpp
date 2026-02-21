// SSEEDialoguePanel.cpp - Dialogue panel implementation
#include "SSEEDialoguePanel.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEDialoguePanel::Construct(const FArguments& InArgs)
{
	ChoiceContainer = SNew(SVerticalBox);

	ChildSlot
	[
		// Anchor to bottom of screen
		SNew(SBox)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Bottom)
		[
			SNew(SBox)
			.HeightOverride(220.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.04f, 0.92f))
				.Padding(FMargin(40, 16))
				[
					SNew(SVerticalBox)

					// Speaker name
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 6)
					[
						MakeSpeakerName()
					]

					// Dialogue text
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						MakeDialogueText()
					]

					// Choices OR continue prompt
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 8, 0, 0)
					[
						SNew(SOverlay)

						// Choice buttons (visible when choices exist and text fully revealed)
						+ SOverlay::Slot()
						[
							ChoiceContainer.ToSharedRef()
						]

						// Continue prompt (visible when no choices)
						+ SOverlay::Slot()
						[
							MakeContinuePrompt()
						]
					]
				]
			]
		]
	];
}

void SSEEDialoguePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Advance typewriter reveal
	if (!IsFullyRevealed())
	{
		RevealTimer += InDeltaTime;
		int32 TargetChars = FMath::FloorToInt(RevealTimer * CharsPerSecond);
		RevealedCharCount = FMath::Min(TargetChars, CurrentLine.DialogueText.ToString().Len());
	}
}

void SSEEDialoguePanel::SetDialogueLine(const FDialogueLine& Line)
{
	CurrentLine = Line;
	RevealedCharCount = 0;
	RevealTimer = 0.0f;

	// Rebuild choice buttons
	ChoiceContainer->ClearChildren();

	if (Line.Choices.Num() > 0)
	{
		for (const FDialogueChoice& Choice : Line.Choices)
		{
			FName ChoiceID = Choice.ChoiceID;
			bool bAvailable = Choice.bIsAvailable;
			FText ChoiceText = Choice.ChoiceText;
			FText UnavailableReason = Choice.UnavailableReason;

			ChoiceContainer->AddSlot()
				.AutoHeight()
				.Padding(0, 2)
				[
					SNew(SButton)
					.IsEnabled(bAvailable)
					.OnClicked_Lambda([this, ChoiceID]()
					{
						OnChoiceSelected.ExecuteIfBound(ChoiceID);
						return FReply::Handled();
					})
					.ButtonColorAndOpacity_Lambda([bAvailable]()
					{
						return bAvailable
							? FLinearColor(0.15f, 0.12f, 0.08f)
							: FLinearColor(0.08f, 0.08f, 0.08f);
					})
					[
						SNew(SHorizontalBox)

						// Choice marker
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(0, 0, 8, 0)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT(">")))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
							.ColorAndOpacity(FSlateColor(bAvailable
								? FLinearColor(0.9f, 0.8f, 0.4f)
								: FLinearColor(0.4f, 0.4f, 0.4f)))
						]

						// Choice text
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ChoiceText)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(FSlateColor(bAvailable
								? FLinearColor(0.85f, 0.85f, 0.85f)
								: FLinearColor(0.45f, 0.45f, 0.45f)))
							.AutoWrapText(true)
						]

						// Unavailable reason (if applicable)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8, 0, 0, 0)
						[
							SNew(STextBlock)
							.Text(UnavailableReason)
							.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.3f, 0.2f)))
							.Visibility(UnavailableReason.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible)
						]
					]
				];
		}
	}
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeSpeakerName()
{
	return SNew(STextBlock)
		.Text_Lambda([this]() { return CurrentLine.SpeakerName; })
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.75f, 0.3f)));
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeDialogueText()
{
	return SNew(STextBlock)
		.Text_Lambda([this]() { return GetRevealedText(); })
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
		.AutoWrapText(true);
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeContinuePrompt()
{
	return SNew(SButton)
		.Visibility_Lambda([this]()
		{
			// Show "Continue" only when text is fully revealed and there are no choices
			if (CurrentLine.Choices.Num() > 0) return EVisibility::Collapsed;
			if (!IsFullyRevealed()) return EVisibility::Collapsed;
			return EVisibility::Visible;
		})
		.OnClicked_Lambda([this]()
		{
			OnDismissed.ExecuteIfBound();
			return FReply::Handled();
		})
		.ButtonColorAndOpacity(FLinearColor(0.15f, 0.12f, 0.08f))
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Continue", "[Continue]"))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.5f)))
		];
}

FText SSEEDialoguePanel::GetRevealedText() const
{
	const FString& FullText = CurrentLine.DialogueText.ToString();
	if (RevealedCharCount >= FullText.Len())
	{
		return CurrentLine.DialogueText;
	}
	return FText::FromString(FullText.Left(RevealedCharCount));
}

bool SSEEDialoguePanel::IsFullyRevealed() const
{
	return RevealedCharCount >= CurrentLine.DialogueText.ToString().Len();
}
