// SSEEPauseMenu.cpp - Pause menu implementation
#include "SSEEPauseMenu.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEPauseMenu::Construct(const FArguments& InArgs)
{
	OnResume = InArgs._OnResume;
	OnSave = InArgs._OnSave;
	OnLoad = InArgs._OnLoad;
	OnSettings = InArgs._OnSettings;
	OnQuitToMenu = InArgs._OnQuitToMenu;

	ChildSlot
	[
		// Semi-transparent backdrop
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		.Padding(0)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.95f))
				.Padding(40.0f)
				[
					SNew(SVerticalBox)

					// Title
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(0, 0, 0, 24)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("HUD", "Paused", "PAUSED"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
					]

					// Menu buttons
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Resume", "RESUME"), OnResume)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Save", "SAVE GAME"), OnSave)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Load", "LOAD GAME"), OnLoad)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "PauseSettings", "SETTINGS"), OnSettings)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(NSLOCTEXT("HUD", "QuitToMenu", "QUIT TO MENU"), OnQuitToMenu)
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEEPauseMenu::MakeMenuButton(const FText& Label, FOnPauseMenuAction Action)
{
	return SNew(SBox)
		.WidthOverride(260.0f)
		.HeightOverride(42.0f)
		[
			SNew(SButton)
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
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.9f)))
			]
		];
}
