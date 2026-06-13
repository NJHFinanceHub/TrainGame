// SEEUISubsystem.h - Game UI manager: owns the Slate screen widgets, input mode switching
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SnowpiercerEE/SEEDialogueManager.h"
#include "SEEUISubsystem.generated.h"

class APlayerController;
class APawn;
class SWidget;
class SSEEDialoguePanel;
class SSEEInventoryScreen;
class USEEInventoryComponent;

UENUM(BlueprintType)
enum class ESEEUIScreen : uint8
{
	None		UMETA(DisplayName = "None"),
	Inventory	UMETA(DisplayName = "Inventory"),
	Character	UMETA(DisplayName = "Character"),
	QuestLog	UMETA(DisplayName = "Quest Log"),
	TrainMap	UMETA(DisplayName = "Train Map"),
	Factions	UMETA(DisplayName = "Factions"),
	Companions	UMETA(DisplayName = "Companions"),
	Crafting	UMETA(DisplayName = "Crafting"),
	Codex		UMETA(DisplayName = "Codex"),
	PauseMenu	UMETA(DisplayName = "Pause Menu"),
	MainMenu	UMETA(DisplayName = "Main Menu"),
	DeathScreen	UMETA(DisplayName = "Death Screen"),
	Dialogue	UMETA(DisplayName = "Dialogue")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScreenChanged, ESEEUIScreen, NewScreen);

/**
 * USEEUISubsystem
 *
 * Single owner of all full-screen Slate UI. OpenScreen() constructs the matching
 * SSEE* widget, adds it to the game viewport and switches the player controller
 * to game-and-UI input with a visible cursor. CloseCurrentScreen() removes the
 * widget and restores game-only input.
 *
 * Also drives the once-per-session main menu (shown paused at boot), the pause
 * menu actions (save/load/settings/quit) and the death screen (reload checkpoint
 * / quit to menu).
 */
UCLASS()
class SNOWPIERCEREE_API USEEUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenScreen(ESEEUIScreen Screen);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void CloseCurrentScreen();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ToggleScreen(ESEEUIScreen Screen);

	UFUNCTION(BlueprintPure, Category = "UI")
	ESEEUIScreen GetCurrentScreen() const { return CurrentScreen; }

	UFUNCTION(BlueprintPure, Category = "UI")
	bool IsScreenOpen() const { return CurrentScreen != ESEEUIScreen::None; }

	/** Show the main menu (paused) the first time a map loads this session.
	 *  Called from ASEEHUD::BeginPlay; subsequent respawns/level loads skip it
	 *  unless QuitToMenu reset the flag. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowMainMenuIfFirstBoot();

	/** Open the death screen with a cause-of-death line. Safe to call repeatedly. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void NotifyPlayerDeath(const FText& Cause);

	// --- Co-op host/join (iteration 1) ---

	/** Host: travel into Zone1_Tail as a listen server, then drop the menu. */
	UFUNCTION(BlueprintCallable, Category = "Net")
	void OpenHostGame();

	/** Join: client-travel to the host at the given IP (blank -> 127.0.0.1). */
	UFUNCTION(BlueprintCallable, Category = "Net")
	void OpenJoinGame(const FString& IP);

	/** Start a conversation with an NPC pawn (uses the entry node stored on its
	 *  ASEENPCAIController brain) and open the dialogue panel over gameplay. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenDialogue(APawn* NPCPawn);

	UPROPERTY(BlueprintAssignable, Category = "UI")
	FOnScreenChanged OnScreenChanged;

private:
	// --- Widget lifecycle ---
	TSharedPtr<SWidget> BuildScreenWidget(ESEEUIScreen Screen);
	void RemoveActiveScreenWidget();
	void ApplyOpenInputMode(const TSharedPtr<SWidget>& FocusWidget);
	void ApplyClosedInputMode();
	static int32 GetScreenZOrder(ESEEUIScreen Screen);

	// --- Settings overlay (shared by main menu and pause menu) ---
	void PushSettingsOverlay();
	void PopSettingsOverlay();

	// --- Menu actions ---
	void HandleMainMenuNewGame();
	void HandleMainMenuContinue();
	void HandleMainMenuHostGame();
	void HandleMainMenuJoinGame(const FString& Address);
	void HandleQuitToDesktop();
	void HandlePauseResume();
	void HandlePauseSave();
	void HandlePauseLoad();
	void HandleQuitToMainMenu();
	void HandleDeathReloadCheckpoint();

	/** Dynamic-delegate target for USEEInventoryComponent::OnInventoryChanged. */
	UFUNCTION()
	void HandlePlayerInventoryChanged();

	// --- Dialogue plumbing ---

	/** Dynamic-delegate target for USEEDialogueManager::OnDialogueNodeChanged. */
	UFUNCTION()
	void HandleDialogueNodeChanged(const FSEEDialogueNode& CurrentNode);

	/** Dynamic-delegate target for USEEDialogueManager::OnDialogueEnded. */
	UFUNCTION()
	void HandleDialogueEnded();

	void HandleDialogueChoiceSelected(FName ChoiceID);
	void HandleDialogueContinue();
	void HandleDialogueCloseRequested();
	void PushNodeToDialoguePanel(const FSEEDialogueNode& Node);
	void CleanupDialogueState();
	USEEDialogueManager* GetDialogueManager() const;

	void HandleWorldTearDown(UWorld* World);

	// --- Helpers ---
	UWorld* GetGameWorld() const;
	APlayerController* GetLocalPlayerController() const;
	APawn* GetLocalPawn() const;
	bool DoesSaveGameExistOnDisk() const;

	ESEEUIScreen CurrentScreen = ESEEUIScreen::None;
	bool bMainMenuShownThisSession = false;
	FText PendingDeathCause;

	TSharedPtr<SWidget> ActiveScreenWidget;
	TSharedPtr<SSEEInventoryScreen> InventoryScreenWidget;
	TSharedPtr<SWidget> SettingsOverlayWidget;
	TSharedPtr<SSEEDialoguePanel> DialoguePanelWidget;

	/** NPC the open dialogue is bound to (its brain pauses while talking). */
	TWeakObjectPtr<APawn> DialogueNPCPawn;

	/** Last NPC speaker/text shown, re-used as context behind choice nodes. */
	FText LastDialogueSpeaker;
	FText LastDialogueText;
	bool bDialogueDelegatesBound = false;

	/** Inventory component the open inventory screen is bound to (for delegate cleanup). */
	UPROPERTY()
	TWeakObjectPtr<USEEInventoryComponent> BoundInventoryComp;

	FDelegateHandle WorldTearDownHandle;
};
