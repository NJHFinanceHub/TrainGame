// SSEEMenuButton.cpp - Industrial menu button implementation
#include "SSEEMenuButton.h"
#include "SSEEUIStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

namespace
{
	constexpr float TickWidthIdle = 4.0f;
	constexpr float TickWidthHovered = 14.0f;
	constexpr float HoverAnimDuration = 0.12f;
}

void SSEEMenuButton::Construct(const FArguments& InArgs)
{
	OnClicked = InArgs._OnClicked;
	TickAccent = InArgs._bUseDefaultTickColor ? SEEUIStyle::EngineAmber : InArgs._TickColor;

	HoverSequence.AddCurve(0.0f, HoverAnimDuration, ECurveEaseFunction::QuadOut);
	HoverSequence.JumpToStart(); // never-played sequences report lerp 1.0; start collapsed

	SetEnabled(InArgs._bEnabled);

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(InArgs._Width)
		.HeightOverride(InArgs._Height)
		[
			SAssignNew(Button, SButton)
			.ButtonStyle(&SEEUIStyle::GetHollowButtonStyle())
			.ContentPadding(FMargin(0.0f))
			.OnClicked(this, &SSEEMenuButton::HandleClicked)
			[
				// Panel body: thin rivet-line outline around the lit panel
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::RivetLine, 0.85f))
				.Padding(FMargin(1.0f))
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor(this, &SSEEMenuButton::GetPanelColor)
					.Padding(FMargin(0.0f))
					[
						SNew(SHorizontalBox)

						// Accent tick bar (expands on hover/focus)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Fill)
						[
							SNew(SBox)
							.WidthOverride(this, &SSEEMenuButton::GetTickWidth)
							[
								SNew(SBorder)
								.BorderImage(SEEUIStyle::WhiteBrush())
								.BorderBackgroundColor(this, &SSEEMenuButton::GetTickColor)
								.Padding(FMargin(0.0f))
							]
						]

						// Spaced-caps label
						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.VAlign(VAlign_Center)
						.Padding(FMargin(20.0f, 0.0f, 12.0f, 0.0f))
						[
							SNew(STextBlock)
							.Text(InArgs._Text)
							.Font(SEEUIStyle::CaptionFont(14))
							.ColorAndOpacity(this, &SSEEMenuButton::GetLabelColor)
						]
					]
				]
			]
		]
	];
}

void SSEEMenuButton::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Drive the tick-bar expansion toward the current highlight state.
	const bool bHighlight = IsHighlighted();
	if (bHighlight != bHoverAnimTarget)
	{
		bHoverAnimTarget = bHighlight;
		if (bHighlight)
		{
			HoverSequence.Play(AsShared());
		}
		else
		{
			HoverSequence.PlayReverse(AsShared());
		}
	}
}

FReply SSEEMenuButton::HandleClicked()
{
	OnClicked.ExecuteIfBound();
	return FReply::Handled();
}

bool SSEEMenuButton::IsHighlighted() const
{
	if (!IsEnabled())
	{
		return false;
	}
	const bool bHovered = Button.IsValid() && Button->IsHovered();
	const bool bFocused = (Button.IsValid() && Button->HasAnyUserFocus().IsSet()) || HasAnyUserFocus().IsSet();
	return bHovered || bFocused;
}

FSlateColor SSEEMenuButton::GetPanelColor() const
{
	const float DisabledAlpha = IsEnabled() ? 1.0f : 0.4f;

	if (Button.IsValid() && Button->IsPressed())
	{
		// Amber flash on press
		const FLinearColor Flash = FLinearColor::LerpUsingHSV(SEEUIStyle::Gunmetal, TickAccent, 0.45f);
		return SEEUIStyle::Dim(Flash, DisabledAlpha);
	}

	const float HoverAlpha = HoverSequence.GetLerp();
	const FLinearColor Body = FMath::Lerp(SEEUIStyle::Gunmetal, SEEUIStyle::GunmetalHover, HoverAlpha);
	return SEEUIStyle::Dim(Body, DisabledAlpha);
}

FSlateColor SSEEMenuButton::GetLabelColor() const
{
	if (!IsEnabled())
	{
		return SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.4f);
	}
	const float HoverAlpha = HoverSequence.GetLerp();
	return FMath::Lerp(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.92f), FLinearColor::White, HoverAlpha * 0.6f);
}

FSlateColor SSEEMenuButton::GetTickColor() const
{
	const float DisabledAlpha = IsEnabled() ? 1.0f : 0.4f;
	const float HoverAlpha = HoverSequence.GetLerp();
	// Idle: dimmed accent; hover: full accent
	return SEEUIStyle::Dim(TickAccent, DisabledAlpha * (0.55f + 0.45f * HoverAlpha));
}

FOptionalSize SSEEMenuButton::GetTickWidth() const
{
	return FMath::Lerp(TickWidthIdle, TickWidthHovered, HoverSequence.GetLerp());
}
