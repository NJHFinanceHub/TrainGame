// SSEEMainMenu.cpp - Main menu implementation
#include "SSEEMainMenu.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SOverlay.h"

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
		// Full screen dark background
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.04f, 1.0f))
		.Padding(0)
		[
			SNew(SOverlay)

			// Centered content
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 60)
				[
					MakeTitle()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					MakeMenuButtons()
				]
			]

			// Version info (bottom-right)
			+ SOverlay::Slot()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(20)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Version", "v0.1.0 - Early Development"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.3f, 0.3f)))
			]
		]
	];
}

TSharedRef<SWidget> SSEEMainMenu::MakeTitle()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "GameTitle", "SNOWPIERCER"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.9f, 1.0f)))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0, 4, 0, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "GameSubtitle", "ETERNAL ENGINE"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.65f, 0.8f)))
		];
}

TSharedRef<SWidget> SSEEMainMenu::MakeMenuButtons()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			MakeMenuButton(NSLOCTEXT("HUD", "NewGame", "NEW GAME"), OnNewGame)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			MakeMenuButton(NSLOCTEXT("HUD", "Continue", "CONTINUE"), OnContinue, bHasSaveGame)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			MakeMenuButton(NSLOCTEXT("HUD", "Settings", "SETTINGS"), OnSettings)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			MakeMenuButton(NSLOCTEXT("HUD", "Credits", "CREDITS"), OnCredits)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			MakeMenuButton(NSLOCTEXT("HUD", "Quit", "QUIT"), OnQuit)
		];
}

TSharedRef<SWidget> SSEEMainMenu::MakeMenuButton(const FText& Label, FOnMenuAction Action, bool bEnabled)
{
	return SNew(SBox)
		.WidthOverride(300.0f)
		.HeightOverride(48.0f)
		[
			SNew(SButton)
			.IsEnabled(bEnabled)
			.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.14f))
			.OnClicked_Lambda([Action]()
			{
				Action.ExecuteIfBound();
				return FReply::Handled();
			})
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity_Lambda([bEnabled]()
				{
					return bEnabled
						? FSlateColor(FLinearColor(0.85f, 0.85f, 0.9f))
						: FSlateColor(FLinearColor(0.3f, 0.3f, 0.35f));
				})
			]
		];
}
