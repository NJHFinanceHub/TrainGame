#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEHUDWidget.generated.h"

class USEEInventoryWidget;
class USEETrainMapWidget;

UCLASS(Abstract)
class SNOWPIERCEREE_API USEEHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Status bars (0.0 - 1.0)
	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetHealthPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetStaminaPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetHungerPercent(float Percent);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetColdPercent(float Percent);

	// Equipment info
	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetAmmoCount(int32 Current, int32 Max);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetWeaponDurability(float Percent);

	// Detection
	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetDetectionLevel(float Level);

	// Compass
	UFUNCTION(BlueprintCallable, Category="HUD")
	void SetCompassHeading(float Yaw);

	// Interaction prompts
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ShowInteractionPrompt(const FText& Text);

	UFUNCTION(BlueprintCallable, Category="HUD")
	void HideInteractionPrompt();

	// Sub-panels
	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category="HUD")
	void ToggleTrainMap();

	UFUNCTION(BlueprintPure, Category="HUD")
	bool IsInventoryVisible() const { return bInventoryVisible; }

	UFUNCTION(BlueprintPure, Category="HUD")
	bool IsTrainMapVisible() const { return bTrainMapVisible; }

protected:
	virtual void NativeConstruct() override;

	// Bindable widget refs set in Blueprint
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> HealthBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> StaminaBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> HungerBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> ColdBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> DetectionBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UProgressBar> DurabilityBar;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UTextBlock> AmmoText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UTextBlock> CompassText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UTextBlock> InteractionText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<class UWidget> InteractionPanel;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<USEEInventoryWidget> InventoryWidget;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="HUD")
	TObjectPtr<USEETrainMapWidget> TrainMapWidget;

private:
	bool bInventoryVisible = false;
	bool bTrainMapVisible = false;
};
