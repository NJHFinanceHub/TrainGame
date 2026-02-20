// SEEDeathScreenWidget.cpp
#include "SEEDeathScreenWidget.h"
#include "SEESaveGameSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void USEEDeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DeathTitleText)
	{
		DeathTitleText->SetText(NSLOCTEXT("Death", "Title", "YOU DIED"));
	}

	if (ReloadButton)
	{
		ReloadButton->OnClicked.AddDynamic(this, &USEEDeathScreenWidget::OnReloadCheckpointClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &USEEDeathScreenWidget::OnQuitToMenuClicked);
	}
}

void USEEDeathScreenWidget::ShowDeathScreen(const FText& CauseOfDeath)
{
	if (CauseOfDeathText)
	{
		CauseOfDeathText->SetText(CauseOfDeath);
	}

	SetVisibility(ESlateVisibility::Visible);
	OnDeathScreenShown(CauseOfDeath);
}

void USEEDeathScreenWidget::OnReloadCheckpointClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		USEESaveGameSubsystem* SaveSub = GI->GetSubsystem<USEESaveGameSubsystem>();
		if (SaveSub)
		{
			SaveSub->LoadFromSlot();
		}
	}

	// Restart current level
	UGameplayStatics::OpenLevel(this, *UGameplayStatics::GetCurrentLevelName(this));

	OnReloadRequested();
}

void USEEDeathScreenWidget::OnQuitToMenuClicked()
{
	if (!MainMenuMap.IsNull())
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
	}
}
