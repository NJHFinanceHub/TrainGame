#include "SEEHUD.h"
#include "SEECharacter.h"
#include "SEEHealthComponent.h"
#include "SEEHungerComponent.h"
#include "SEEColdComponent.h"
#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

ASEEHUD::ASEEHUD()
{
}

void ASEEHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ASEEHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas) return;

	// Cache player references
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

void ASEEHUD::DrawHealthBar()
{
	if (!HealthComp.IsValid()) return;

	float Percent = HealthComp->GetHealthPercent();
	FLinearColor Color = FMath::Lerp(FLinearColor::Red, FLinearColor::Green, Percent);

	DrawBar(20.0f, 20.0f, 200.0f, 20.0f, Percent, Color, FLinearColor(0.1f, 0.1f, 0.1f, 0.7f));

	// HP text
	FString HPText = FString::Printf(TEXT("HP: %d/%d"),
		FMath::RoundToInt32(HealthComp->GetCurrentHealth()),
		FMath::RoundToInt32(HealthComp->GetMaxHealth()));
	DrawText(HPText, FLinearColor::White, 25.0f, 22.0f);

	// Downed indicator
	if (HealthComp->IsDowned())
	{
		DrawText(TEXT("DOWNED - NEED REVIVE"), FLinearColor::Red,
			Canvas->SizeX * 0.5f - 100.0f, Canvas->SizeY * 0.4f);
	}

	// Injury indicators
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
	if (Percent >= 0.75f) return; // Only show when below 75%

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

	// Durability bar
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
	// Simple centered bottom prompt when looking at interactable
	// This will be expanded with actual interaction detection
	// For now just show "Press E to interact" placeholder when close to objects
}

void ASEEHUD::DrawCrosshair()
{
	float CenterX = Canvas->SizeX * 0.5f;
	float CenterY = Canvas->SizeY * 0.5f;
	float Size = 4.0f;
	float Gap = 3.0f;

	FLinearColor Color = FLinearColor(0.8f, 0.8f, 0.8f, 0.6f);

	// Dot crosshair
	DrawRect(Color, CenterX - 1.0f, CenterY - 1.0f, 2.0f, 2.0f);

	// Small cross lines
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

	// Simple red vignette on damage side
	float CenterX = Canvas->SizeX * 0.5f;
	float CenterY = Canvas->SizeY * 0.5f;

	// Convert world damage direction to screen-relative
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	FRotator ViewRot = PC->GetControlRotation();
	FVector LocalDir = ViewRot.UnrotateVector(LastDamageDirection);

	float VignetteSize = 100.0f;
	if (LocalDir.X < -0.3f) // From behind
	{
		DrawRect(VignetteColor, CenterX - VignetteSize, Canvas->SizeY - 30.0f, VignetteSize * 2.0f, 30.0f);
	}
	if (LocalDir.Y > 0.3f) // From right
	{
		DrawRect(VignetteColor, Canvas->SizeX - 30.0f, CenterY - VignetteSize, 30.0f, VignetteSize * 2.0f);
	}
	if (LocalDir.Y < -0.3f) // From left
	{
		DrawRect(VignetteColor, 0.0f, CenterY - VignetteSize, 30.0f, VignetteSize * 2.0f);
	}
	if (LocalDir.X > 0.3f) // From front
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
