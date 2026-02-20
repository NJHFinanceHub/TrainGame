#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SEEHUD.generated.h"

class USEEHUDWidget;

UCLASS()
class SNOWPIERCEREE_API ASEEHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASEEHUD();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="UI")
	USEEHUDWidget* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(BlueprintCallable, Category="UI")
	void ShowInteractionPrompt(const FText& Text);

	UFUNCTION(BlueprintCallable, Category="UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category="UI")
	void ToggleTrainMap();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="UI")
	TSubclassOf<USEEHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category="UI")
	TObjectPtr<USEEHUDWidget> HUDWidget;
};
