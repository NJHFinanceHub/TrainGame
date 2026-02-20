// SSEEDeathScreen.cpp - Death screen implementation
#include "SSEEDeathScreen.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEDeathScreen::Construct(const FArguments& InArgs)
{
	OnReloadCheckpoint = InArgs._OnReloadCheckpoint;
	OnQuitToMenu = InArgs._OnQuitToMenu;
	DeathMessage = NSLOCTEXT("HUD", "DefaultDeath", "The cold claims another soul...");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.0f, 0.0f, 0.9f))
		[
			SNew(SVerticalBox)

			// Top spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.3f)

			// Death header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 16)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "YouDied", "YOU DIED"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.15f, 0.1f)))
			]

			// Death message
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(40, 0, 40, 40)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return DeathMessage; })
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 13))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.5f, 0.45f)))
				.Justification(ETextJustify::Center)
				.AutoWrapText(true)
			]

			// Reload checkpoint
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				SNew(SBox)
				.WidthOverride(280.0f)
				.HeightOverride(44.0f)
				[
					SNew(SButton)
					.ButtonColorAndOpacity(FLinearColor(0.15f, 0.08f, 0.05f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked_Lambda([this]()
					{
						OnReloadCheckpoint.ExecuteIfBound();
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("HUD", "Reload", "RELOAD CHECKPOINT"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.65f)))
						.Justification(ETextJustify::Center)
					]
				]
			]

			// Quit
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 12)
			[
				SNew(SBox)
				.WidthOverride(280.0f)
				.HeightOverride(44.0f)
				[
					SNew(SButton)
					.ButtonColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.OnClicked_Lambda([this]()
					{
						OnQuitToMenu.ExecuteIfBound();
						return FReply::Handled();
					})
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("HUD", "DeathQuit", "QUIT TO MENU"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.55f, 0.5f)))
						.Justification(ETextJustify::Center)
					]
				]
			]

			// Bottom spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.4f)
		]
	];
}

void SSEEDeathScreen::SetDeathMessage(const FText& Message)
{
	DeathMessage = Message;
}
