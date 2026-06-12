// SSEEPauseMenu.cpp - Pause menu implementation ("Eternal Engine" chrome)
#include "SSEEPauseMenu.h"
#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEPanelFrame.h"
#include "Widgets/SSEEMenuButton.h"

#include "InputCoreTypes.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"

void SSEEPauseMenu::Construct(const FArguments& InArgs)
{
	OnResume = InArgs._OnResume;
	OnSave = InArgs._OnSave;
	OnLoad = InArgs._OnLoad;
	OnSettings = InArgs._OnSettings;
	OnQuitToMenu = InArgs._OnQuitToMenu;
	OnQuitToDesktop = InArgs._OnQuitToDesktop;

	ChildSlot
	[
		// Semi-transparent steel backdrop; clicks do not fall through
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::SteelBlack, 0.82f))
		.Padding(0.0f)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SSEEPanelFrame)
				.Title(NSLOCTEXT("HUD", "PausedTitle", "PAUSED - THE TRAIN DOES NOT STOP"))
				.ContentPadding(FMargin(28.0f, 24.0f, 28.0f, 28.0f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Resume", "RESUME"), OnResume)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Save", "SAVE GAME"), OnSave)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "Load", "LOAD GAME"), OnLoad)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "PauseSettings", "SETTINGS"), OnSettings)
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
					[
						MakeMenuButton(NSLOCTEXT("HUD", "QuitToMenu", "QUIT TO MENU"), OnQuitToMenu, /*bDanger=*/true)
					]
					+ SVerticalBox::Slot().AutoHeight()
					[
						MakeMenuButton(NSLOCTEXT("HUD", "QuitToDesktop", "QUIT TO DESKTOP"), OnQuitToDesktop, /*bDanger=*/true)
					]
				]
			]
		]
	];
}

FReply SSEEPauseMenu::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnResume.ExecuteIfBound();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

TSharedRef<SWidget> SSEEPauseMenu::MakeMenuButton(const FText& Label, FOnPauseMenuAction Action, bool bDanger)
{
	return SNew(SSEEMenuButton)
		.Text(Label)
		.Width(300.0f)
		.Height(44.0f)
		.TickColor(bDanger ? SEEUIStyle::BloodRed : SEEUIStyle::EngineAmber)
		.bUseDefaultTickColor(false)
		.OnClicked(FSimpleDelegate::CreateLambda([Action]() { Action.ExecuteIfBound(); }));
}
