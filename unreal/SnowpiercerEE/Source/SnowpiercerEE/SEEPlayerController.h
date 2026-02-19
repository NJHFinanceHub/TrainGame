#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SEEPlayerController.generated.h"

UENUM(BlueprintType)
enum class ESEEViewMode : uint8
{
    FirstPerson UMETA(DisplayName="First Person"),
    ThirdPerson UMETA(DisplayName="Third Person")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSEEViewModeChanged, ESEEViewMode, NewMode);

UCLASS()
class SNOWPIERCEREE_API ASEEPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Camera")
    void ToggleViewMode();

    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetViewMode(ESEEViewMode NewMode);

    UFUNCTION(BlueprintPure, Category="Camera")
    ESEEViewMode GetViewMode() const { return ViewMode; }

    UPROPERTY(BlueprintAssignable, Category="Camera")
    FOnSEEViewModeChanged OnViewModeChanged;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
    ESEEViewMode ViewMode = ESEEViewMode::ThirdPerson;
};
