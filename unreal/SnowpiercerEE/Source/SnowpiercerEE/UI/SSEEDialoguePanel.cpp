// SSEEDialoguePanel.cpp - Cinematic dialogue panel implementation
#include "SSEEDialoguePanel.h"

#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEPanelFrame.h"

#include "Input/Events.h"
#include "Misc/App.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Framework/Text/TextLayout.h"

namespace SSEEDialoguePrivate
{
	constexpr float FadeDuration = 0.30f;        // panel fade in/out
	constexpr float NamePlateDuration = 0.28f;   // speaker-change slide/fade
	constexpr float NamePlateSlideDistance = 26.0f;
	constexpr float ContentWidth = 980.0f;       // dialogue column width
	constexpr float LetterboxFraction = 0.08f;   // 8% darkened bands
}

void SSEEDialoguePanel::Construct(const FArguments& InArgs)
{
	using namespace SSEEDialoguePrivate;

	FadeSequence.AddCurve(0.0f, FadeDuration, ECurveEaseFunction::QuadOut);
	NamePlateSequence.AddCurve(0.0f, NamePlateDuration, ECurveEaseFunction::QuadOut);

	// Fade the whole panel in on open.
	FadeSequence.Play(AsShared());
	NamePlateSequence.JumpToEnd();

	ChoiceGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(4.0f, 3.0f));

	ChildSlot
	[
		SNew(SOverlay)

		// --- Letterbox: top and bottom darkened bands ---
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.FillHeight(LetterboxFraction)
			[
				MakeLetterboxBand()
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f - 2.0f * LetterboxFraction)
			[
				SNew(SSpacer)
			]

			+ SVerticalBox::Slot()
			.FillHeight(LetterboxFraction)
			[
				MakeLetterboxBand()
			]
		]

		// --- Bottom-anchored conversation column ---
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 46.0f))
		[
			SNew(SBox)
			.WidthOverride(ContentWidth)
			[
				SNew(SVerticalBox)

				// Speaker name plate (left, above the text box)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				.Padding(6.0f, 0.0f, 0.0f, 6.0f)
				[
					MakeNamePlate()
				]

				// Dialogue text box in steel chrome
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeDialogueBox()
				]

				// Choice grid (2x2, bottom-center) - only when the line offers choices
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 8.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.WidthOverride(ContentWidth)
					.Visibility_Lambda([this]()
					{
						return (CurrentLine.Choices.Num() > 0 && IsFullyRevealed())
							? EVisibility::Visible
							: EVisibility::Collapsed;
					})
					[
						ChoiceGrid.ToSharedRef()
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeLetterboxBand()
{
	return SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))
		.Padding(FMargin(0.0f));
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeNamePlate()
{
	using namespace SSEEDialoguePrivate;

	return SNew(SBox)
		// Slide in from the left + fade as the plate animates
		.RenderTransform_Lambda([this]()
		{
			const float Lerp = NamePlateSequence.GetLerp();
			return TOptional<FSlateRenderTransform>(
				FSlateRenderTransform(FVector2D(-NamePlateSlideDistance * (1.0f - Lerp), 0.0f)));
		})
		.Visibility_Lambda([this]()
		{
			return CurrentLine.SpeakerName.IsEmpty() ? EVisibility::Hidden : EVisibility::SelfHitTestInvisible;
		})
		[
			SNew(SBorder)
			.BorderImage(SEEUIStyle::WhiteBrush())
			// Fade the whole plate with the slide animation (RenderOpacity is
			// not a declarative arg in 5.7 — tint children instead)
			.ColorAndOpacity_Lambda([this]()
			{
				return FLinearColor(1.0f, 1.0f, 1.0f, NamePlateSequence.GetLerp());
			})
			.BorderBackgroundColor_Lambda([this]()
			{
				FLinearColor C = SEEUIStyle::Dim(SEEUIStyle::SteelBlack, 0.85f);
				C.A *= NamePlateSequence.GetLerp();
				return C;
			})
			.Padding(FMargin(14.0f, 7.0f, 18.0f, 7.0f))
			[
				SNew(SVerticalBox)

				// Spaced-caps speaker name, faction-tinted
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						// Uppercase manually (TextTransformPolicy is not a
						// declarative STextBlock arg in 5.7)
						return FText::FromString(CurrentLine.SpeakerName.ToString().ToUpper());
					})
					.Font(SEEUIStyle::MakeFont("Bold", 16, 260))
					.ColorAndOpacity_Lambda([this]() { return FSlateColor(CachedSpeakerColor); })
				]

				// Thin engine-amber underline
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f, 0.0f, 0.0f)
				[
					SNew(SBox)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(SEEUIStyle::WhiteBrush())
						.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.9f))
						.Padding(FMargin(0.0f))
					]
				]
			]
		];
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeDialogueBox()
{
	return SNew(SSEEPanelFrame)
		.bShowTexture(true)
		.ContentPadding(FMargin(26.0f, 18.0f, 26.0f, 14.0f))
		[
			SNew(SVerticalBox)

			// Dialogue text (typewriter reveal); fixed min height keeps the box
			// from jittering as text reveals.
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.MinDesiredHeight(74.0f)
				.VAlign(VAlign_Top)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetRevealedText(); })
					.Font(SEEUIStyle::BodyFont(17))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::BoneText))
					.AutoWrapText(true)
					.LineHeightPercentage(1.18f)
				]
			]

			// "Continue >" affordance, bottom-right (no-choice lines only)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				MakeContinuePrompt()
			]
		];
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeContinuePrompt()
{
	return SNew(SButton)
		.ButtonStyle(&SEEUIStyle::GetHollowButtonStyle())
		.ContentPadding(FMargin(8.0f, 2.0f))
		.Visibility_Lambda([this]()
		{
			// Show "Continue" only when text is fully revealed and there are no choices
			if (CurrentLine.Choices.Num() > 0) return EVisibility::Collapsed;
			if (!IsFullyRevealed()) return EVisibility::Collapsed;
			return EVisibility::Visible;
		})
		.OnClicked_Lambda([this]()
		{
			if (!bClosing)
			{
				OnDismissed.ExecuteIfBound();
			}
			return FReply::Handled();
		})
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "DlgContinue", "Continue ▸"))
			.Font(SEEUIStyle::CaptionFont(11))
			.ColorAndOpacity_Lambda([]()
			{
				// Gentle amber pulse so the affordance reads without shouting
				const float Pulse = 0.65f + 0.35f * static_cast<float>(FMath::Sin(FApp::GetCurrentTime() * 3.0));
				return FSlateColor(SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.45f + 0.4f * Pulse));
			})
		];
}

void SSEEDialoguePanel::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Master fade
	SetRenderOpacity(FadeSequence.GetLerp());

	// Fade-out finished: hand the close request to the owner exactly once.
	if (bClosing && !FadeSequence.IsPlaying())
	{
		bClosing = false;
		OnCloseRequested.ExecuteIfBound();
		return;
	}

	// Advance typewriter reveal (cache the substring only when it grows)
	if (!IsFullyRevealed())
	{
		RevealTimer += InDeltaTime;
		const int32 TargetChars = FMath::Min(
			FMath::FloorToInt(RevealTimer * CharsPerSecond), CachedFullText.Len());
		if (TargetChars != RevealedCharCount)
		{
			RevealedCharCount = TargetChars;
			CachedRevealedText = (RevealedCharCount >= CachedFullText.Len())
				? CurrentLine.DialogueText
				: FText::FromString(CachedFullText.Left(RevealedCharCount));
		}
	}
}

FReply SSEEDialoguePanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (bClosing)
	{
		return FReply::Handled(); // input frozen during the close fade
	}

	const FKey Key = InKeyEvent.GetKey();

	// Esc ends the conversation outright (after a short fade).
	if (Key == EKeys::Escape)
	{
		RequestClose();
		return FReply::Handled();
	}

	// Number keys pick choices (1-4).
	if (Key == EKeys::One)   { return SelectChoiceByIndex(0) ? FReply::Handled() : FReply::Unhandled(); }
	if (Key == EKeys::Two)   { return SelectChoiceByIndex(1) ? FReply::Handled() : FReply::Unhandled(); }
	if (Key == EKeys::Three) { return SelectChoiceByIndex(2) ? FReply::Handled() : FReply::Unhandled(); }
	if (Key == EKeys::Four)  { return SelectChoiceByIndex(3) ? FReply::Handled() : FReply::Unhandled(); }

	// E / Enter / Space: first press finishes the typewriter, second continues.
	if (Key == EKeys::E || Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		if (!IsFullyRevealed())
		{
			CompleteReveal();
			return FReply::Handled();
		}
		if (CurrentLine.Choices.Num() == 0)
		{
			OnDismissed.ExecuteIfBound();
		}
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SSEEDialoguePanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bClosing)
	{
		return FReply::Handled();
	}

	// Click anywhere: finish the typewriter; a second click advances
	// no-choice lines (Fallout-style).
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (!IsFullyRevealed())
		{
			CompleteReveal();
			return FReply::Handled();
		}
		if (CurrentLine.Choices.Num() == 0)
		{
			OnDismissed.ExecuteIfBound();
			return FReply::Handled();
		}
	}

	return SCompoundWidget::OnMouseButtonDown(MyGeometry, MouseEvent);
}

bool SSEEDialoguePanel::SelectChoiceByIndex(int32 ChoiceIndex)
{
	if (!CurrentLine.Choices.IsValidIndex(ChoiceIndex)) return false;
	if (!CurrentLine.Choices[ChoiceIndex].bIsAvailable) return false;

	// Choices are only actionable once the line is readable.
	if (!IsFullyRevealed())
	{
		CompleteReveal();
		return true;
	}

	OnChoiceSelected.ExecuteIfBound(CurrentLine.Choices[ChoiceIndex].ChoiceID);
	return true;
}

void SSEEDialoguePanel::CompleteReveal()
{
	RevealedCharCount = CachedFullText.Len();
	CachedRevealedText = CurrentLine.DialogueText;
}

void SSEEDialoguePanel::RequestClose()
{
	if (bClosing) return;
	bClosing = true;
	FadeSequence.PlayReverse(AsShared());
}

FLinearColor SSEEDialoguePanel::GetSpeakerColor() const
{
	return CachedSpeakerColor;
}

FText SSEEDialoguePanel::MakeGatePrefix(const FDialogueChoice& Choice)
{
	if (Choice.StatRequirement.IsNone())
	{
		return FText::GetEmpty();
	}

	// "Perception" + 5 -> "[PER 5]"
	const FString StatString = Choice.StatRequirement.ToString().ToUpper().Left(3);
	return FText::FromString(FString::Printf(TEXT("[%s %d]"),
		*StatString, FMath::RoundToInt(Choice.RequiredValue)));
}

FText SSEEDialoguePanel::StripLeadingNumber(const FText& ChoiceText)
{
	const FString Raw = ChoiceText.ToString();

	int32 Index = 0;
	while (Index < Raw.Len() && FChar::IsDigit(Raw[Index]))
	{
		++Index;
	}
	if (Index > 0 && Index < Raw.Len() && Raw[Index] == TEXT('.'))
	{
		++Index;
		while (Index < Raw.Len() && FChar::IsWhitespace(Raw[Index]))
		{
			++Index;
		}
		return FText::FromString(Raw.Mid(Index));
	}
	return ChoiceText;
}

void SSEEDialoguePanel::SetDialogueLine(const FDialogueLine& Line)
{
	CurrentLine = Line;
	RevealedCharCount = 0;
	RevealTimer = 0.0f;
	CachedFullText = Line.DialogueText.ToString();
	CachedRevealedText = FText::GetEmpty();

	// Speaker change: recompute the faction tint and replay the plate animation.
	const FString SpeakerString = Line.SpeakerName.ToString();
	if (!SpeakerString.Equals(LastSpeakerString))
	{
		LastSpeakerString = SpeakerString;

		// Faction tint heuristic: enforcers run blood-red, the Order and the
		// Engine's voices run frost-blue, everyone else burns engine-amber.
		if (SpeakerString.Contains(TEXT("Jackboot")) ||
			SpeakerString.Contains(TEXT("Commander")) ||
			SpeakerString.Contains(TEXT("Grey")))
		{
			CachedSpeakerColor = FMath::Lerp(SEEUIStyle::BloodRed, FLinearColor::White, 0.30f);
		}
		else if (SpeakerString.Contains(TEXT("Wilford")) ||
			SpeakerString.Contains(TEXT("Order")) ||
			SpeakerString.Contains(TEXT("Engine")))
		{
			CachedSpeakerColor = SEEUIStyle::FrostBlue;
		}
		else
		{
			CachedSpeakerColor = SEEUIStyle::EngineAmber;
		}

		NamePlateSequence.JumpToStart();
		NamePlateSequence.Play(AsShared());
	}

	// Rebuild the choice grid (2 columns, up to 2 rows)
	if (ChoiceGrid.IsValid())
	{
		ChoiceGrid->ClearChildren();

		const int32 NumChoices = FMath::Min(Line.Choices.Num(), 4);
		for (int32 Index = 0; Index < NumChoices; ++Index)
		{
			ChoiceGrid->AddSlot(Index % 2, Index / 2)
			[
				MakeChoiceTile(Line.Choices[Index], Index)
			];
		}
	}
}

TSharedRef<SWidget> SSEEDialoguePanel::MakeChoiceTile(const FDialogueChoice& Choice, int32 DisplayIndex)
{
	const FName ChoiceID = Choice.ChoiceID;
	const bool bAvailable = Choice.bIsAvailable;
	const FText ChoiceText = StripLeadingNumber(Choice.ChoiceText);
	const FText GatePrefix = MakeGatePrefix(Choice);

	// Tooltip carries the full (untruncated) choice text, plus the lockout
	// reason when the option is gated off.
	FText TooltipText = ChoiceText;
	if (!bAvailable && !Choice.UnavailableReason.IsEmpty())
	{
		TooltipText = FText::Format(NSLOCTEXT("HUD", "DlgChoiceLockedTip", "{0}\n{1}"),
			ChoiceText, Choice.UnavailableReason);
	}

	// Build the button shell first so the hover lambda below can hold a valid
	// weak pointer (nested SNew content is constructed before SAssignNew binds).
	TSharedRef<SButton> TileButton = SNew(SButton)
		.ButtonStyle(&SEEUIStyle::GetHollowButtonStyle())
		.ContentPadding(FMargin(0.0f))
		.IsEnabled(bAvailable)
		.ToolTipText(TooltipText)
		.OnClicked_Lambda([this, ChoiceID]()
		{
			if (!bClosing)
			{
				if (!IsFullyRevealed())
				{
					CompleteReveal();
				}
				else
				{
					OnChoiceSelected.ExecuteIfBound(ChoiceID);
				}
			}
			return FReply::Handled();
		});

	TileButton->SetContent(
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::RivetLine, 0.9f))
				.Padding(FMargin(1.0f))
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					// Hover warms the steel toward engine-amber
					.BorderBackgroundColor_Lambda([WeakButton = TWeakPtr<SButton>(TileButton), bAvailable]()
					{
						if (!bAvailable)
						{
							return FSlateColor(SEEUIStyle::Dim(SEEUIStyle::SteelBlack, 0.85f));
						}
						const TSharedPtr<SButton> Pinned = WeakButton.Pin();
						const bool bHover = Pinned.IsValid() && Pinned->IsHovered();
						const bool bPressed = Pinned.IsValid() && Pinned->IsPressed();
						if (bPressed)
						{
							return FSlateColor(FMath::Lerp(SEEUIStyle::Gunmetal, SEEUIStyle::EngineAmber, 0.40f));
						}
						return FSlateColor(bHover
							? FMath::Lerp(SEEUIStyle::GunmetalHover, SEEUIStyle::EngineAmber, 0.18f)
							: SEEUIStyle::Dim(SEEUIStyle::Gunmetal, 0.96f));
					})
					.Padding(FMargin(6.0f, 0.0f, 10.0f, 0.0f))
					[
						SNew(SHorizontalBox)

						// Number badge (1-4)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, 10.0f, 0.0f)
						[
							SNew(SBox)
							.WidthOverride(22.0f)
							.HeightOverride(22.0f)
							[
								SNew(SBorder)
								.BorderImage(SEEUIStyle::WhiteBrush())
								.BorderBackgroundColor(bAvailable
									? SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.18f)
									: SEEUIStyle::Dim(SEEUIStyle::TextFaint, 0.10f))
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								.Padding(FMargin(0.0f))
								[
									SNew(STextBlock)
									.Text(FText::AsNumber(DisplayIndex + 1))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
									.ColorAndOpacity(FSlateColor(bAvailable
										? SEEUIStyle::EngineAmber
										: SEEUIStyle::TextDisabled))
								]
							]
						]

						// Optional dim stat-gate prefix, e.g. "[PER 5]"
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(0.0f, 0.0f, 8.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(GatePrefix)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FSlateColor(bAvailable
								? SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.75f)
								: SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.35f)))
							.Visibility(GatePrefix.IsEmpty() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible)
						]

						// Choice text: one line, ellipsis overflow (tooltip has the rest)
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(ChoiceText)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
							.ColorAndOpacity(FSlateColor(bAvailable
								? SEEUIStyle::BoneText
								: SEEUIStyle::TextDisabled))
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						]

						// Lockout reason tag (gated choices)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						.Padding(8.0f, 0.0f, 0.0f, 0.0f)
						[
							SNew(STextBlock)
							.Text(Choice.UnavailableReason)
							.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
							.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BloodRed, 0.8f)))
							.Visibility((bAvailable || Choice.UnavailableReason.IsEmpty())
								? EVisibility::Collapsed
								: EVisibility::SelfHitTestInvisible)
						]
					]
				]
	);

	return SNew(SBox)
		.HeightOverride(40.0f)
		[
			TileButton
		];
}

bool SSEEDialoguePanel::IsFullyRevealed() const
{
	return RevealedCharCount >= CachedFullText.Len();
}
