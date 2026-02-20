#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/SEEUISubsystem.h"
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
    // Camera
    UFUNCTION(BlueprintCallable, Category="Camera")
    void ToggleViewMode();

    UFUNCTION(BlueprintCallable, Category="Camera")
    void SetViewMode(ESEEViewMode NewMode);

    UFUNCTION(BlueprintPure, Category="Camera")
    ESEEViewMode GetViewMode() const { return ViewMode; }

    UPROPERTY(BlueprintAssignable, Category="Camera")
    FOnSEEViewModeChanged OnViewModeChanged;

    // UI Screen Management
    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleInventory();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleCharacterScreen();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleQuestLog();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleTrainMap();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleFactions();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleCompanions();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleCrafting();

    UFUNCTION(BlueprintCallable, Category="UI")
    void ToggleCodex();

    UFUNCTION(BlueprintCallable, Category="UI")
    void TogglePauseMenu();

    UFUNCTION(BlueprintCallable, Category="UI")
    void CloseCurrentUI();

protected:
    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
    ESEEViewMode ViewMode = ESEEViewMode::ThirdPerson;

private:
    USEEUISubsystem* GetUISubsystem() const;
};
