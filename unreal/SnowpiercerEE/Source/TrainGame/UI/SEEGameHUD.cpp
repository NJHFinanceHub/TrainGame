// SEEGameHUD.cpp - Main HUD implementation
#include "SEEGameHUD.h"

#include "Widgets/SSEESurvivalBars.h"
#include "Widgets/SSEECombatOverlay.h"
#include "Widgets/SSEEInventoryScreen.h"
#include "Widgets/SSEEDialoguePanel.h"
#include "Widgets/SSEETrainMap.h"
#include "Widgets/SSEECraftingPanel.h"
#include "Widgets/SSEEFactionPanel.h"

#include "SnowyEngine/Survival/SurvivalComponent.h"
#include "SnowyEngine/Survival/KronoleComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"
#include "SnowyEngine/Crafting/CraftingComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"

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
}

void ASEEGameHUD::UnbindFromPlayerPawn()
{
	SurvivalComp.Reset();
	CombatComp.Reset();
	WeaponComp.Reset();
	InventoryComp.Reset();
	CraftingComp.Reset();
	KronoleComp.Reset();
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
