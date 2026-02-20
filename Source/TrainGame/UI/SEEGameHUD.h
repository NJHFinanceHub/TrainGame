// SEEGameHUD.h - Main HUD class that owns and manages all UI widgets
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SEEHUDTypes.h"
#include "SEEGameHUD.generated.h"

class SSEESurvivalBars;
class SSEECombatOverlay;
class SSEEInventoryScreen;
class SSEEDialoguePanel;
class SSEETrainMap;
class SSEECraftingPanel;
class SSEEFactionPanel;
class SSEECharacterScreen;
class SSEEQuestLog;
class SSEECompanionPanel;
class SSEECodexPanel;
class SSEEMainMenu;
class SSEEPauseMenu;
class SSEEDeathScreen;

class USurvivalComponent;
class UCombatComponent;
class UWeaponComponent;
class UInventoryComponent;
class UCraftingComponent;
class UKronoleComponent;
class USEEStatsComponent;
class USEEQuestManager;
class UCompanionRosterSubsystem;
class UCollectibleJournalSubsystem;
class USEECarStreamingSubsystem;

/**
 * ASEEGameHUD
 *
 * Central HUD manager for Snowpiercer: Eternal Engine.
 * Creates and manages all Slate-based UI panels.
 * Binds to gameplay components on the player pawn via delegates.
 *
 * Panel visibility is managed here - inventory, crafting, train map,
 * faction, character, quest, companion, and codex panels are toggle-able
 * overlays. Survival bars and combat overlay are always visible during
 * gameplay. Main menu, pause menu, and death screen are special overlays.
 */
UCLASS()
class TRAINGAME_API ASEEGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Panel Toggles ---

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleCrafting();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleTrainMap();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleFactionPanel();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleCharacter();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleQuestLog();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleCompanionPanel();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ToggleCodex();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void CloseAllPanels();

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsAnyPanelOpen() const;

	// --- Dialogue ---

	UFUNCTION(BlueprintCallable, Category = "HUD|Dialogue")
	void ShowDialogue(const FDialogueLine& Line);

	UFUNCTION(BlueprintCallable, Category = "HUD|Dialogue")
	void HideDialogue();

	UFUNCTION(BlueprintPure, Category = "HUD|Dialogue")
	bool IsDialogueActive() const;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Dialogue")
	FOnDialogueChoiceSelected OnDialogueChoiceSelected;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Dialogue")
	FOnDialogueDismissed OnDialogueDismissed;

	// --- Faction Reputation ---

	UFUNCTION(BlueprintCallable, Category = "HUD|Faction")
	void SetFactionReputations(const TArray<FFactionReputation>& Reputations);

	// --- Pause Menu ---

	UFUNCTION(BlueprintCallable, Category = "HUD|Pause")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "HUD|Pause")
	void HidePauseMenu();

	UFUNCTION(BlueprintPure, Category = "HUD|Pause")
	bool IsPauseMenuVisible() const;

	// --- Death Screen ---

	UFUNCTION(BlueprintCallable, Category = "HUD|Death")
	void ShowDeathScreen(const FText& DeathMessage);

	UFUNCTION(BlueprintCallable, Category = "HUD|Death")
	void HideDeathScreen();

	// --- Death/Pause Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "HUD|Death")
	FOnHUDAction OnReloadCheckpoint;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Death")
	FOnHUDAction OnQuitToMenu;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Pause")
	FOnHUDAction OnSaveGame;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Pause")
	FOnHUDAction OnLoadGame;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Pause")
	FOnHUDAction OnOpenSettings;

protected:
	// Find and cache component references from the player pawn
	void BindToPlayerPawn();
	void UnbindFromPlayerPawn();

	// Build all Slate widgets and add to viewport
	void CreateWidgets();
	void RemoveWidgets();

	// Toggle a panel's visibility, closing others if needed
	void SetPanelVisible(TSharedPtr<SWidget> Panel, bool bVisible);

	// Cached component references
	UPROPERTY()
	TWeakObjectPtr<USurvivalComponent> SurvivalComp;

	UPROPERTY()
	TWeakObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY()
	TWeakObjectPtr<UWeaponComponent> WeaponComp;

	UPROPERTY()
	TWeakObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TWeakObjectPtr<UCraftingComponent> CraftingComp;

	UPROPERTY()
	TWeakObjectPtr<UKronoleComponent> KronoleComp;

	UPROPERTY()
	TWeakObjectPtr<USEEStatsComponent> StatsComp;

private:
	// Slate widget references
	TSharedPtr<SSEESurvivalBars> SurvivalBarsWidget;
	TSharedPtr<SSEECombatOverlay> CombatOverlayWidget;
	TSharedPtr<SSEEInventoryScreen> InventoryWidget;
	TSharedPtr<SSEEDialoguePanel> DialogueWidget;
	TSharedPtr<SSEETrainMap> TrainMapWidget;
	TSharedPtr<SSEECraftingPanel> CraftingWidget;
	TSharedPtr<SSEEFactionPanel> FactionWidget;
	TSharedPtr<SSEECharacterScreen> CharacterWidget;
	TSharedPtr<SSEEQuestLog> QuestLogWidget;
	TSharedPtr<SSEECompanionPanel> CompanionWidget;
	TSharedPtr<SSEECodexPanel> CodexWidget;
	TSharedPtr<SSEEPauseMenu> PauseMenuWidget;
	TSharedPtr<SSEEDeathScreen> DeathScreenWidget;

	// Track which full-screen panel is open (only one at a time)
	TSharedPtr<SWidget> ActivePanel;

	bool bDialogueActive = false;
};
