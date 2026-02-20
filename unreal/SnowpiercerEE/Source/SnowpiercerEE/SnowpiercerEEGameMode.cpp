#include "SnowpiercerEEGameMode.h"
#include "SEEPlayerController.h"
#include "SEECharacter.h"
#include "SEEHUD.h"

ASnowpiercerEEGameMode::ASnowpiercerEEGameMode()
{
    PlayerControllerClass = ASEEPlayerController::StaticClass();
    DefaultPawnClass = ASEECharacter::StaticClass();
    HUDClass = ASEEHUD::StaticClass();
}
