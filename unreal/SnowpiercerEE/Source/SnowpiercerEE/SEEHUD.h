#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UI/SEEUISubsystem.h"
#include "SEEHUD.generated.h"

class USEEHealthComponent;
class USEEHungerComponent;
class USEEColdComponent;
class USEECombatComponent;
class ASEECharacter;
class USEEGameHUDWidget;
class USEEInventoryWidget;
class USEECharacterWidget;
// NOTE: Quest log is now SSEEQuestLogWidget (Slate). UMG placeholder uses UUserWidget.
class USEETrainMapWidget;
class USEEFactionWidget;
class USEECompanionWidget;
class USEECraftingWidget;
class USEECodexWidget;
class USEEDialogueWidget;
class USEEPauseMenuWidget;
class USEEDeathScreenWidget;

UCLASS()
class SNOWPIERCEREE_API ASEEHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASEEHUD();

	virtual void DrawHUD() override;

	// --- UMG Widget Accessors ---

	UFUNCTION(BlueprintPure, Category = "HUD")
	USEEGameHUDWidget* GetGameHUD() const { return GameHUDWidget; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	USEEDialogueWidget* GetDialogueWidget() const { return DialogueWidget; }

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowDialogue();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideDialogue();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowDeathScreen(const FText& CauseOfDeath);

protected:
	virtual void BeginPlay() override;

	// Widget classes (set in Blueprint or C++ subclass)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEGameHUDWidget> GameHUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEInventoryWidget> InventoryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEECharacterWidget> CharacterWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<UUserWidget> QuestLogWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEETrainMapWidget> TrainMapWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEFactionWidget> FactionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEECompanionWidget> CompanionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEECraftingWidget> CraftingWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEECodexWidget> CodexWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEDialogueWidget> DialogueWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD|Widgets")
	TSubclassOf<USEEDeathScreenWidget> DeathScreenWidgetClass;

private:
	void OnScreenChanged(ESEEUIScreen NewScreen);
	void ShowScreenWidget(ESEEUIScreen Screen);
	void HideAllScreenWidgets();
	UUserWidget* GetOrCreateWidget(ESEEUIScreen Screen);
	void SetGameInputMode(bool bGameOnly);

	// Canvas HUD fallback (used when UMG not configured)
	void DrawHealthBar();
	void DrawStaminaBar();
	void DrawHungerIndicator();
	void DrawColdIndicator();
	void DrawWeaponIndicator();
	void DrawInteractionPrompt();
	void DrawDetectionIndicator();
	void DrawCrosshair();
	void DrawDamageDirection();

	void DrawBar(float X, float Y, float Width, float Height, float Percent,
				 FLinearColor FillColor, FLinearColor BackColor);

	// Cached references
	UPROPERTY()
	TWeakObjectPtr<ASEECharacter> PlayerCharacter;

	UPROPERTY()
	TWeakObjectPtr<USEEHealthComponent> HealthComp;

	UPROPERTY()
	TWeakObjectPtr<USEEHungerComponent> HungerComp;

	UPROPERTY()
	TWeakObjectPtr<USEEColdComponent> ColdComp;

	UPROPERTY()
	TWeakObjectPtr<USEECombatComponent> CombatComp;

	// UMG widget instances
	UPROPERTY()
	TObjectPtr<USEEGameHUDWidget> GameHUDWidget;

	UPROPERTY()
	TObjectPtr<USEEInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<USEECharacterWidget> CharacterWidget;

	UPROPERTY()
	TObjectPtr<UUserWidget> QuestLogWidget;

	UPROPERTY()
	TObjectPtr<USEETrainMapWidget> TrainMapWidget;

	UPROPERTY()
	TObjectPtr<USEEFactionWidget> FactionWidget;

	UPROPERTY()
	TObjectPtr<USEECompanionWidget> CompanionWidget;

	UPROPERTY()
	TObjectPtr<USEECraftingWidget> CraftingWidget;

	UPROPERTY()
	TObjectPtr<USEECodexWidget> CodexWidget;

	UPROPERTY()
	TObjectPtr<USEEDialogueWidget> DialogueWidget;

	UPROPERTY()
	TObjectPtr<USEEPauseMenuWidget> PauseMenuWidget;

	UPROPERTY()
	TObjectPtr<USEEDeathScreenWidget> DeathScreenWidget;

	// Damage direction tracking
	float LastDamageTime = 0.0f;
	FVector LastDamageDirection = FVector::ZeroVector;

	bool bUsingUMG = false;
};
