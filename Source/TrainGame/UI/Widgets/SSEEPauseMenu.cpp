// SSEEPauseMenu.cpp - Pause menu implementation
#include "SSEEPauseMenu.h"

#include "Widgets/SBoxPanel.h"
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
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		[
			SNew(SVerticalBox)

			// Top spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.3f)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 32)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Paused", "PAUSED"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.82f, 0.7f)))
			]

			// Menu buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 10)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "Resume", "RESUME"), OnResume)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 10)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "SaveGame", "SAVE GAME"), OnSave)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 10)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "LoadGame", "LOAD GAME"), OnLoad)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 10)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "PSettings", "SETTINGS"), OnSettings)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0, 0, 0, 10)
			[
				MakeMenuButton(NSLOCTEXT("HUD", "QuitMenu", "QUIT TO MENU"), OnQuitToMenu)
			]

			// Bottom spacer
			+ SVerticalBox::Slot()
			.FillHeight(0.4f)
		]
	];
}

TSharedRef<SWidget> SSEEPauseMenu::MakeMenuButton(const FText& Label, const FOnPauseMenuAction& Action)
{
	return SNew(SBox)
		.WidthOverride(260.0f)
		.HeightOverride(40.0f)
		[
			SNew(SButton)
			.ButtonColorAndOpacity(FLinearColor(0.1f, 0.1f, 0.12f))
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
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.82f, 0.7f)))
				.Justification(ETextJustify::Center)
			]
		];
}
