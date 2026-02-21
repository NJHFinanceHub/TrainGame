// SEEDeathScreenWidget.h - Death screen: reload checkpoint / quit options
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEDeathScreenWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Death")
	void ShowDeathScreen(const FText& CauseOfDeath);

	UFUNCTION(BlueprintCallable, Category = "Death")
	void OnReloadCheckpointClicked();

	UFUNCTION(BlueprintCallable, Category = "Death")
	void OnQuitToMenuClicked();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DeathTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CauseOfDeathText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ReloadButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death")
	TSoftObjectPtr<UWorld> MainMenuMap;

	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void OnDeathScreenShown(const FText& CauseOfDeath);

	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void OnReloadRequested();
};
