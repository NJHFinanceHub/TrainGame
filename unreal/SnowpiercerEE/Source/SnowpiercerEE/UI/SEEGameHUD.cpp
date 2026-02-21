// SEEGameHUD.cpp - Main HUD implementation
#include "SEEGameHUD.h"

#include "SSEESurvivalBars.h"
#include "SSEECombatOverlay.h"
#include "SSEEInventoryScreen.h"
#include "SSEEDialoguePanel.h"
#include "SSEETrainMap.h"
#include "SSEECraftingPanel.h"
#include "SSEEFactionPanel.h"
#include "SSEEQuestLog.h"
#include "SSEECharacterScreen.h"
#include "SSEECompanionScreen.h"
#include "SSEECodexPanel.h"
#include "SSEEPauseMenu.h"
#include "SSEEDeathScreen.h"

#include "SnowyEngine/Survival/SurvivalComponent.h"
#include "SnowyEngine/Survival/KronoleComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"
#include "SnowyEngine/Crafting/CraftingComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "../SEEStatsComponent.h"
#include "../SEEQuestManager.h"
#include "TrainGame/Companions/CompanionRosterSubsystem.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SWeakWidget.h"
#include "Engine/Engine.h"

void ASEEGameHUD::BeginPlay()
{
	Super::BeginPlay();
	BindToPlayerPawn();
	CreateWidgets();
}

void ASEEGameHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveWidgets();
	UnbindFromPlayerPawn();
	Super::EndPlay(EndPlayReason);
}

void ASEEGameHUD::BindToPlayerPawn()
{
	APawn* Pawn = GetOwningPawn();
	if (!Pawn)
	{
		return;
	}

	SurvivalComp = Pawn->FindComponentByClass<USurvivalComponent>();
	CombatComp = Pawn->FindComponentByClass<UCombatComponent>();
	WeaponComp = Pawn->FindComponentByClass<UWeaponComponent>();
	InventoryComp = Pawn->FindComponentByClass<UInventoryComponent>();
	CraftingComp = Pawn->FindComponentByClass<UCraftingComponent>();
	KronoleComp = Pawn->FindComponentByClass<UKronoleComponent>();
	StatsComp = Pawn->FindComponentByClass<USEEStatsComponent>();
}

void ASEEGameHUD::UnbindFromPlayerPawn()
{
	SurvivalComp.Reset();
	CombatComp.Reset();
	WeaponComp.Reset();
	InventoryComp.Reset();
	CraftingComp.Reset();
	KronoleComp.Reset();
	StatsComp.Reset();
}

void ASEEGameHUD::CreateWidgets()
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	// Survival bars - always visible (bottom-left)
	SurvivalBarsWidget = SNew(SSEESurvivalBars)
		.SurvivalComponent(SurvivalComp.Get())
		.KronoleComponent(KronoleComp.Get());

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(SurvivalBarsWidget.ToSharedRef()),
		10 // Z-order: base layer
	);

	// Combat overlay - always visible (top-center and bottom-right)
	CombatOverlayWidget = SNew(SSEECombatOverlay)
		.CombatComponent(CombatComp.Get())
		.WeaponComponent(WeaponComp.Get());

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(CombatOverlayWidget.ToSharedRef()),
		11 // Z-order: above survival bars
	);

	// Inventory screen - hidden by default
	InventoryWidget = SNew(SSEEInventoryScreen)
		.InventoryComponent(InventoryComp.Get());
	InventoryWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(InventoryWidget.ToSharedRef()),
		50 // Z-order: full-screen panels
	);

	// Dialogue panel - hidden by default (bottom of screen)
	DialogueWidget = SNew(SSEEDialoguePanel);
	DialogueWidget->SetVisibility(EVisibility::Collapsed);
	DialogueWidget->SetOnChoiceSelected(FOnDialogueChoiceSelectedSlate::CreateLambda(
		[this](FName ChoiceID)
		{
			OnDialogueChoiceSelected.Broadcast(ChoiceID);
			HideDialogue();
		}));
	DialogueWidget->SetOnDismissed(FSimpleDelegate::CreateLambda(
		[this]()
		{
			OnDialogueDismissed.Broadcast();
			HideDialogue();
		}));

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(DialogueWidget.ToSharedRef()),
		60 // Z-order: above panels
	);

	// Train map - hidden by default
	TrainMapWidget = SNew(SSEETrainMap);
	TrainMapWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(TrainMapWidget.ToSharedRef()),
		50
	);

	// Crafting panel - hidden by default
	CraftingWidget = SNew(SSEECraftingPanel)
		.CraftingComponent(CraftingComp.Get())
		.InventoryComponent(InventoryComp.Get());
	CraftingWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(CraftingWidget.ToSharedRef()),
		50
	);

	// Faction panel - hidden by default
	FactionWidget = SNew(SSEEFactionPanel);
	FactionWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(FactionWidget.ToSharedRef()),
		50
	);

	// Quest log - hidden by default
	QuestLogWidget = SNew(SSEEQuestLog);
	QuestLogWidget->SetVisibility(EVisibility::Collapsed);
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		QuestLogWidget->SetQuestManager(GI->GetSubsystem<USEEQuestManager>());
	}

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(QuestLogWidget.ToSharedRef()),
		50
	);

	// Character screen - hidden by default
	CharacterWidget = SNew(SSEECharacterScreen)
		.StatsComponent(StatsComp.Get())
		.InventoryComponent(InventoryComp.Get());
	CharacterWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(CharacterWidget.ToSharedRef()),
		50
	);

	// Companion screen - hidden by default
	CompanionWidget = SNew(SSEECompanionScreen);
	CompanionWidget->SetVisibility(EVisibility::Collapsed);
	if (UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		CompanionWidget->SetRosterSubsystem(GI->GetSubsystem<UCompanionRosterSubsystem>());
	}

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(CompanionWidget.ToSharedRef()),
		50
	);

	// Codex panel - hidden by default
	CodexWidget = SNew(SSEECodexPanel);
	CodexWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(CodexWidget.ToSharedRef()),
		50
	);

	// Pause menu - hidden by default
	PauseMenuWidget = SNew(SSEEPauseMenu);
	PauseMenuWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(PauseMenuWidget.ToSharedRef()),
		100 // Z-order: above all gameplay panels
	);

	// Death screen - hidden by default
	DeathScreenWidget = SNew(SSEEDeathScreen);
	DeathScreenWidget->SetVisibility(EVisibility::Collapsed);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(DeathScreenWidget.ToSharedRef()),
		110 // Z-order: above everything
	);
}

void ASEEGameHUD::RemoveWidgets()
{
	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	// Slate widgets are reference-counted; releasing shared pointers handles cleanup
	SurvivalBarsWidget.Reset();
	CombatOverlayWidget.Reset();
	InventoryWidget.Reset();
	DialogueWidget.Reset();
	TrainMapWidget.Reset();
	CraftingWidget.Reset();
	FactionWidget.Reset();
	QuestLogWidget.Reset();
	CharacterWidget.Reset();
	CompanionWidget.Reset();
	CodexWidget.Reset();
	PauseMenuWidget.Reset();
	DeathScreenWidget.Reset();
	ActivePanel.Reset();
}

void ASEEGameHUD::SetPanelVisible(TSharedPtr<SWidget> Panel, bool bVisible)
{
	if (!Panel.IsValid())
	{
		return;
	}

	if (bVisible)
	{
		// Close any other active full-screen panel
		if (ActivePanel.IsValid() && ActivePanel != Panel)
		{
			ActivePanel->SetVisibility(EVisibility::Collapsed);
		}
		Panel->SetVisibility(EVisibility::SelfHitTestInvisible);
		ActivePanel = Panel;
	}
	else
	{
		Panel->SetVisibility(EVisibility::Collapsed);
		if (ActivePanel == Panel)
		{
			ActivePanel.Reset();
		}
	}
}

void ASEEGameHUD::ToggleInventory()
{
	if (!InventoryWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = InventoryWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(InventoryWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleCrafting()
{
	if (!CraftingWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = CraftingWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(CraftingWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleTrainMap()
{
	if (!TrainMapWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = TrainMapWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(TrainMapWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleFactionPanel()
{
	if (!FactionWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = FactionWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(FactionWidget, !bIsVisible);
}

void ASEEGameHUD::CloseAllPanels()
{
	if (InventoryWidget.IsValid())
	{
		InventoryWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (CraftingWidget.IsValid())
	{
		CraftingWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (TrainMapWidget.IsValid())
	{
		TrainMapWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (FactionWidget.IsValid())
	{
		FactionWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (QuestLogWidget.IsValid())
	{
		QuestLogWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (CharacterWidget.IsValid())
	{
		CharacterWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (CompanionWidget.IsValid())
	{
		CompanionWidget->SetVisibility(EVisibility::Collapsed);
	}
	if (CodexWidget.IsValid())
	{
		CodexWidget->SetVisibility(EVisibility::Collapsed);
	}
	ActivePanel.Reset();
}

bool ASEEGameHUD::IsAnyPanelOpen() const
{
	return ActivePanel.IsValid() && ActivePanel->GetVisibility() != EVisibility::Collapsed;
}

void ASEEGameHUD::ShowDialogue(const FDialogueLine& Line)
{
	if (DialogueWidget.IsValid())
	{
		DialogueWidget->SetDialogueLine(Line);
		DialogueWidget->SetVisibility(EVisibility::Visible);
		bDialogueActive = true;
	}
}

void ASEEGameHUD::HideDialogue()
{
	if (DialogueWidget.IsValid())
	{
		DialogueWidget->SetVisibility(EVisibility::Collapsed);
		bDialogueActive = false;
	}
}

bool ASEEGameHUD::IsDialogueActive() const
{
	return bDialogueActive;
}

void ASEEGameHUD::SetFactionReputations(const TArray<FFactionReputation>& Reputations)
{
	if (FactionWidget.IsValid())
	{
		FactionWidget->UpdateReputations(Reputations);
	}
}

void ASEEGameHUD::ToggleQuestLog()
{
	if (!QuestLogWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = QuestLogWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(QuestLogWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleCharacterScreen()
{
	if (!CharacterWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = CharacterWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(CharacterWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleCompanionScreen()
{
	if (!CompanionWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = CompanionWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(CompanionWidget, !bIsVisible);
}

void ASEEGameHUD::ToggleCodex()
{
	if (!CodexWidget.IsValid())
	{
		return;
	}

	const bool bIsVisible = CodexWidget->GetVisibility() != EVisibility::Collapsed;
	SetPanelVisible(CodexWidget, !bIsVisible);
}

void ASEEGameHUD::ShowPauseMenu()
{
	if (PauseMenuWidget.IsValid())
	{
		CloseAllPanels();
		PauseMenuWidget->SetVisibility(EVisibility::Visible);
		bPauseMenuActive = true;
	}
}

void ASEEGameHUD::HidePauseMenu()
{
	if (PauseMenuWidget.IsValid())
	{
		PauseMenuWidget->SetVisibility(EVisibility::Collapsed);
		bPauseMenuActive = false;
	}
}

void ASEEGameHUD::ShowDeathScreen(const FText& DeathCause)
{
	if (DeathScreenWidget.IsValid())
	{
		CloseAllPanels();
		HidePauseMenu();
		DeathScreenWidget->SetDeathCause(DeathCause);
		DeathScreenWidget->SetVisibility(EVisibility::Visible);
		bDeathScreenActive = true;
	}
}

void ASEEGameHUD::HideDeathScreen()
{
	if (DeathScreenWidget.IsValid())
	{
		DeathScreenWidget->SetVisibility(EVisibility::Collapsed);
		bDeathScreenActive = false;
	}
}
