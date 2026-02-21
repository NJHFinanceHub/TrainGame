#include "SEEHUD.h"
#include "SEECharacter.h"
#include "SEEHealthComponent.h"
#include "SEEHungerComponent.h"
#include "SEEColdComponent.h"
#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"
#include "SEEInventoryComponent.h"
#include "UI/SEEGameHUDWidget.h"
#include "UI/SEEInventoryWidget.h"
#include "UI/SEECharacterWidget.h"
#include "UI/SEEQuestLogWidget.h"
#include "UI/SEETrainMapWidget.h"
#include "UI/SEEFactionWidget.h"
#include "UI/SEECompanionWidget.h"
#include "UI/SEECraftingWidget.h"
#include "UI/SEECodexWidget.h"
#include "UI/SEEDialogueWidget.h"
#include "UI/SEEPauseMenuWidget.h"
#include "UI/SEEDeathScreenWidget.h"
#include "UI/SEEUISubsystem.h"
#include "SnowyEngine/Crafting/CraftingComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

ASEEHUD::ASEEHUD()
{
}

void ASEEHUD::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// Try to create UMG HUD widget
	if (GameHUDWidgetClass)
	{
		GameHUDWidget = CreateWidget<USEEGameHUDWidget>(PC, GameHUDWidgetClass);
		if (GameHUDWidget)
		{
			GameHUDWidget->AddToViewport(0);
			bUsingUMG = true;

			// Initialize with character when available
			APawn* Pawn = PC->GetPawn();
			if (ASEECharacter* Char = Cast<ASEECharacter>(Pawn))
			{
				GameHUDWidget->InitializeHUD(Char);
			}
		}
	}

	// Subscribe to UI screen changes
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>();
		if (UISub)
		{
			UISub->OnScreenChanged.AddDynamic(this, &ASEEHUD::OnScreenChanged);
		}
	}

	// Cache character references for canvas fallback
	if (PC)
	{
		PlayerCharacter = Cast<ASEECharacter>(PC->GetPawn());
		if (PlayerCharacter.IsValid())
		{
			HealthComp = PlayerCharacter->FindComponentByClass<USEEHealthComponent>();
			HungerComp = PlayerCharacter->FindComponentByClass<USEEHungerComponent>();
			ColdComp = PlayerCharacter->FindComponentByClass<USEEColdComponent>();
			CombatComp = PlayerCharacter->FindComponentByClass<USEECombatComponent>();
		}
	}
}

void ASEEHUD::DrawHUD()
{
	Super::DrawHUD();

	// If UMG is active, skip canvas drawing
	if (bUsingUMG) return;

	// Canvas fallback
	if (!Canvas) return;

	if (!PlayerCharacter.IsValid())
	{
		APlayerController* PC = GetOwningPlayerController();
		if (PC)
		{
			PlayerCharacter = Cast<ASEECharacter>(PC->GetPawn());
			if (PlayerCharacter.IsValid())
			{
				HealthComp = PlayerCharacter->FindComponentByClass<USEEHealthComponent>();
				HungerComp = PlayerCharacter->FindComponentByClass<USEEHungerComponent>();
				ColdComp = PlayerCharacter->FindComponentByClass<USEEColdComponent>();
				CombatComp = PlayerCharacter->FindComponentByClass<USEECombatComponent>();
			}
		}
	}

	if (!PlayerCharacter.IsValid()) return;

	DrawHealthBar();
	DrawStaminaBar();
	DrawHungerIndicator();
	DrawColdIndicator();
	DrawWeaponIndicator();
	DrawInteractionPrompt();
	DrawCrosshair();
	DrawDamageDirection();
}

// --- UMG Screen Management ---

void ASEEHUD::OnScreenChanged(ESEEUIScreen NewScreen)
{
	HideAllScreenWidgets();

	if (NewScreen != ESEEUIScreen::None)
	{
		ShowScreenWidget(NewScreen);
		SetGameInputMode(false);
	}
	else
	{
		SetGameInputMode(true);
	}
}

void ASEEHUD::ShowScreenWidget(ESEEUIScreen Screen)
{
	UUserWidget* Widget = GetOrCreateWidget(Screen);
	if (Widget)
	{
		Widget->SetVisibility(ESlateVisibility::Visible);
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(10);
		}
	}
}

void ASEEHUD::HideAllScreenWidgets()
{
	auto HideWidget = [](UUserWidget* W)
	{
		if (W && W->IsInViewport())
		{
			W->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	HideWidget(InventoryWidget);
	HideWidget(CharacterWidget);
	HideWidget(QuestLogWidget);
	HideWidget(TrainMapWidget);
	HideWidget(FactionWidget);
	HideWidget(CompanionWidget);
	HideWidget(CraftingWidget);
	HideWidget(CodexWidget);
	HideWidget(PauseMenuWidget);
	HideWidget(DeathScreenWidget);
}

UUserWidget* ASEEHUD::GetOrCreateWidget(ESEEUIScreen Screen)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return nullptr;

	switch (Screen)
	{
	case ESEEUIScreen::Inventory:
	{
		if (!InventoryWidget && InventoryWidgetClass)
		{
			InventoryWidget = CreateWidget<USEEInventoryWidget>(PC, InventoryWidgetClass);
		}
		if (InventoryWidget && PlayerCharacter.IsValid())
		{
			USEEInventoryComponent* InvComp = PlayerCharacter->FindComponentByClass<USEEInventoryComponent>();
			if (InvComp)
			{
				InventoryWidget->InitInventory(InvComp);
			}
		}
		return InventoryWidget;
	}
	case ESEEUIScreen::Character:
	{
		if (!CharacterWidget && CharacterWidgetClass)
		{
			CharacterWidget = CreateWidget<USEECharacterWidget>(PC, CharacterWidgetClass);
		}
		if (CharacterWidget && PlayerCharacter.IsValid())
		{
			CharacterWidget->InitCharacter(PlayerCharacter.Get());
		}
		return CharacterWidget;
	}
	case ESEEUIScreen::QuestLog:
		if (!QuestLogWidget && QuestLogWidgetClass) { QuestLogWidget = CreateWidget<UUserWidget>(PC, QuestLogWidgetClass); }
		return QuestLogWidget;
	case ESEEUIScreen::TrainMap:
		if (!TrainMapWidget && TrainMapWidgetClass) { TrainMapWidget = CreateWidget<USEETrainMapWidget>(PC, TrainMapWidgetClass); }
		return TrainMapWidget;
	case ESEEUIScreen::Factions:
		if (!FactionWidget && FactionWidgetClass) { FactionWidget = CreateWidget<USEEFactionWidget>(PC, FactionWidgetClass); }
		return FactionWidget;
	case ESEEUIScreen::Companions:
		if (!CompanionWidget && CompanionWidgetClass) { CompanionWidget = CreateWidget<USEECompanionWidget>(PC, CompanionWidgetClass); }
		return CompanionWidget;
	case ESEEUIScreen::Crafting:
	{
		if (!CraftingWidget && CraftingWidgetClass)
		{
			CraftingWidget = CreateWidget<USEECraftingWidget>(PC, CraftingWidgetClass);
		}
		if (CraftingWidget && PlayerCharacter.IsValid())
		{
			UCraftingComponent* CraftComp = PlayerCharacter->FindComponentByClass<UCraftingComponent>();
			USEEInventoryComponent* InvComp = PlayerCharacter->FindComponentByClass<USEEInventoryComponent>();
			if (CraftComp)
			{
				CraftingWidget->InitCrafting(CraftComp, InvComp);
			}
		}
		return CraftingWidget;
	}
	case ESEEUIScreen::Codex:
		if (!CodexWidget && CodexWidgetClass) { CodexWidget = CreateWidget<USEECodexWidget>(PC, CodexWidgetClass); }
		return CodexWidget;
	case ESEEUIScreen::PauseMenu:
		if (!PauseMenuWidget && PauseMenuWidgetClass) { PauseMenuWidget = CreateWidget<USEEPauseMenuWidget>(PC, PauseMenuWidgetClass); }
		return PauseMenuWidget;
	case ESEEUIScreen::DeathScreen:
		if (!DeathScreenWidget && DeathScreenWidgetClass) { DeathScreenWidget = CreateWidget<USEEDeathScreenWidget>(PC, DeathScreenWidgetClass); }
		return DeathScreenWidget;
	default:
		return nullptr;
	}
}

void ASEEHUD::SetGameInputMode(bool bGameOnly)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (bGameOnly)
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
	}
	else
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void ASEEHUD::ShowDialogue()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (!DialogueWidget && DialogueWidgetClass)
	{
		DialogueWidget = CreateWidget<USEEDialogueWidget>(PC, DialogueWidgetClass);
	}

	if (DialogueWidget)
	{
		if (!DialogueWidget->IsInViewport())
		{
			DialogueWidget->AddToViewport(5);
		}
		DialogueWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ASEEHUD::HideDialogue()
{
	if (DialogueWidget)
	{
		DialogueWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ASEEHUD::ShowDeathScreen(const FText& CauseOfDeath)
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	if (!DeathScreenWidget && DeathScreenWidgetClass)
	{
		DeathScreenWidget = CreateWidget<USEEDeathScreenWidget>(PC, DeathScreenWidgetClass);
	}

	if (DeathScreenWidget)
	{
		if (!DeathScreenWidget->IsInViewport())
		{
			DeathScreenWidget->AddToViewport(20);
		}
		DeathScreenWidget->ShowDeathScreen(CauseOfDeath);
		SetGameInputMode(false);
	}
}

// --- Canvas Fallback Drawing (same as original) ---

void ASEEHUD::DrawHealthBar()
{
	if (!HealthComp.IsValid()) return;

	float Percent = HealthComp->GetHealthPercent();
	FLinearColor Color = FMath::Lerp(FLinearColor::Red, FLinearColor::Green, Percent);

	DrawBar(20.0f, 20.0f, 200.0f, 20.0f, Percent, Color, FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));

	FString HPText = FString::Printf(TEXT("HP: %d/%d"),
		FMath::RoundToInt32(HealthComp->GetCurrentHealth()),
		FMath::RoundToInt32(HealthComp->GetMaxHealth()));
	DrawText(HPText, FLinearColor::White, 25.0f, 22.0f);

	if (HealthComp->IsDowned())
	{
		DrawText(TEXT("DOWNED - NEED REVIVE"), FLinearColor::Red,
			Canvas->SizeX * 0.5f - 100.0f, Canvas->SizeY * 0.4f);
	}

	TArray<FSEEInjury> Injuries = HealthComp->GetActiveInjuries();
	float InjuryY = 45.0f;
	for (const FSEEInjury& Injury : Injuries)
	{
		FString InjuryText;
		switch (Injury.Type)
		{
		case ESEEInjuryType::BrokenArm: InjuryText = TEXT("[Broken Arm -30% melee]"); break;
		case ESEEInjuryType::LegWound: InjuryText = TEXT("[Leg Wound -40% speed]"); break;
		case ESEEInjuryType::Concussion: InjuryText = TEXT("[Concussion]"); break;
		case ESEEInjuryType::DeepCut: InjuryText = TEXT("[Deep Cut]"); break;
		case ESEEInjuryType::InternalBleeding: InjuryText = TEXT("[Internal Bleeding]"); break;
		default: break;
		}
		if (!InjuryText.IsEmpty())
		{
			DrawText(InjuryText, FLinearColor(1.0f, 0.5f, 0.0f), 20.0f, InjuryY);
			InjuryY += 15.0f;
		}
	}
}

void ASEEHUD::DrawStaminaBar()
{
	float Percent = PlayerCharacter->GetStamina() / PlayerCharacter->GetMaxStamina();
	FLinearColor Color = FLinearColor(0.2f, 0.6f, 1.0f);
	if (Percent < 0.25f) Color = FLinearColor(1.0f, 0.8f, 0.0f);

	float Y = HealthComp.IsValid() ? 45.0f : 20.0f;
	DrawBar(20.0f, Y, 180.0f, 14.0f, Percent, Color, FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
}

void ASEEHUD::DrawHungerIndicator()
{
	if (!HungerComp.IsValid()) return;

	float Percent = HungerComp->GetHungerPercent();
	if (Percent >= 0.75f) return;

	float Y = 70.0f;
	FLinearColor Color = Percent > 0.25f ? FLinearColor(0.8f, 0.6f, 0.2f) : FLinearColor(1.0f, 0.2f, 0.0f);

	FString Text = FString::Printf(TEXT("Hunger: %d%%"), FMath::RoundToInt32(Percent * 100.0f));
	DrawText(Text, Color, 20.0f, Y);
}

void ASEEHUD::DrawColdIndicator()
{
	if (!ColdComp.IsValid() || !ColdComp->IsInColdZone()) return;

	float Temp = ColdComp->GetTemperature();
	ESEEFrostbiteStage Stage = ColdComp->GetFrostbiteStage();

	FLinearColor Color;
	FString StageText;
	switch (Stage)
	{
	case ESEEFrostbiteStage::Shivers:
		Color = FLinearColor(0.5f, 0.7f, 1.0f);
		StageText = TEXT("Shivers");
		break;
	case ESEEFrostbiteStage::Numbness:
		Color = FLinearColor(0.3f, 0.4f, 1.0f);
		StageText = TEXT("Numbness -40% speed");
		break;
	case ESEEFrostbiteStage::Blackout:
		Color = FLinearColor(0.8f, 0.2f, 1.0f);
		StageText = TEXT("FROSTBITE - SEEK WARMTH");
		break;
	default:
		Color = FLinearColor(0.6f, 0.8f, 1.0f);
		StageText = TEXT("Cold Zone");
		break;
	}

	FString TempText = FString::Printf(TEXT("COLD: %.0f C - %s"), Temp, *StageText);
	DrawText(TempText, Color, 20.0f, 90.0f);
}

void ASEEHUD::DrawWeaponIndicator()
{
	if (!CombatComp.IsValid()) return;

	ASEEWeaponBase* Weapon = CombatComp->GetEquippedWeapon();
	if (!Weapon) return;

	float X = Canvas->SizeX - 220.0f;
	float Y = Canvas->SizeY - 60.0f;

	FString WeaponText = Weapon->WeaponName.ToString();
	DrawText(WeaponText, FLinearColor::White, X, Y);

	float DurPercent = Weapon->GetDurabilityPercent();
	FLinearColor DurColor = DurPercent > 0.3f ? FLinearColor(0.5f, 0.8f, 0.5f) : FLinearColor(1.0f, 0.3f, 0.0f);
	DrawBar(X, Y + 18.0f, 150.0f, 8.0f, DurPercent, DurColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));

	if (Weapon->IsBroken())
	{
		DrawText(TEXT("BROKEN"), FLinearColor::Red, X + 160.0f, Y + 14.0f);
	}
}

void ASEEHUD::DrawInteractionPrompt()
{
}

void ASEEHUD::DrawDetectionIndicator()
{
}

void ASEEHUD::DrawCrosshair()
{
	float CenterX = Canvas->SizeX * 0.5f;
	float CenterY = Canvas->SizeY * 0.5f;
	float Size = 4.0f;
	float Gap = 3.0f;

	FLinearColor Color = FLinearColor(0.8f, 0.8f, 0.8f, 0.6f);

	DrawRect(Color, CenterX - 1.0f, CenterY - 1.0f, 2.0f, 2.0f);
	DrawLine(CenterX - Size - Gap, CenterY, CenterX - Gap, CenterY, Color);
	DrawLine(CenterX + Gap, CenterY, CenterX + Size + Gap, CenterY, Color);
	DrawLine(CenterX, CenterY - Size - Gap, CenterX, CenterY - Gap, Color);
	DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Size + Gap, Color);
}

void ASEEHUD::DrawDamageDirection()
{
	float TimeSinceDamage = GetWorld()->GetTimeSeconds() - LastDamageTime;
	if (TimeSinceDamage > 2.0f || LastDamageDirection.IsNearlyZero()) return;

	float Alpha = FMath::Clamp(1.0f - TimeSinceDamage / 2.0f, 0.0f, 0.8f);
	FLinearColor VignetteColor = FLinearColor(1.0f, 0.0f, 0.0f, Alpha);

	float CenterX = Canvas->SizeX * 0.5f;
	float CenterY = Canvas->SizeY * 0.5f;

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	FRotator ViewRot = PC->GetControlRotation();
	FVector LocalDir = ViewRot.UnrotateVector(LastDamageDirection);

	float VignetteSize = 100.0f;
	if (LocalDir.X < -0.3f)
	{
		DrawRect(VignetteColor, CenterX - VignetteSize, Canvas->SizeY - 30.0f, VignetteSize * 2.0f, 30.0f);
	}
	if (LocalDir.Y > 0.3f)
	{
		DrawRect(VignetteColor, Canvas->SizeX - 30.0f, CenterY - VignetteSize, 30.0f, VignetteSize * 2.0f);
	}
	if (LocalDir.Y < -0.3f)
	{
		DrawRect(VignetteColor, 0.0f, CenterY - VignetteSize, 30.0f, VignetteSize * 2.0f);
	}
	if (LocalDir.X > 0.3f)
	{
		DrawRect(VignetteColor, CenterX - VignetteSize, 0.0f, VignetteSize * 2.0f, 30.0f);
	}
}

void ASEEHUD::DrawBar(float X, float Y, float Width, float Height, float Percent,
					   FLinearColor FillColor, FLinearColor BackColor)
{
	Percent = FMath::Clamp(Percent, 0.0f, 1.0f);
	DrawRect(BackColor, X, Y, Width, Height);
	DrawRect(FillColor, X, Y, Width * Percent, Height);
}
