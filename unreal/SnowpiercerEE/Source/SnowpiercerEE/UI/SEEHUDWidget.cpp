#include "SEEHUDWidget.h"
#include "SEEInventoryWidget.h"
#include "SEETrainMapWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void USEEHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (InteractionPanel)
	{
		InteractionPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (TrainMapWidget)
	{
		TrainMapWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USEEHUDWidget::SetHealthPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetStaminaPercent(float Percent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetHungerPercent(float Percent)
{
	if (HungerBar)
	{
		HungerBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetColdPercent(float Percent)
{
	if (ColdBar)
	{
		ColdBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetAmmoCount(int32 Current, int32 Max)
{
	if (AmmoText)
	{
		AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max)));
	}
}

void USEEHUDWidget::SetWeaponDurability(float Percent)
{
	if (DurabilityBar)
	{
		DurabilityBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetDetectionLevel(float Level)
{
	if (DetectionBar)
	{
		DetectionBar->SetPercent(FMath::Clamp(Level, 0.f, 1.f));
	}
}

void USEEHUDWidget::SetCompassHeading(float Yaw)
{
	if (CompassText)
	{
		// Normalize to 0-360
		float Heading = FMath::Fmod(Yaw + 360.f, 360.f);
		FString Direction;
		if (Heading >= 337.5f || Heading < 22.5f)
			Direction = TEXT("N");
		else if (Heading < 67.5f)
			Direction = TEXT("NE");
		else if (Heading < 112.5f)
			Direction = TEXT("E");
		else if (Heading < 157.5f)
			Direction = TEXT("SE");
		else if (Heading < 202.5f)
			Direction = TEXT("S");
		else if (Heading < 247.5f)
			Direction = TEXT("SW");
		else if (Heading < 292.5f)
			Direction = TEXT("W");
		else
			Direction = TEXT("NW");

		CompassText->SetText(FText::FromString(FString::Printf(TEXT("%s %.0f°"), *Direction, Heading)));
	}
}

void USEEHUDWidget::ShowInteractionPrompt(const FText& Text)
{
	if (InteractionText)
	{
		InteractionText->SetText(Text);
	}
	if (InteractionPanel)
	{
		InteractionPanel->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void USEEHUDWidget::HideInteractionPrompt()
{
	if (InteractionPanel)
	{
		InteractionPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USEEHUDWidget::ToggleInventory()
{
	bInventoryVisible = !bInventoryVisible;
	if (InventoryWidget)
	{
		InventoryWidget->SetVisibility(bInventoryVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void USEEHUDWidget::ToggleTrainMap()
{
	bTrainMapVisible = !bTrainMapVisible;
	if (TrainMapWidget)
	{
		TrainMapWidget->SetVisibility(bTrainMapVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
