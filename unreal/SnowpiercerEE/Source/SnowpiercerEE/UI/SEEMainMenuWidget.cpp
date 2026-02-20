// SEEMainMenuWidget.cpp
#include "SEEMainMenuWidget.h"
#include "SEESaveGameSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void USEEMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GameTitleText)
	{
		GameTitleText->SetText(NSLOCTEXT("Menu", "Title", "SNOWPIERCER: ETERNAL ENGINE"));
	}

	// Disable continue if no save data
	if (ContinueButton)
	{
		ContinueButton->SetIsEnabled(HasSaveData());
	}

	// Bind button clicks
	if (NewGameButton)
	{
		NewGameButton->OnClicked.AddDynamic(this, &USEEMainMenuWidget::OnNewGameClicked);
	}
	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddDynamic(this, &USEEMainMenuWidget::OnContinueClicked);
	}
	if (SettingsButton)
	{
		SettingsButton->OnClicked.AddDynamic(this, &USEEMainMenuWidget::OnSettingsClicked);
	}
	if (CreditsButton)
	{
		CreditsButton->OnClicked.AddDynamic(this, &USEEMainMenuWidget::OnCreditsClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &USEEMainMenuWidget::OnQuitClicked);
	}
}

bool USEEMainMenuWidget::HasSaveData() const
{
	return UGameplayStatics::DoesSaveGameExist(TEXT("SnowpiercerEE_Main"), 0);
}

void USEEMainMenuWidget::OnNewGameClicked()
{
	if (NewGameMap.IsNull()) return;

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, NewGameMap);
}

void USEEMainMenuWidget::OnContinueClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	USEESaveGameSubsystem* SaveSub = GI->GetSubsystem<USEESaveGameSubsystem>();
	if (SaveSub)
	{
		SaveSub->LoadFromSlot();
	}

	if (!NewGameMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, NewGameMap);
	}
}

void USEEMainMenuWidget::OnSettingsClicked()
{
	OnSettingsRequested();
}

void USEEMainMenuWidget::OnCreditsClicked()
{
	OnCreditsRequested();
}

void USEEMainMenuWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
