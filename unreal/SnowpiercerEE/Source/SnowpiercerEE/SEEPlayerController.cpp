#include "SEEPlayerController.h"

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
