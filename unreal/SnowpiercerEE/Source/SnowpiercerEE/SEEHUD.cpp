#include "SEEHUD.h"
#include "SEECharacter.h"
#include "SEEHealthComponent.h"
#include "SEEColdComponent.h"
#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"
#include "UI/SEEUISubsystem.h"
#include "UI/Widgets/SSEEUIStyle.h"
#include "TrainGame/Economy/ArmorComponent.h"
#include "AI/SEENPCAIController.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

ASEEHUD::ASEEHUD()
{
}

void ASEEHUD::BeginPlay()
{
	Super::BeginPlay();

	CachePawnComponents();

	// Boot flow: show the main menu (paused) the first time a map loads this session.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			UISub->ShowMainMenuIfFirstBoot();
		}
	}
}

void ASEEHUD::CachePawnComponents()
{
	APlayerController* PC = GetOwningPlayerController();
	PlayerCharacter = PC ? Cast<ASEECharacter>(PC->GetPawn()) : nullptr;

	if (PlayerCharacter.IsValid())
	{
		HealthComp = PlayerCharacter->FindComponentByClass<USEEHealthComponent>();
		ColdComp = PlayerCharacter->FindComponentByClass<USEEColdComponent>();
		CombatComp = PlayerCharacter->FindComponentByClass<USEECombatComponent>();
		ArmorComp = PlayerCharacter->FindComponentByClass<UArmorComponent>();

		if (HealthComp.IsValid())
		{
			HealthComp->OnDeath.AddUniqueDynamic(this, &ASEEHUD::HandleOwnerDeath);
		}
	}
	else
	{
		HealthComp.Reset();
		ColdComp.Reset();
		CombatComp.Reset();
		ArmorComp.Reset();
	}
}

void ASEEHUD::HandleOwnerDeath()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			UISub->NotifyPlayerDeath(
				NSLOCTEXT("HUD", "DeathCauseGeneric", "The Eternal Engine grinds on without you."));
		}
	}
}

void ASEEHUD::ShowDeathScreen(const FText& CauseOfDeath)
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			UISub->NotifyPlayerDeath(CauseOfDeath);
		}
	}
}

void ASEEHUD::ShowDialogue()
{
	// Dialogue opens with an NPC context via USEEUISubsystem::OpenDialogue
	// (driven from ASEECharacter::Interact); nothing sensible to show from here.
}

void ASEEHUD::HideDialogue()
{
	// Delegate: close the subsystem's dialogue panel if one is up.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			if (UISub->GetCurrentScreen() == ESEEUIScreen::Dialogue)
			{
				UISub->CloseCurrentScreen();
			}
		}
	}
}

void ASEEHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	// Re-cache when the pawn changes (possession after BeginPlay, respawn, ...).
	APlayerController* PC = GetOwningPlayerController();
	APawn* CurrentPawn = PC ? PC->GetPawn() : nullptr;
	if (PlayerCharacter.Get() != CurrentPawn)
	{
		CachePawnComponents();
	}

	if (!PlayerCharacter.IsValid()) return;

	// Suppress the gameplay HUD while a full-screen UI screen is up.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			if (UISub->IsScreenOpen()) return;
		}
	}

	DrawHealthBar();
	DrawStaminaBar();
	DrawArmorIndicator();
	DrawColdIndicator();
	DrawWeaponIndicator();
	DrawCrosshair();
	DrawDamageDirection();
	DrawNPCLabels();
}

void ASEEHUD::DrawNPCLabels()
{
	APlayerController* PC = GetOwningPlayerController();
	APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;
	UWorld* World = GetWorld();
	if (!PC || !PlayerPawn || !World) return;

	const FVector PlayerLoc = PlayerPawn->GetActorLocation();
	constexpr float MaxLabelDistSq = 2000.0f * 2000.0f;
	constexpr float TalkHintDistSq = 350.0f * 350.0f;

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* NPC = *It;
		if (!NPC || NPC == PlayerPawn) continue;

		const float DistSq = FVector::DistSquared(PlayerLoc, NPC->GetActorLocation());
		if (DistSq > MaxLabelDistSq) continue;

		ASEENPCAIController* Brain = Cast<ASEENPCAIController>(NPC->GetController());
		if (!Brain) continue;

		// Role color: aggroed red, hostile dark red, talkable green, else grey
		FLinearColor Color(0.75f, 0.75f, 0.75f, 0.9f);
		FString Suffix;
		if (Brain->bHostile)
		{
			Color = Brain->IsAggroed()
				? FLinearColor(1.0f, 0.15f, 0.1f, 1.0f)
				: FLinearColor(0.7f, 0.2f, 0.15f, 0.9f);
		}
		else if (Brain->CanStartDialogue())
		{
			Color = FLinearColor(0.4f, 0.9f, 0.5f, 0.9f);
			if (DistSq <= TalkHintDistSq)
			{
				Suffix = TEXT("  [E]");
			}
		}

		// Display name from the placed label (NPC_Gilliam -> Gilliam),
		// falling back to a class-derived role
		FString Name = NPC->GetActorNameOrLabel();
		Name.RemoveFromStart(TEXT("NPC_"));
		Name.RemoveFromStart(TEXT("Boss_"));
		if (Name.StartsWith(TEXT("BP_NPC_")) || Name.Contains(TEXT("_C_")))
		{
			const FString ClassName = NPC->GetClass()->GetName();
			if (ClassName.Contains(TEXT("Jackboot")))      Name = TEXT("Jackboot");
			else if (ClassName.Contains(TEXT("Merchant")))  Name = TEXT("Merchant");
			else if (ClassName.Contains(TEXT("Breachman"))) Name = TEXT("Breachman");
			else                                            Name = TEXT("Tailie");
		}

		// Project a point above the head; skip if behind the camera
		FVector2D Screen;
		const FVector HeadLoc = NPC->GetActorLocation() + FVector(0, 0, 115.0f);
		if (!PC->ProjectWorldLocationToScreen(HeadLoc, Screen, true)) continue;

		// Fade with distance
		const float DistFade = 1.0f - FMath::Clamp(
			(FMath::Sqrt(DistSq) - 600.0f) / 1400.0f, 0.0f, 0.65f);
		Color.A *= DistFade;

		const FString Label = Name + Suffix;
		float TextW = 0.0f, TextH = 0.0f;
		GetTextSize(Label, TextW, TextH, GEngine->GetSmallFont());
		DrawText(Label, Color, Screen.X - TextW * 0.5f, Screen.Y - TextH,
				 GEngine->GetSmallFont());

		// Floating health bar for hostiles — shown once aggroed or wounded so the
		// player gets unmistakable feedback that hits are landing.
		if (Brain->bHostile)
		{
			if (const USEEHealthComponent* NPCHealth = NPC->FindComponentByClass<USEEHealthComponent>())
			{
				const float HP = NPCHealth->GetHealthPercent();
				if (Brain->IsAggroed() || HP < 0.999f)
				{
					const float BarW = 56.0f;
					const float BarH = 5.0f;
					const float BX = Screen.X - BarW * 0.5f;
					const float BY = Screen.Y + 2.0f;
					DrawRect(FLinearColor(0.05f, 0.05f, 0.05f, 0.8f * DistFade), BX - 1.0f, BY - 1.0f, BarW + 2.0f, BarH + 2.0f);
					DrawRect(FLinearColor(0.85f, 0.12f, 0.10f, 0.95f * DistFade), BX, BY, BarW * FMath::Clamp(HP, 0.0f, 1.0f), BarH);
				}
			}
		}
	}

	// Center hitmarker: a brief white X for ~0.18s after a melee hit connects.
	if (CombatComp.IsValid() && CombatComp->GetTimeSinceHitLanded() < 0.18f && Canvas)
	{
		const float CX = Canvas->SizeX * 0.5f;
		const float CY = Canvas->SizeY * 0.5f;
		const FLinearColor MarkerColor(1.0f, 1.0f, 1.0f, 0.9f);
		const float G = 5.0f;   // inner gap
		const float L = 9.0f;   // arm length
		// four diagonal ticks forming an X
		for (int32 sx = -1; sx <= 1; sx += 2)
		{
			for (int32 sy = -1; sy <= 1; sy += 2)
			{
				const float x0 = CX + sx * G;
				const float y0 = CY + sy * G;
				DrawLine(x0, y0, x0 + sx * L, y0 + sy * L, MarkerColor, 2.0f);
			}
		}
	}
}

// --- Canvas Drawing ---

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
		DrawText(TEXT("DOWNED - NEED REVIVE"), SEEUIStyle::DangerRed,
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
			DrawText(InjuryText, SEEUIStyle::WarnAmber, 20.0f, InjuryY);
			InjuryY += 15.0f;
		}
	}
}

void ASEEHUD::DrawStaminaBar()
{
	float Percent = PlayerCharacter->GetMaxStamina() > 0.0f
		? PlayerCharacter->GetStamina() / PlayerCharacter->GetMaxStamina()
		: 0.0f;
	FLinearColor Color = FLinearColor(0.2f, 0.6f, 1.0f);
	if (Percent < 0.25f) Color = FLinearColor(1.0f, 0.8f, 0.0f);

	float Y = HealthComp.IsValid() ? 45.0f : 20.0f;
	DrawBar(20.0f, Y, 180.0f, 14.0f, Percent, Color, FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));
}

void ASEEHUD::DrawArmorIndicator()
{
	if (!ArmorComp.IsValid()) return;

	const float X = 20.0f;
	const float Y = 70.0f;

	// Total passive damage reduction (0-75, percent points).
	const float DR = ArmorComp->GetTotalDamageReduction();
	FString ArmorText = FString::Printf(TEXT("ARMOR: %d%% DR"), FMath::RoundToInt32(DR));
	DrawText(ArmorText, SEEUIStyle::AccentSteel, X, Y);

	// Per-slot durability pips (Head / Torso / Shield).
	struct FSlotDisplay { EArmorSlot Slot; const TCHAR* Label; };
	const FSlotDisplay Slots[] =
	{
		{ EArmorSlot::Head,   TEXT("H") },
		{ EArmorSlot::Torso,  TEXT("T") },
		{ EArmorSlot::Shield, TEXT("S") },
	};

	float PipX = X;
	const float PipY = Y + 18.0f;
	for (const FSlotDisplay& Entry : Slots)
	{
		FEquippedArmor Armor;
		if (!ArmorComp->GetArmorInSlot(Entry.Slot, Armor)) continue;

		const float Durability = Armor.MaxDurability > 0.0f
			? FMath::Clamp(Armor.CurrentDurability / Armor.MaxDurability, 0.0f, 1.0f)
			: 0.0f;

		FLinearColor FillColor = SEEUIStyle::OkGreen;
		if (Armor.IsBroken())            FillColor = SEEUIStyle::DangerRed;
		else if (Durability < 0.3f)      FillColor = SEEUIStyle::WarnAmber;

		DrawText(Entry.Label, SEEUIStyle::TextDim, PipX, PipY - 3.0f);
		DrawBar(PipX + 12.0f, PipY, 42.0f, 8.0f, Durability, FillColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));

		if (Armor.IsBroken())
		{
			DrawText(TEXT("!"), SEEUIStyle::DangerRed, PipX + 56.0f, PipY - 3.0f);
		}

		PipX += 72.0f;
	}
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
	DrawText(TempText, Color, 20.0f, 96.0f);
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
	FLinearColor DurColor = DurPercent > 0.3f ? SEEUIStyle::OkGreen : SEEUIStyle::DangerRed;
	DrawBar(X, Y + 18.0f, 150.0f, 8.0f, DurPercent, DurColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.5f));

	if (Weapon->IsBroken())
	{
		DrawText(TEXT("BROKEN"), SEEUIStyle::DangerRed, X + 160.0f, Y + 14.0f);
	}
}

void ASEEHUD::DrawCrosshair()
{
	float CenterX = Canvas->SizeX * 0.5f;
	float CenterY = Canvas->SizeY * 0.5f;
	float Size = 4.0f;
	float Gap = 3.0f;

	FLinearColor Color = SEEUIStyle::TextPrimary.CopyWithNewOpacity(0.6f);

	// Center dot
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
