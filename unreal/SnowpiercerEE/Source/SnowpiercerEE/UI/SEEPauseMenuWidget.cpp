// SEEPauseMenuWidget.cpp
#include "SEEPauseMenuWidget.h"
#include "SEESaveGameSubsystem.h"
#include "UI/SEEUISubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USEEPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PauseTitleText)
	{
		PauseTitleText->SetText(NSLOCTEXT("Pause", "Title", "PAUSED"));
	}

	if (ResumeButton) ResumeButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnResumeClicked);
	if (SaveButton) SaveButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnSaveClicked);
	if (LoadButton) LoadButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnLoadClicked);
	if (SettingsButton) SettingsButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnSettingsClicked);
	if (QuitToMenuButton) QuitToMenuButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnQuitToMenuClicked);
	if (QuitToDesktopButton) QuitToDesktopButton->OnClicked.AddDynamic(this, &USEEPauseMenuWidget::OnQuitToDesktopClicked);
}

void USEEPauseMenuWidget::OnResumeClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		USEEUISubsystem* UI = GI->GetSubsystem<USEEUISubsystem>();
		if (UI)
		{
			UI->CloseCurrentScreen();
		}
	}

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		UGameplayStatics::SetGamePaused(this, false);
	}

	OnResumeRequested();
}

void USEEPauseMenuWidget::OnSaveClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USEESaveGameSubsystem* SaveSub = GI->GetSubsystem<USEESaveGameSubsystem>();
	bool bSuccess = false;
	if (SaveSub)
	{
		bSuccess = SaveSub->WriteToSlot();
	}

	if (SaveStatusText)
	{
		SaveStatusText->SetText(bSuccess
			? NSLOCTEXT("Pause", "SaveOK", "Game Saved")
			: NSLOCTEXT("Pause", "SaveFail", "Save Failed"));
		SaveStatusText->SetColorAndOpacity(bSuccess
			? FSlateColor(FLinearColor::Green)
			: FSlateColor(FLinearColor::Red));
		SaveStatusText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	OnSaveCompleted(bSuccess);
}

void USEEPauseMenuWidget::OnLoadClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USEESaveGameSubsystem* SaveSub = GI->GetSubsystem<USEESaveGameSubsystem>();
	if (SaveSub)
	{
		SaveSub->LoadFromSlot();
	}

	// Unpause and reload level
	UGameplayStatics::SetGamePaused(this, false);
}

void USEEPauseMenuWidget::OnSettingsClicked()
{
	OnSettingsRequested();
}

void USEEPauseMenuWidget::OnQuitToMenuClicked()
{
	UGameplayStatics::SetGamePaused(this, false);

	if (!MainMenuMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
	}
}

void USEEPauseMenuWidget::OnQuitToDesktopClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
