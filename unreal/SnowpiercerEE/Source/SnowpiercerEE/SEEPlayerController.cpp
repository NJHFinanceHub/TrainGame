#include "SEEPlayerController.h"
#include "UI/SEEUISubsystem.h"
#include "Kismet/GameplayStatics.h"

void ASEEPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!InputComponent) return;

    InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &ASEEPlayerController::ToggleInventory);
    InputComponent->BindAction("ToggleCharacter", IE_Pressed, this, &ASEEPlayerController::ToggleCharacterScreen);
    InputComponent->BindAction("ToggleQuestLog", IE_Pressed, this, &ASEEPlayerController::ToggleQuestLog);
    InputComponent->BindAction("ToggleMap", IE_Pressed, this, &ASEEPlayerController::ToggleTrainMap);
    InputComponent->BindAction("ToggleFactions", IE_Pressed, this, &ASEEPlayerController::ToggleFactions);
    InputComponent->BindAction("ToggleCompanions", IE_Pressed, this, &ASEEPlayerController::ToggleCompanions);
    InputComponent->BindAction("ToggleCrafting", IE_Pressed, this, &ASEEPlayerController::ToggleCrafting);
    InputComponent->BindAction("ToggleCodex", IE_Pressed, this, &ASEEPlayerController::ToggleCodex);
    InputComponent->BindAction("PauseMenu", IE_Pressed, this, &ASEEPlayerController::TogglePauseMenu);
    InputComponent->BindAction("CloseUI", IE_Pressed, this, &ASEEPlayerController::CloseCurrentUI);
}

void ASEEPlayerController::ToggleViewMode()
{
    const ESEEViewMode NextMode =
        (ViewMode == ESEEViewMode::FirstPerson) ? ESEEViewMode::ThirdPerson : ESEEViewMode::FirstPerson;
    SetViewMode(NextMode);
}

void ASEEPlayerController::SetViewMode(ESEEViewMode NewMode)
{
    if (ViewMode == NewMode)
    {
        return;
    }

    ViewMode = NewMode;
    OnViewModeChanged.Broadcast(ViewMode);
}

USEEUISubsystem* ASEEPlayerController::GetUISubsystem() const
{
    UGameInstance* GI = GetGameInstance();
    return GI ? GI->GetSubsystem<USEEUISubsystem>() : nullptr;
}

void ASEEPlayerController::ToggleInventory()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Inventory);
}

void ASEEPlayerController::ToggleCharacterScreen()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Character);
}

void ASEEPlayerController::ToggleQuestLog()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::QuestLog);
}

void ASEEPlayerController::ToggleTrainMap()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::TrainMap);
}

void ASEEPlayerController::ToggleFactions()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Factions);
}

void ASEEPlayerController::ToggleCompanions()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Companions);
}

void ASEEPlayerController::ToggleCrafting()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Crafting);
}

void ASEEPlayerController::ToggleCodex()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI) UI->ToggleScreen(ESEEUIScreen::Codex);
}

void ASEEPlayerController::TogglePauseMenu()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (!UI) return;

    if (UI->GetCurrentScreen() == ESEEUIScreen::PauseMenu)
    {
        UI->CloseCurrentScreen();
        UGameplayStatics::SetGamePaused(this, false);
    }
    else
    {
        UI->OpenScreen(ESEEUIScreen::PauseMenu);
        UGameplayStatics::SetGamePaused(this, true);
    }
}

void ASEEPlayerController::CloseCurrentUI()
{
    USEEUISubsystem* UI = GetUISubsystem();
    if (UI && UI->IsScreenOpen())
    {
        if (UI->GetCurrentScreen() == ESEEUIScreen::PauseMenu)
        {
            UGameplayStatics::SetGamePaused(this, false);
        }
        UI->CloseCurrentScreen();
    }
}
