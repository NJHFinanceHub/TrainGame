// SEEPauseMenuWidget.h - Pause menu: resume, save, load, settings, quit
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEPauseMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnResumeClicked();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnSaveClicked();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnLoadClicked();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnQuitToMenuClicked();

	UFUNCTION(BlueprintCallable, Category = "PauseMenu")
	void OnQuitToDesktopClicked();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PauseTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SaveButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> LoadButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitToMenuButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitToDesktopButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PauseMenu")
	TSoftObjectPtr<UWorld> MainMenuMap;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SaveStatusText;

	UFUNCTION(BlueprintImplementableEvent, Category = "PauseMenu")
	void OnResumeRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "PauseMenu")
	void OnSettingsRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "PauseMenu")
	void OnSaveCompleted(bool bSuccess);
};
