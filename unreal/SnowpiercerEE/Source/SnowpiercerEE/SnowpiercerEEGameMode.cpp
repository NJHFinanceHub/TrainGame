#include "SnowpiercerEEGameMode.h"
#include "SEEPlayerController.h"
#include "SEECharacter.h"

ASnowpiercerEEGameMode::ASnowpiercerEEGameMode()
{
    PlayerControllerClass = ASEEPlayerController::StaticClass();
    DefaultPawnClass = ASEECharacter::StaticClass();
}
