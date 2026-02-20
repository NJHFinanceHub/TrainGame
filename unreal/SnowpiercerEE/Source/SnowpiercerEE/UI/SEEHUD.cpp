#include "SEEHUD.h"
#include "SEEHUDWidget.h"
#include "SEEInventoryWidget.h"
#include "SEETrainMapWidget.h"
#include "Blueprint/UserWidget.h"

ASEEHUD::ASEEHUD()
{
}

void ASEEHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<USEEHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}

void ASEEHUD::ShowInteractionPrompt(const FText& Text)
{
	if (HUDWidget)
	{
		HUDWidget->ShowInteractionPrompt(Text);
	}
}

void ASEEHUD::HideInteractionPrompt()
{
	if (HUDWidget)
	{
		HUDWidget->HideInteractionPrompt();
	}
}

void ASEEHUD::ToggleInventory()
{
	if (HUDWidget)
	{
		HUDWidget->ToggleInventory();
	}
}

void ASEEHUD::ToggleTrainMap()
{
	if (HUDWidget)
	{
		HUDWidget->ToggleTrainMap();
	}
}
