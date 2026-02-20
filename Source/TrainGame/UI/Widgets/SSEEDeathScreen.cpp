// SSEEDeathScreen.cpp - Death screen implementation
#include "SSEEDeathScreen.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEDeathScreen::Construct(const FArguments& InArgs)
{
	OnReloadCheckpoint = InArgs._OnReloadCheckpoint;
	OnQuitToMenu = InArgs._OnQuitToMenu;

	ChildSlot
	[
		// Dark red-tinted backdrop
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.08f, 0.0f, 0.0f, 0.9f))
		.Padding(0)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)

				// "YOU DIED" header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 12)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "YouDied", "YOU DIED"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.15f, 0.1f)))
				]

				// Death cause
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 40)
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return DeathCause.IsEmpty()
							? NSLOCTEXT("HUD", "DeathDefault", "The cold claims another soul.")
							: DeathCause;
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Italic", 12))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.4f, 0.4f)))
				]

				// Reload Checkpoint
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0, 0, 0, 8)
				[
					SNew(SBox)
					.WidthOverride(280.0f)
					.HeightOverride(48.0f)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(FLinearColor(0.15f, 0.08f, 0.08f))
						.OnClicked_Lambda([this]()
						{
							OnReloadCheckpoint.ExecuteIfBound();
							return FReply::Handled();
						})
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("HUD", "Reload", "RELOAD CHECKPOINT"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.7f)))
						]
					]
				]

				// Quit to Menu
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(280.0f)
					.HeightOverride(48.0f)
					[
						SNew(SButton)
						.ButtonColorAndOpacity(FLinearColor(0.1f, 0.05f, 0.05f))
						.OnClicked_Lambda([this]()
						{
							OnQuitToMenu.ExecuteIfBound();
							return FReply::Handled();
						})
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("HUD", "DeathQuit", "QUIT TO MENU"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
							.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.4f, 0.4f)))
						]
					]
				]
			]
		]
	];
}

void SSEEDeathScreen::SetDeathCause(const FText& InCause)
{
	DeathCause = InCause;
}
