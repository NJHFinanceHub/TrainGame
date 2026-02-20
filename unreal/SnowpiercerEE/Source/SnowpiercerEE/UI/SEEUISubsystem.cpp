// SEEUISubsystem.cpp
#include "SEEUISubsystem.h"

void USEEUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentScreen = ESEEUIScreen::None;
}

void USEEUISubsystem::OpenScreen(ESEEUIScreen Screen)
{
	if (CurrentScreen == Screen) return;

	CurrentScreen = Screen;
	OnScreenChanged.Broadcast(CurrentScreen);
}

void USEEUISubsystem::CloseCurrentScreen()
{
	if (CurrentScreen == ESEEUIScreen::None) return;

	CurrentScreen = ESEEUIScreen::None;
	OnScreenChanged.Broadcast(CurrentScreen);
}

void USEEUISubsystem::ToggleScreen(ESEEUIScreen Screen)
{
	if (CurrentScreen == Screen)
	{
		CloseCurrentScreen();
	}
	else
	{
		OpenScreen(Screen);
	}
}
