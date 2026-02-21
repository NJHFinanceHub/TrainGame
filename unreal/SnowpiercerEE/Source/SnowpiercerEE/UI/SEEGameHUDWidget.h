// SEEGameHUDWidget.h - Main HUD overlay: health, stamina, hunger, cold, weapon, crosshair
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEGameHUDWidget.generated.h"

class USEEHealthComponent;
class USEEHungerComponent;
class USEEColdComponent;
class USEECombatComponent;
class USEEStatsComponent;
class USEEInventoryComponent;
class ASEECharacter;
class UProgressBar;
class UTextBlock;
class UImage;
class UCanvasPanel;
class UOverlay;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeHUD(ASEECharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractionPrompt(const FText& Prompt, bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowDamageDirection(FVector WorldDirection);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowNotification(const FText& Message, float Duration = 3.0f);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- Health ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InjuryText;

	// --- Stamina ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> StaminaBar;

	// --- Hunger ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HungerBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HungerText;

	// --- Cold ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ColdText;

	// --- Weapon ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WeaponNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> DurabilityBar;

	// --- Interaction ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InteractionText;

	// --- Crosshair ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairImage;

	// --- Damage Vignette ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageVignetteLeft;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageVignetteRight;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageVignetteTop;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> DamageVignetteBottom;

	// --- Notifications ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NotificationText;

	// --- Quick Slots ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuickSlot1Text;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuickSlot2Text;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuickSlot3Text;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> QuickSlot4Text;

	// --- XP / Level ---
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> XPBar;

	// --- Blueprint implementable events ---
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnHUDInitialized();

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void OnNotificationReceived(const FText& Message);

private:
	void UpdateHealthDisplay();
	void UpdateStaminaDisplay();
	void UpdateHungerDisplay();
	void UpdateColdDisplay();
	void UpdateWeaponDisplay();
	void UpdateXPDisplay();
	void UpdateDamageVignette(float DeltaTime);

	TWeakObjectPtr<ASEECharacter> CachedCharacter;
	TWeakObjectPtr<USEEHealthComponent> HealthComp;
	TWeakObjectPtr<USEEHungerComponent> HungerComp;
	TWeakObjectPtr<USEEColdComponent> ColdComp;
	TWeakObjectPtr<USEECombatComponent> CombatComp;
	TWeakObjectPtr<USEEStatsComponent> StatsComp;

	float DamageVignetteAlpha = 0.0f;
	FVector LastDamageDir = FVector::ZeroVector;
	float NotificationTimer = 0.0f;
};
