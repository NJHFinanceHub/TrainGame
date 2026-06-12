// SSEEMainMenu.cpp - Main menu implementation ("Eternal Engine" chrome)
#include "SSEEMainMenu.h"
#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEMenuButton.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

void SSEEMainMenu::Construct(const FArguments& InArgs)
{
	bHasSaveGame = InArgs._bHasSaveGame;
	bShowCredits = InArgs._bShowCredits;
	OnNewGame = InArgs._OnNewGame;
	OnContinue = InArgs._OnContinue;
	OnSettings = InArgs._OnSettings;
	OnCredits = InArgs._OnCredits;
	OnQuit = InArgs._OnQuit;

	BackdropTextureBrush = SEEUIStyle::CreateBackdropTextureBrush();

	// Slow breathing pulse for the amber underline: up, then down, looped.
	PulseUpCurve   = PulseSequence.AddCurve(0.0f, 1.4f, ECurveEaseFunction::QuadInOut);
	PulseDownCurve = PulseSequence.AddCurve(1.4f, 1.4f, ECurveEaseFunction::QuadInOut);
	PulseSequence.Play(AsShared(), /*bPlayLooped=*/true);

	ChildSlot
	[
		// Full-screen near-black steel base
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::SteelBlack)
		.Padding(0.0f)
		[
			SNew(SOverlay)

			// Cold-blue vertical gradient + faint metal wash
			+ SOverlay::Slot()
			[
				MakeBackdropLayers()
			]

			// Centered title block (upper third)
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Top)
			.Padding(FMargin(0.0f, 110.0f, 0.0f, 0.0f))
			[
				MakeTitle()
			]

			// Left-aligned vertical menu
			+ SOverlay::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(140.0f, 120.0f, 0.0f, 0.0f))
			[
				MakeMenuButtons()
			]

			// Bottom strip: version left, tagline right
			+ SOverlay::Slot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(40.0f, 0.0f, 40.0f, 24.0f))
			[
				MakeBottomStrip()
			]
		]
	];
}

TSharedRef<SWidget> SSEEMainMenu::MakeBackdropLayers()
{
	return SNew(SOverlay)

		// Vertical cold gradient, banded from frost-tinted top to pure steel bottom
		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.085f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.05f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.022f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.10f))
				.Padding(0.0f)
			]
		]

		// Faint scratched-steel wash over the gradient (transparent if asset missing)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(BackdropTextureBrush.Get())
		];
}

TSharedRef<SWidget> SSEEMainMenu::MakeTitle()
{
	return SNew(SVerticalBox)

		// Amber stencil overline - the litany of the train
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 14.0f))
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "TitleOverline", "THE ETERNAL ENGINE PROVIDES"))
			.Font(SEEUIStyle::OverlineFont(13))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.9f)))
		]

		// Masthead
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "GameTitle", "SNOWPIERCER"))
			.Font(SEEUIStyle::TitleFont(48))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TitleIce))
		]

		// Pulsing amber underline (idle animation, Slate-native)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(360.0f)
			.HeightOverride(2.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(this, &SSEEMainMenu::GetPulseUnderlineColor)
				.Padding(0.0f)
			]
		]

		// Sub-line
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "GameSubtitle", "ETERNAL ENGINE"))
			.Font(SEEUIStyle::MakeFont("Bold", 18, 540))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.95f)))
		]

		// Thin rule with rivet dots
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
		[
			MakeRivetRule()
		];
}

TSharedRef<SWidget> SSEEMainMenu::MakeRivetRule()
{
	const auto MakeLine = []()
	{
		return SNew(SBox)
			.WidthOverride(110.0f)
			.HeightOverride(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::RivetLine)
				.Padding(0.0f)
			];
	};

	const auto MakeRivet = []()
	{
		return SNew(SBox)
			.WidthOverride(4.0f)
			.HeightOverride(4.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.55f))
				.Padding(0.0f)
			];
	};

	TSharedRef<SHorizontalBox> Rule = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			MakeLine()
		];

	for (int32 i = 0; i < 5; ++i)
	{
		Rule->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(FMargin(7.0f, 0.0f))
		[
			MakeRivet()
		];
	}

	Rule->AddSlot().AutoWidth().VAlign(VAlign_Center)
	[
		MakeLine()
	];

	return Rule;
}

FSlateColor SSEEMainMenu::GetPulseUnderlineColor() const
{
	// Triangle wave 0->1->0 built from the two sequence curves, eased per-curve.
	const float Pulse = FMath::Clamp(PulseUpCurve.GetLerp() - PulseDownCurve.GetLerp(), 0.0f, 1.0f);
	return SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.35f + 0.6f * Pulse);
}

FReply SSEEMainMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	// Consume everything; menu interaction is mouse-driven.
	return FReply::Handled();
}

TSharedRef<SWidget> SSEEMainMenu::MakeMenuButtons()
{
	constexpr float ButtonWidth = 340.0f;
	constexpr float ButtonHeight = 52.0f;

	TSharedRef<SVerticalBox> Buttons = SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "NewGame", "NEW GAME"))
			.Width(ButtonWidth).Height(ButtonHeight)
			.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnNewGame.ExecuteIfBound(); }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "Continue", "CONTINUE"))
			.Width(ButtonWidth).Height(ButtonHeight)
			.bEnabled(bHasSaveGame)
			.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnContinue.ExecuteIfBound(); }))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "Settings", "SETTINGS"))
			.Width(ButtonWidth).Height(ButtonHeight)
			.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnSettings.ExecuteIfBound(); }))
		];

	if (bShowCredits)
	{
		Buttons->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
		[
			SNew(SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "Credits", "CREDITS"))
			.Width(ButtonWidth).Height(ButtonHeight)
			.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnCredits.ExecuteIfBound(); }))
		];
	}

	Buttons->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
	[
		SNew(SSEEMenuButton)
		.Text(NSLOCTEXT("HUD", "Quit", "QUIT"))
		.Width(ButtonWidth).Height(ButtonHeight)
		.TickColor(SEEUIStyle::BloodRed)
		.bUseDefaultTickColor(false)
		.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnQuit.ExecuteIfBound(); }))
	];

	return Buttons;
}

TSharedRef<SWidget> SSEEMainMenu::MakeBottomStrip()
{
	return SNew(SVerticalBox)

		// Thin rivet-line rule above the strip
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 10.0f))
		[
			SNew(SBox)
			.HeightOverride(1.0f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::RivetLine, 0.7f))
				.Padding(0.0f)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Version", "v0.1.0 - Early Development"))
				.Font(SEEUIStyle::CaptionFont(11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.4f)))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Tagline", "1001 CARS. ONE ENGINE. NO MERCY."))
				.Font(SEEUIStyle::CaptionFont(11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::FrostBlue, 0.55f)))
			]
		];
}
