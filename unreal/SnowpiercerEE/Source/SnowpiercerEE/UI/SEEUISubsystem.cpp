// SEEUISubsystem.cpp - Slate screen management implementation
#include "SEEUISubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Framework/Application/SlateApplication.h"

// Game data sources
#include "SnowpiercerEE/AI/SEENPCAIController.h"
#include "SnowpiercerEE/AI/SEENPCDialogueComponent.h"
#include "SnowpiercerEE/SEECharacter.h"
#include "SnowpiercerEE/SEEInventoryComponent.h"
#include "SnowpiercerEE/SEEHealthComponent.h"
#include "SnowpiercerEE/SEEStatsComponent.h"
#include "SnowpiercerEE/SEEQuestManager.h"
#include "SnowpiercerEE/SEEFactionManager.h"
#include "SnowpiercerEE/SEESaveGameSubsystem.h"
#include "SnowpiercerEE/SEETypes.h"
#include "SnowpiercerEE/SnowpiercerEEGameMode.h"
#include "SnowpiercerEE/Net/SEENetTravel.h"
#include "TrainGame/Companions/CompanionRosterSubsystem.h"
#include "TrainGame/UI/SEEHUDTypes.h"
#include "SnowyEngine/Crafting/CraftingComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"

// Slate screens
#include "SSEEInventoryScreen.h"
#include "SSEECharacterScreen.h"
#include "SSEEQuestLog.h"
#include "SSEETrainMap.h"
#include "SSEEFactionPanel.h"
#include "SSEECompanionScreen.h"
#include "SSEECodexPanel.h"
#include "SSEECraftingPanel.h"
#include "SSEEPauseMenu.h"
#include "SSEEMainMenu.h"
#include "SSEEDeathScreen.h"
#include "SSEEDialoguePanel.h"
#include "Widgets/SSEESettingsPanel.h"

namespace
{
	// Mirrors USEESaveGameSubsystem's private slot name (no public accessor).
	const TCHAR* GSEESaveSlotName = TEXT("SnowpiercerEE_Main");
	constexpr int32 GSEESaveUserIndex = 0;
}

void USEEUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentScreen = ESEEUIScreen::None;
	bMainMenuShownThisSession = false;

	// Tear widgets down before a level transition destroys the actors they read.
	WorldTearDownHandle = FWorldDelegates::OnWorldBeginTearDown.AddUObject(
		this, &USEEUISubsystem::HandleWorldTearDown);
}

void USEEUISubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldBeginTearDown.Remove(WorldTearDownHandle);
	PopSettingsOverlay();
	RemoveActiveScreenWidget();
	Super::Deinitialize();
}

// --- Public API ---

void USEEUISubsystem::OpenScreen(ESEEUIScreen Screen)
{
	if (CurrentScreen == Screen || Screen == ESEEUIScreen::None) return;
	if (!GEngine || !GEngine->GameViewport) return;

	PopSettingsOverlay();
	RemoveActiveScreenWidget();

	TSharedPtr<SWidget> Widget = BuildScreenWidget(Screen);
	if (!Widget.IsValid())
	{
		// Nothing to show; restore gameplay input if we just tore a screen down.
		if (CurrentScreen != ESEEUIScreen::None)
		{
			CurrentScreen = ESEEUIScreen::None;
			ApplyClosedInputMode();
			OnScreenChanged.Broadcast(CurrentScreen);
		}
		return;
	}

	ActiveScreenWidget = Widget;
	GEngine->GameViewport->AddViewportWidgetContent(Widget.ToSharedRef(), GetScreenZOrder(Screen));

	CurrentScreen = Screen;
	ApplyOpenInputMode(Widget);

	// The main menu pauses the world (pause menu pausing is handled by the
	// player controller's TogglePauseMenu, which owns the Escape flow).
	if (Screen == ESEEUIScreen::MainMenu)
	{
		UGameplayStatics::SetGamePaused(GetGameWorld(), true);
	}

	OnScreenChanged.Broadcast(CurrentScreen);
}

void USEEUISubsystem::CloseCurrentScreen()
{
	if (CurrentScreen == ESEEUIScreen::None) return;

	const ESEEUIScreen Closing = CurrentScreen;

	PopSettingsOverlay();
	RemoveActiveScreenWidget();

	CurrentScreen = ESEEUIScreen::None;
	ApplyClosedInputMode();

	// Menus that pause the world unpause it on close. (The controller also
	// unpauses on its own pause-menu path; double-unpausing is harmless.)
	if (Closing == ESEEUIScreen::MainMenu || Closing == ESEEUIScreen::PauseMenu)
	{
		UGameplayStatics::SetGamePaused(GetGameWorld(), false);
	}

	OnScreenChanged.Broadcast(CurrentScreen);
}

void USEEUISubsystem::ToggleScreen(ESEEUIScreen Screen)
{
	if (CurrentScreen == Screen)
	{
		CloseCurrentScreen();
	}
	else
	{
		OpenScreen(Screen);
	}
}

void USEEUISubsystem::ShowMainMenuIfFirstBoot()
{
	if (bMainMenuShownThisSession) return;
	if (!GEngine || !GEngine->GameViewport) return;

	bMainMenuShownThisSession = true;
	OpenScreen(ESEEUIScreen::MainMenu);
}

void USEEUISubsystem::NotifyPlayerDeath(const FText& Cause)
{
	if (CurrentScreen == ESEEUIScreen::DeathScreen) return;

	PendingDeathCause = Cause;
	OpenScreen(ESEEUIScreen::DeathScreen);
}

void USEEUISubsystem::OpenDialogue(APawn* NPCPawn)
{
	if (!NPCPawn || CurrentScreen != ESEEUIScreen::None) return;

	USEEDialogueManager* Dialogue = GetDialogueManager();
	if (!Dialogue || Dialogue->IsInConversation()) return;

	// CO-OP: resolve the entry node + talkability from the pawn's REPLICATED
	// dialogue component (present on host, standalone AND clients) instead of the
	// server-only AI controller. This is the single code path that lets guests talk.
	USEENPCDialogueComponent* NPCDialogue = NPCPawn->FindComponentByClass<USEENPCDialogueComponent>();
	if (!NPCDialogue || !NPCDialogue->bReplicatedCanStartDialogue) return;

	// Start the conversation first: BuildScreenWidget pulls the current node. The
	// dialogue DataTable exists on every machine, so the tree renders locally.
	if (!Dialogue->StartConversationAtNode(NPCDialogue->ReplicatedDialogueEntryId)) return;

	DialogueNPCPawn = NPCPawn;

	// Tell the brain to stand still / face us while talking. On host/standalone the
	// pawn IS authority so this applies directly; a guest routes it through a Server
	// RPC on its own pawn so the server-side brain pauses for the conversation.
	if (APawn* LocalPawn = GetLocalPawn())
	{
		if (ASEECharacter* LocalChar = Cast<ASEECharacter>(LocalPawn))
		{
			LocalChar->SetNPCInDialogue(NPCPawn, true);
		}
	}

	OpenScreen(ESEEUIScreen::Dialogue);

	// Panel failed to come up (no viewport?) — don't leave a headless conversation.
	if (CurrentScreen != ESEEUIScreen::Dialogue)
	{
		CleanupDialogueState();
	}
}

// --- Widget factory ---

TSharedPtr<SWidget> USEEUISubsystem::BuildScreenWidget(ESEEUIScreen Screen)
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GetGameWorld();
	APawn* Pawn = GetLocalPawn();

	switch (Screen)
	{
	case ESEEUIScreen::Inventory:
	{
		USEEInventoryComponent* InvComp =
			Pawn ? Pawn->FindComponentByClass<USEEInventoryComponent>() : nullptr;

		TSharedRef<SSEEInventoryScreen> Inventory = SNew(SSEEInventoryScreen)
			.SEEInventoryComponent(InvComp)
			.OnRequestClose(FSimpleDelegate::CreateUObject(this, &USEEUISubsystem::CloseCurrentScreen));

		// Live refresh: re-pull the slot list whenever the component mutates.
		if (InvComp)
		{
			InvComp->OnInventoryChanged.AddDynamic(this, &USEEUISubsystem::HandlePlayerInventoryChanged);
			BoundInventoryComp = InvComp;
		}

		InventoryScreenWidget = Inventory;
		return Inventory;
	}

	case ESEEUIScreen::Character:
	{
		USEEStatsComponent* StatsComp =
			Pawn ? Pawn->FindComponentByClass<USEEStatsComponent>() : nullptr;
		return SNew(SSEECharacterScreen)
			.StatsComponent(StatsComp)
			.InventoryComponent(nullptr); // legacy SnowyEngine inventory not present on the pawn
	}

	case ESEEUIScreen::QuestLog:
	{
		TSharedRef<SSEEQuestLog> QuestLog = SNew(SSEEQuestLog);
		if (GI)
		{
			QuestLog->SetQuestManager(GI->GetSubsystem<USEEQuestManager>());
		}
		return QuestLog;
	}

	case ESEEUIScreen::TrainMap:
	{
		TSharedRef<SSEETrainMap> Map = SNew(SSEETrainMap);

		// Zone 1 (The Tail) car list - mirrors USEECarStreamingSubsystem::RegisterZone1Cars.
		static const TCHAR* Zone1CarNames[] =
		{
			TEXT("Caboose"), TEXT("Tail Quarters A"), TEXT("Tail Quarters B"),
			TEXT("The Pit"), TEXT("Nursery"), TEXT("Elders' Car"),
			TEXT("Sickbay"), TEXT("Workshop"), TEXT("Listening Post"),
			TEXT("Blockade"), TEXT("Dark Car"), TEXT("Freezer Breach"),
			TEXT("Kronole Den"), TEXT("Smugglers' Cache"), TEXT("Martyr's Gate")
		};

		TArray<FName> CarNames;
		TArray<int32> ZoneIndices;
		for (const TCHAR* Name : Zone1CarNames)
		{
			CarNames.Add(FName(Name));
			ZoneIndices.Add(0); // all Zone 1 cars are in the Tail
		}
		Map->SetCarData(CarNames, ZoneIndices);

		ASnowpiercerEEGameMode* GM = World ? World->GetAuthGameMode<ASnowpiercerEEGameMode>() : nullptr;
		USEESaveGameSubsystem* SaveSub = GI ? GI->GetSubsystem<USEESaveGameSubsystem>() : nullptr;
		for (int32 i = 0; i < CarNames.Num(); ++i)
		{
			FSEECarState State;
			const bool bHasState = SaveSub && SaveSub->GetCarState(i, State);
			const bool bVisited = (GM && GM->HasVisitedCar(i)) || (bHasState && State.bVisited);
			const bool bCompleted = bHasState && State.bCompleted;
			Map->UpdateCarState(i, bVisited, bCompleted);
		}
		if (GM)
		{
			Map->SetCurrentCar(GM->GetCurrentCarIndex());
		}
		return Map;
	}

	case ESEEUIScreen::Factions:
	{
		TSharedRef<SSEEFactionPanel> Panel = SNew(SSEEFactionPanel);

		if (USEEFactionManager* Factions = GI ? GI->GetSubsystem<USEEFactionManager>() : nullptr)
		{
			struct FFactionDisplay { ESEEFaction Faction; FText Name; FLinearColor Color; };
			const FFactionDisplay Display[] =
			{
				{ ESEEFaction::Tailies,          NSLOCTEXT("HUD", "FacTailies", "Tailies"),               FLinearColor(0.55f, 0.60f, 0.70f) },
				{ ESEEFaction::ThirdClassUnion,  NSLOCTEXT("HUD", "FacThird",   "Third Class Union"),     FLinearColor(0.75f, 0.50f, 0.25f) },
				{ ESEEFaction::Jackboots,        NSLOCTEXT("HUD", "FacJack",    "Jackboots"),             FLinearColor(0.65f, 0.15f, 0.12f) },
				{ ESEEFaction::Bureaucracy,      NSLOCTEXT("HUD", "FacBureau",  "Bureaucracy"),           FLinearColor(0.70f, 0.65f, 0.45f) },
				{ ESEEFaction::FirstClassElite,  NSLOCTEXT("HUD", "FacFirst",   "First Class Elite"),     FLinearColor(0.85f, 0.72f, 0.38f) },
				{ ESEEFaction::OrderOfTheEngine, NSLOCTEXT("HUD", "FacOrder",   "Order of the Engine"),   FLinearColor(0.55f, 0.75f, 0.95f) },
				{ ESEEFaction::KronoleNetwork,   NSLOCTEXT("HUD", "FacKronole", "Kronole Network"),       FLinearColor(0.60f, 0.30f, 0.80f) },
				{ ESEEFaction::TheThaw,          NSLOCTEXT("HUD", "FacThaw",    "The Thaw"),              FLinearColor(0.30f, 0.75f, 0.65f) },
			};

			TArray<FFactionReputation> Reps;
			for (const FFactionDisplay& Entry : Display)
			{
				FFactionReputation Rep;
				Rep.FactionName = Entry.Name;
				Rep.Reputation = static_cast<float>(Factions->GetReputation(Entry.Faction));
				Rep.FactionColor = Entry.Color;
				Reps.Add(Rep);
			}
			Panel->UpdateReputations(Reps);
		}
		return Panel;
	}

	case ESEEUIScreen::Companions:
	{
		TSharedRef<SSEECompanionScreen> Companions = SNew(SSEECompanionScreen);
		if (GI)
		{
			Companions->SetRosterSubsystem(GI->GetSubsystem<UCompanionRosterSubsystem>());
		}
		return Companions;
	}

	case ESEEUIScreen::Codex:
	{
		TSharedRef<SSEECodexPanel> Codex = SNew(SSEECodexPanel);

		// Baseline lore entries so the codex is functional before pickups exist.
		auto MakeEntry = [](FName ID, const FText& Title, const FText& Content, ECodexCategory Category)
		{
			FCodexEntry Entry;
			Entry.EntryID = ID;
			Entry.Title = Title;
			Entry.Content = Content;
			Entry.Category = Category;
			return Entry;
		};
		Codex->AddEntry(MakeEntry(TEXT("Lore_Engine"),
			NSLOCTEXT("HUD", "CodexEngineT", "The Eternal Engine"),
			NSLOCTEXT("HUD", "CodexEngineC", "A perpetual-motion locomotive built by Mr. Wilford. As long as the Engine turns, what remains of humanity survives the frozen world outside."),
			ECodexCategory::Lore));
		Codex->AddEntry(MakeEntry(TEXT("Lore_Freeze"),
			NSLOCTEXT("HUD", "CodexFreezeT", "The Freeze"),
			NSLOCTEXT("HUD", "CodexFreezeC", "CW-7, released to reverse global warming, worked too well. The world froze. Everything outside the train has been dead for years - or so the announcements say."),
			ECodexCategory::Lore));
		Codex->AddEntry(MakeEntry(TEXT("Loc_Tail"),
			NSLOCTEXT("HUD", "CodexTailT", "The Tail"),
			NSLOCTEXT("HUD", "CodexTailC", "The rear cars. No windows, no tickets, no rights. Tailies survive on protein blocks and whatever they can barter, while the Jackboots keep them behind the Blockade."),
			ECodexCategory::Locations));
		Codex->AddEntry(MakeEntry(TEXT("Lore_Kronole"),
			NSLOCTEXT("HUD", "CodexKronoleT", "Kronole"),
			NSLOCTEXT("HUD", "CodexKronoleC", "Industrial solvent turned street drug. Dulls pain, sharpens reflexes, and rots the user from the inside. The Kronole Network trades it the length of the train."),
			ECodexCategory::Lore));
		return Codex;
	}

	case ESEEUIScreen::Crafting:
	{
		UCraftingComponent* CraftComp =
			Pawn ? Pawn->FindComponentByClass<UCraftingComponent>() : nullptr;
		UInventoryComponent* LegacyInv =
			Pawn ? Pawn->FindComponentByClass<UInventoryComponent>() : nullptr;
		return SNew(SSEECraftingPanel)
			.CraftingComponent(CraftComp)
			.InventoryComponent(LegacyInv);
	}

	case ESEEUIScreen::PauseMenu:
	{
		return SNew(SSEEPauseMenu)
			.OnResume(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::HandlePauseResume))
			.OnSave(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::HandlePauseSave))
			.OnLoad(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::HandlePauseLoad))
			.OnSettings(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::PushSettingsOverlay))
			.OnQuitToMenu(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::HandleQuitToMainMenu))
			.OnQuitToDesktop(FOnPauseMenuAction::CreateUObject(this, &USEEUISubsystem::HandleQuitToDesktop));
	}

	case ESEEUIScreen::MainMenu:
	{
		return SNew(SSEEMainMenu)
			.bHasSaveGame(DoesSaveGameExistOnDisk())
			.bShowCredits(false)
			.OnNewGame(FOnMenuAction::CreateUObject(this, &USEEUISubsystem::HandleMainMenuNewGame))
			.OnContinue(FOnMenuAction::CreateUObject(this, &USEEUISubsystem::HandleMainMenuContinue))
			.OnHostGame(FOnMenuAction::CreateUObject(this, &USEEUISubsystem::HandleMainMenuHostGame))
			.OnJoinGame(FOnMenuJoinAction::CreateUObject(this, &USEEUISubsystem::HandleMainMenuJoinGame))
			.OnSettings(FOnMenuAction::CreateUObject(this, &USEEUISubsystem::PushSettingsOverlay))
			.OnQuit(FOnMenuAction::CreateUObject(this, &USEEUISubsystem::HandleQuitToDesktop));
	}

	case ESEEUIScreen::DeathScreen:
	{
		TSharedRef<SSEEDeathScreen> Death = SNew(SSEEDeathScreen)
			.OnReloadCheckpoint(FOnDeathScreenAction::CreateUObject(this, &USEEUISubsystem::HandleDeathReloadCheckpoint))
			.OnQuitToMenu(FOnDeathScreenAction::CreateUObject(this, &USEEUISubsystem::HandleQuitToMainMenu));
		Death->SetDeathCause(PendingDeathCause);
		return Death;
	}

	case ESEEUIScreen::Dialogue:
	{
		USEEDialogueManager* Dialogue = GetDialogueManager();
		if (!Dialogue || !Dialogue->IsInConversation()) return nullptr;

		TSharedRef<SSEEDialoguePanel> Panel = SNew(SSEEDialoguePanel);
		Panel->SetOnChoiceSelected(FOnDialogueChoiceSelectedSlate::CreateUObject(
			this, &USEEUISubsystem::HandleDialogueChoiceSelected));
		Panel->SetOnDismissed(FSimpleDelegate::CreateUObject(
			this, &USEEUISubsystem::HandleDialogueContinue));
		Panel->SetOnCloseRequested(FSimpleDelegate::CreateUObject(
			this, &USEEUISubsystem::HandleDialogueCloseRequested));

		Dialogue->OnDialogueNodeChanged.AddDynamic(this, &USEEUISubsystem::HandleDialogueNodeChanged);
		Dialogue->OnDialogueEnded.AddDynamic(this, &USEEUISubsystem::HandleDialogueEnded);
		bDialogueDelegatesBound = true;

		DialoguePanelWidget = Panel;
		LastDialogueSpeaker = FText::GetEmpty();
		LastDialogueText = FText::GetEmpty();
		PushNodeToDialoguePanel(Dialogue->GetCurrentNode());
		return Panel;
	}

	default:
		return nullptr;
	}
}

void USEEUISubsystem::RemoveActiveScreenWidget()
{
	if (BoundInventoryComp.IsValid())
	{
		BoundInventoryComp->OnInventoryChanged.RemoveDynamic(this, &USEEUISubsystem::HandlePlayerInventoryChanged);
	}
	BoundInventoryComp.Reset();
	InventoryScreenWidget.Reset();

	CleanupDialogueState();

	if (ActiveScreenWidget.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(ActiveScreenWidget.ToSharedRef());
		}
		ActiveScreenWidget.Reset();
	}
}

void USEEUISubsystem::ApplyOpenInputMode(const TSharedPtr<SWidget>& FocusWidget)
{
	APlayerController* PC = GetLocalPlayerController();
	if (!PC) return;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(FocusWidget);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = true;
}

void USEEUISubsystem::ApplyClosedInputMode()
{
	APlayerController* PC = GetLocalPlayerController();
	if (!PC) return;

	FInputModeGameOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->bShowMouseCursor = false;
	PC->FlushPressedKeys();
}

int32 USEEUISubsystem::GetScreenZOrder(ESEEUIScreen Screen)
{
	switch (Screen)
	{
	case ESEEUIScreen::MainMenu:    return 90;
	case ESEEUIScreen::PauseMenu:   return 100;
	case ESEEUIScreen::DeathScreen: return 110;
	default:                        return 50; // gameplay screens
	}
}

// --- Settings overlay ---

void USEEUISubsystem::PushSettingsOverlay()
{
	if (SettingsOverlayWidget.IsValid()) return;
	if (!GEngine || !GEngine->GameViewport) return;

	TSharedRef<SSEESettingsPanel> Settings = SNew(SSEESettingsPanel)
		.PlayerController(GetLocalPlayerController())
		.OnClose(FSimpleDelegate::CreateUObject(this, &USEEUISubsystem::PopSettingsOverlay));

	SettingsOverlayWidget = Settings;
	GEngine->GameViewport->AddViewportWidgetContent(Settings, 200);

	// Route Escape/keys to the overlay while it is up.
	ApplyOpenInputMode(SettingsOverlayWidget);
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(SettingsOverlayWidget, EFocusCause::SetDirectly);
	}
}

void USEEUISubsystem::PopSettingsOverlay()
{
	if (!SettingsOverlayWidget.IsValid()) return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(SettingsOverlayWidget.ToSharedRef());
	}
	SettingsOverlayWidget.Reset();

	// Hand focus back to the menu underneath (if any).
	if (ActiveScreenWidget.IsValid())
	{
		ApplyOpenInputMode(ActiveScreenWidget);
	}
}

// --- Menu actions ---

void USEEUISubsystem::HandleMainMenuNewGame()
{
	// The world booted fresh underneath the menu; just drop into it.
	CloseCurrentScreen();
}

void USEEUISubsystem::HandleMainMenuContinue()
{
	if (USEESaveGameSubsystem* SaveSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USEESaveGameSubsystem>() : nullptr)
	{
		SaveSub->LoadFromSlot();
	}
	CloseCurrentScreen();
}

void USEEUISubsystem::HandleMainMenuHostGame()
{
	OpenHostGame();
}

void USEEUISubsystem::HandleMainMenuJoinGame(const FString& Address)
{
	OpenJoinGame(Address);
}

void USEEUISubsystem::OpenHostGame()
{
	// Drop the menu (restores game input + unpauses) before the listen-server travel.
	CloseCurrentScreen();

	// Re-arm so the boot menu reappears if we ever quit back out, then travel.
	bMainMenuShownThisSession = true; // we're entering gameplay, not the menu
	USEENetTravel::HostListenServer(GetGameWorld());
}

void USEEUISubsystem::OpenJoinGame(const FString& IP)
{
	CloseCurrentScreen();
	bMainMenuShownThisSession = true;
	USEENetTravel::JoinByAddress(GetGameWorld(), IP);
}

void USEEUISubsystem::HandleQuitToDesktop()
{
	UKismetSystemLibrary::QuitGame(GetGameWorld(), GetLocalPlayerController(), EQuitPreference::Quit, false);
}

void USEEUISubsystem::HandlePauseResume()
{
	CloseCurrentScreen(); // unpauses (pause-menu close path)
}

void USEEUISubsystem::HandlePauseSave()
{
	if (USEESaveGameSubsystem* SaveSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USEESaveGameSubsystem>() : nullptr)
	{
		SaveSub->WriteToSlot();
	}
}

void USEEUISubsystem::HandlePauseLoad()
{
	if (USEESaveGameSubsystem* SaveSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<USEESaveGameSubsystem>() : nullptr)
	{
		SaveSub->LoadFromSlot();
	}
}

void USEEUISubsystem::HandleQuitToMainMenu()
{
	UWorld* World = GetGameWorld();

	// Re-arm the boot menu, drop the current screen, and reload the level so the
	// world (and a possibly-dead pawn) comes back in a clean main-menu state.
	bMainMenuShownThisSession = false;
	CloseCurrentScreen();

	if (World)
	{
		const FString LevelName = UGameplayStatics::GetCurrentLevelName(World, true);
		UGameplayStatics::OpenLevel(World, FName(*LevelName));
	}
}

void USEEUISubsystem::HandleDeathReloadCheckpoint()
{
	CloseCurrentScreen();

	if (APawn* Pawn = GetLocalPawn())
	{
		if (USEEHealthComponent* Health = Pawn->FindComponentByClass<USEEHealthComponent>())
		{
			Health->Revive(0.5f);
		}
	}

	if (UWorld* World = GetGameWorld())
	{
		if (ASnowpiercerEEGameMode* GM = World->GetAuthGameMode<ASnowpiercerEEGameMode>())
		{
			GM->RespawnPlayer();
		}
	}
}

void USEEUISubsystem::HandlePlayerInventoryChanged()
{
	if (InventoryScreenWidget.IsValid())
	{
		InventoryScreenWidget->Refresh();
	}
}

// --- Dialogue plumbing ---

void USEEUISubsystem::HandleDialogueNodeChanged(const FSEEDialogueNode& CurrentNode)
{
	PushNodeToDialoguePanel(CurrentNode);
}

void USEEUISubsystem::HandleDialogueEnded()
{
	if (CurrentScreen == ESEEUIScreen::Dialogue)
	{
		CloseCurrentScreen(); // RemoveActiveScreenWidget cleans up the bindings
	}
}

void USEEUISubsystem::HandleDialogueChoiceSelected(FName ChoiceID)
{
	// ChoiceID carries the display index of the available choice ("0".."3").
	const int32 ChoiceIndex = FCString::Atoi(*ChoiceID.ToString());
	if (USEEDialogueManager* Dialogue = GetDialogueManager())
	{
		Dialogue->SelectChoice(ChoiceIndex);
	}
}

void USEEUISubsystem::HandleDialogueContinue()
{
	if (USEEDialogueManager* Dialogue = GetDialogueManager())
	{
		Dialogue->AdvanceDialogue();
	}
}

void USEEUISubsystem::HandleDialogueCloseRequested()
{
	if (USEEDialogueManager* Dialogue = GetDialogueManager())
	{
		Dialogue->EndConversation(); // OnDialogueEnded closes the panel
	}
}

void USEEUISubsystem::PushNodeToDialoguePanel(const FSEEDialogueNode& Node)
{
	if (!DialoguePanelWidget.IsValid()) return;

	FDialogueLine Line;

	// Choice nodes in DT_Dialogue_Zone1 have empty speaker/text — keep the
	// preceding NPC line on screen as context behind the choices.
	if (Node.NodeType == ESEEDialogueNodeType::PlayerChoice && Node.DialogueText.IsEmpty())
	{
		Line.SpeakerName = LastDialogueSpeaker;
		Line.DialogueText = LastDialogueText;
	}
	else
	{
		Line.SpeakerName = Node.SpeakerName;
		Line.DialogueText = Node.DialogueText;
		LastDialogueSpeaker = Node.SpeakerName;
		LastDialogueText = Node.DialogueText;
	}

	if (Node.NodeType == ESEEDialogueNodeType::PlayerChoice)
	{
		if (USEEDialogueManager* Dialogue = GetDialogueManager())
		{
			const TArray<FSEEDialogueChoice> Available = Dialogue->GetAvailableChoices();
			for (int32 i = 0; i < Available.Num(); ++i)
			{
				FDialogueChoice Choice;
				Choice.ChoiceText = FText::Format(NSLOCTEXT("HUD", "DialogueChoiceFmt", "{0}. {1}"),
					FText::AsNumber(i + 1), Available[i].ChoiceText);
				Choice.ChoiceID = FName(*FString::FromInt(i));
				Choice.bIsAvailable = true;
				Line.Choices.Add(MoveTemp(Choice));
			}
		}
	}

	DialoguePanelWidget->SetDialogueLine(Line);

	// Keep keyboard focus on the panel (clicking a choice button steals it),
	// so 1-4 / E / Esc keep working for the whole conversation.
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().SetKeyboardFocus(DialoguePanelWidget, EFocusCause::SetDirectly);
	}
}

void USEEUISubsystem::CleanupDialogueState()
{
	if (bDialogueDelegatesBound)
	{
		if (USEEDialogueManager* Dialogue = GetDialogueManager())
		{
			Dialogue->OnDialogueNodeChanged.RemoveDynamic(this, &USEEUISubsystem::HandleDialogueNodeChanged);
			Dialogue->OnDialogueEnded.RemoveDynamic(this, &USEEUISubsystem::HandleDialogueEnded);

			// Screen torn down mid-conversation (e.g. another screen opened
			// over it) — end the talk; delegates are already unbound.
			if (Dialogue->IsInConversation())
			{
				Dialogue->EndConversation();
			}
		}
		bDialogueDelegatesBound = false;
	}

	if (DialogueNPCPawn.IsValid())
	{
		// Release the brain's "in dialogue" pause via the same host/guest path used
		// to set it (direct on authority, Server RPC from a guest).
		if (APawn* LocalPawn = GetLocalPawn())
		{
			if (ASEECharacter* LocalChar = Cast<ASEECharacter>(LocalPawn))
			{
				LocalChar->SetNPCInDialogue(DialogueNPCPawn.Get(), false);
			}
		}
	}
	DialogueNPCPawn.Reset();
	DialoguePanelWidget.Reset();
}

USEEDialogueManager* USEEUISubsystem::GetDialogueManager() const
{
	return GetGameInstance() ? GetGameInstance()->GetSubsystem<USEEDialogueManager>() : nullptr;
}

void USEEUISubsystem::HandleWorldTearDown(UWorld* World)
{
	// Drop all viewport widgets before the world (and the components the
	// widgets observe) goes away. No input-mode changes - the controller is dying.
	PopSettingsOverlay();
	RemoveActiveScreenWidget();
	CurrentScreen = ESEEUIScreen::None;
}

// --- Helpers ---

UWorld* USEEUISubsystem::GetGameWorld() const
{
	return GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
}

APlayerController* USEEUISubsystem::GetLocalPlayerController() const
{
	return GetGameInstance() ? GetGameInstance()->GetFirstLocalPlayerController() : nullptr;
}

APawn* USEEUISubsystem::GetLocalPawn() const
{
	APlayerController* PC = GetLocalPlayerController();
	return PC ? PC->GetPawn() : nullptr;
}

bool USEEUISubsystem::DoesSaveGameExistOnDisk() const
{
	return UGameplayStatics::DoesSaveGameExist(GSEESaveSlotName, GSEESaveUserIndex);
}
