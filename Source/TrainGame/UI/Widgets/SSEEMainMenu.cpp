// SSEEMainMenu.cpp - Main menu implementation
#include "SSEEMainMenu.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEMainMenu::Construct(const FArguments& InArgs)
{
	bHasSaveGame = InArgs._bHasSaveGame;
	OnNewGame = InArgs._OnNewGame;
	OnContinue = InArgs._OnContinue;
	OnSettings = InArgs._OnSettings;
	OnCredits = InArgs._OnCredits;
	OnQuit = InArgs._OnQuit;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.04f, 0.95f))
		[
			SNew(SVerticalBox)

			// Top spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.25f)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "GameTitle", "SNOWPIERCER"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.8f, 0.65f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 40)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "GameSubtitle", "ETERNAL ENGINE"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.55f)))
			]

			// Menu buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "NewGame", "NEW GAME"), OnNewGame)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "Continue", "CONTINUE"), OnContinue, bHasSaveGame)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "Settings", "SETTINGS"), OnSettings)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "Credits", "CREDITS"), OnCredits)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "Quit", "QUIT"), OnQuit)
			]

			// Bottom spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.35f)

			// Version
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			.Padding(20, 0, 20, 10)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Version", "Early Access Build"))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.35f, 0.35f, 0.35f)))
			]
		]
	];
}

void SSEEMainMenu::SetHasSaveGame(bool bHasSave)
{
	bHasSaveGame = bHasSave;
}

TSharedRef<SWidget> SSEEMainMenu::MakeMenuButton(const FText& Label, const FOnMainMenuAction& Action, bool bEnabled)
{
	return SNew(SBox)
		.WidthOverride(280.0f)
		.HeightOverride(44.0f)
		[
			SNew(SButton)
			.ButtonColorAndOpacity(bEnabled
				? FLinearColor(0.12f, 0.12f, 0.14f)
				: FLinearColor(0.06f, 0.06f, 0.07f))
			.IsEnabled(bEnabled)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.OnClicked_Lambda([Action]()
			{
				Action.ExecuteIfBound();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
				.ColorAndOpacity(FSlateColor(bEnabled
					? FLinearColor(0.85f, 0.82f, 0.7f)
					: FLinearColor(0.35f, 0.35f, 0.35f)))
				.Justification(ETextJustify::Center)
			]
		];
}
