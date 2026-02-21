// SEEMainMenuWidget.h - Main menu: new game, continue, settings, credits
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEMainMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnNewGameClicked();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnContinueClicked();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnSettingsClicked();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnCreditsClicked();

	UFUNCTION(BlueprintCallable, Category = "MainMenu")
	void OnQuitClicked();

	UFUNCTION(BlueprintPure, Category = "MainMenu")
	bool HasSaveData() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> VersionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> NewGameButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> ContinueButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> SettingsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> CreditsButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> QuitButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MainMenu")
	TSoftObjectPtr<UWorld> NewGameMap;

	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void OnSettingsRequested();

	UFUNCTION(BlueprintImplementableEvent, Category = "MainMenu")
	void OnCreditsRequested();
};
