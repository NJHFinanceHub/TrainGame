// SSEEDeathScreen.cpp - Death screen implementation ("Eternal Engine" chrome)
#include "SSEEDeathScreen.h"
#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEMenuButton.h"

#include "InputCoreTypes.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	// Deep red-black base of the death backdrop.
	const FLinearColor DeathBase = FLinearColor(0.045f, 0.012f, 0.010f, 0.96f);
}

void SSEEDeathScreen::Construct(const FArguments& InArgs)
{
	OnReloadCheckpoint = InArgs._OnReloadCheckpoint;
	OnQuitToMenu = InArgs._OnQuitToMenu;

	ChildSlot
	[
		// Deep red-black backdrop
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(DeathBase)
		.Padding(0.0f)
		[
			SNew(SOverlay)

			// Vignette layering (darker bands at the screen edges)
			+ SOverlay::Slot()
			[
				MakeVignetteLayers()
			]

			// Centered content
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)

				// Stencil overline
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "DeathOverline", "PASSENGER TERMINATED"))
					.Font(SEEUIStyle::OverlineFont(12))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BloodRed, 0.85f)))
				]

				// Huge stencil header: FROZEN / THE TAIL REMEMBERS
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					SNew(STextBlock)
					.Text(this, &SSEEDeathScreen::GetHeaderText)
					.Font(SEEUIStyle::TitleFont(46))
					.ColorAndOpacity(this, &SSEEDeathScreen::GetHeaderColor)
				]

				// Thin blood-red rule
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 14.0f)
				[
					SNew(SBox)
					.WidthOverride(300.0f)
					.HeightOverride(2.0f)
					[
						SNew(SBorder)
						.BorderImage(SEEUIStyle::WhiteBrush())
						.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::BloodRed, 0.7f))
						.Padding(0.0f)
					]
				]

				// Death cause
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 44.0f)
				[
					SNew(STextBlock)
					.Text(this, &SSEEDeathScreen::GetCauseText)
					.Font(SEEUIStyle::BodyFont(15))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.75f)))
				]

				// Reload Checkpoint
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 10.0f)
				[
					SNew(SSEEMenuButton)
					.Text(NSLOCTEXT("HUD", "Reload", "RELOAD CHECKPOINT"))
					.Width(320.0f).Height(50.0f)
					.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnReloadCheckpoint.ExecuteIfBound(); }))
				]

				// Quit to Menu
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SSEEMenuButton)
					.Text(NSLOCTEXT("HUD", "DeathQuit", "QUIT TO MENU"))
					.Width(320.0f).Height(50.0f)
					.TickColor(SEEUIStyle::BloodRed)
					.bUseDefaultTickColor(false)
					.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnQuitToMenu.ExecuteIfBound(); }))
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEEDeathScreen::MakeVignetteLayers()
{
	// Layered darkening bands: heavier at top/bottom, lighter mid - a cheap,
	// asset-free vignette that pushes the eye to the centered header.
	return SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().FillHeight(0.22f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(0.16f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.25f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(0.24f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(0.16f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.25f))
				.Padding(0.0f)
			]
			+ SVerticalBox::Slot().FillHeight(0.22f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f))
				.Padding(0.0f)
			]
		]

		// Side bands for the horizontal axis of the vignette
		+ SOverlay::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(0.14f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.40f))
				.Padding(0.0f)
			]
			+ SHorizontalBox::Slot().FillWidth(0.72f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f))
				.Padding(0.0f)
			]
			+ SHorizontalBox::Slot().FillWidth(0.14f)
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.40f))
				.Padding(0.0f)
			]
		];
}

bool SSEEDeathScreen::IsColdDeath() const
{
	if (DeathCause.IsEmpty())
	{
		return true; // default flavor is the cold
	}
	const FString Cause = DeathCause.ToString().ToLower();
	return Cause.Contains(TEXT("cold")) || Cause.Contains(TEXT("frost"))
		|| Cause.Contains(TEXT("froze")) || Cause.Contains(TEXT("ice"))
		|| Cause.Contains(TEXT("frozen")) || Cause.Contains(TEXT("hypotherm"));
}

FText SSEEDeathScreen::GetHeaderText() const
{
	return IsColdDeath()
		? NSLOCTEXT("HUD", "DeathFrozen", "FROZEN")
		: NSLOCTEXT("HUD", "DeathTail", "THE TAIL REMEMBERS");
}

FSlateColor SSEEDeathScreen::GetHeaderColor() const
{
	return IsColdDeath() ? FSlateColor(SEEUIStyle::FrostBlue) : FSlateColor(SEEUIStyle::BloodRed);
}

FText SSEEDeathScreen::GetCauseText() const
{
	return DeathCause.IsEmpty()
		? NSLOCTEXT("HUD", "DeathDefault", "The cold claims another soul.")
		: DeathCause;
}

void SSEEDeathScreen::SetDeathCause(const FText& InCause)
{
	DeathCause = InCause;
}

FReply SSEEDeathScreen::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
	{
		OnReloadCheckpoint.ExecuteIfBound();
	}
	return FReply::Handled();
}
