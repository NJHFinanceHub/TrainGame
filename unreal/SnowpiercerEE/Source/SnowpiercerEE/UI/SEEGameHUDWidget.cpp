// SEEGameHUDWidget.cpp
#include "SEEGameHUDWidget.h"
#include "SEECharacter.h"
#include "SEEHealthComponent.h"
#include "SEEHungerComponent.h"
#include "SEEColdComponent.h"
#include "SEECombatComponent.h"
#include "SEEStatsComponent.h"
#include "SEEWeaponBase.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void USEEGameHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (InteractionText)
	{
		InteractionText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (NotificationText)
	{
		NotificationText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void USEEGameHUDWidget::InitializeHUD(ASEECharacter* Character)
{
	if (!Character) return;

	CachedCharacter = Character;
	HealthComp = Character->FindComponentByClass<USEEHealthComponent>();
	HungerComp = Character->FindComponentByClass<USEEHungerComponent>();
	ColdComp = Character->FindComponentByClass<USEEColdComponent>();
	CombatComp = Character->FindComponentByClass<USEECombatComponent>();
	StatsComp = Character->FindComponentByClass<USEEStatsComponent>();

	OnHUDInitialized();
}

void USEEGameHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CachedCharacter.IsValid()) return;

	UpdateHealthDisplay();
	UpdateStaminaDisplay();
	UpdateHungerDisplay();
	UpdateColdDisplay();
	UpdateWeaponDisplay();
	UpdateXPDisplay();
	UpdateDamageVignette(InDeltaTime);

	// Notification timer
	if (NotificationTimer > 0.0f)
	{
		NotificationTimer -= InDeltaTime;
		if (NotificationTimer <= 0.0f && NotificationText)
		{
			NotificationText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USEEGameHUDWidget::UpdateHealthDisplay()
{
	if (!HealthComp.IsValid()) return;

	float Percent = HealthComp->GetHealthPercent();

	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
		FLinearColor Color = FMath::Lerp(FLinearColor::Red, FLinearColor::Green, Percent);
		HealthBar->SetFillColorAndOpacity(Color);
	}

	if (HealthText)
	{
		HealthText->SetText(FText::Format(
			NSLOCTEXT("HUD", "HP", "HP: {0}/{1}"),
			FText::AsNumber(FMath::RoundToInt32(HealthComp->GetCurrentHealth())),
			FText::AsNumber(FMath::RoundToInt32(HealthComp->GetMaxHealth()))));
	}

	if (InjuryText)
	{
		TArray<FSEEInjury> Injuries = HealthComp->GetActiveInjuries();
		if (Injuries.Num() > 0)
		{
			FString Combined;
			for (const FSEEInjury& Injury : Injuries)
			{
				switch (Injury.Type)
				{
				case ESEEInjuryType::BrokenArm: Combined += TEXT("[Broken Arm] "); break;
				case ESEEInjuryType::LegWound: Combined += TEXT("[Leg Wound] "); break;
				case ESEEInjuryType::Concussion: Combined += TEXT("[Concussion] "); break;
				case ESEEInjuryType::DeepCut: Combined += TEXT("[Deep Cut] "); break;
				case ESEEInjuryType::InternalBleeding: Combined += TEXT("[Bleeding] "); break;
				default: break;
				}
			}
			InjuryText->SetText(FText::FromString(Combined));
			InjuryText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			InjuryText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void USEEGameHUDWidget::UpdateStaminaDisplay()
{
	if (!CachedCharacter.IsValid() || !StaminaBar) return;

	float Percent = CachedCharacter->GetStamina() / CachedCharacter->GetMaxStamina();
	StaminaBar->SetPercent(Percent);

	FLinearColor Color = Percent < 0.25f
		? FLinearColor(1.0f, 0.8f, 0.0f)
		: FLinearColor(0.2f, 0.6f, 1.0f);
	StaminaBar->SetFillColorAndOpacity(Color);
}

void USEEGameHUDWidget::UpdateHungerDisplay()
{
	if (!HungerComp.IsValid()) return;

	float Percent = HungerComp->GetHungerPercent();
	bool bShow = Percent < 0.75f;

	if (HungerBar)
	{
		HungerBar->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShow)
		{
			HungerBar->SetPercent(Percent);
			FLinearColor Color = Percent > 0.25f
				? FLinearColor(0.8f, 0.6f, 0.2f)
				: FLinearColor(1.0f, 0.2f, 0.0f);
			HungerBar->SetFillColorAndOpacity(Color);
		}
	}

	if (HungerText)
	{
		HungerText->SetVisibility(bShow ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bShow)
		{
			HungerText->SetText(FText::Format(
				NSLOCTEXT("HUD", "Hunger", "Hunger: {0}%"),
				FText::AsNumber(FMath::RoundToInt32(Percent * 100.0f))));
		}
	}
}

void USEEGameHUDWidget::UpdateColdDisplay()
{
	if (!ColdComp.IsValid() || !ColdText) return;

	if (!ColdComp->IsInColdZone())
	{
		ColdText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ColdText->SetVisibility(ESlateVisibility::HitTestInvisible);
	ESEEFrostbiteStage Stage = ColdComp->GetFrostbiteStage();
	float Temp = ColdComp->GetTemperature();

	FString StageStr;
	FSlateColor Color;
	switch (Stage)
	{
	case ESEEFrostbiteStage::Shivers:
		StageStr = TEXT("Shivers");
		Color = FSlateColor(FLinearColor(0.5f, 0.7f, 1.0f));
		break;
	case ESEEFrostbiteStage::Numbness:
		StageStr = TEXT("Numbness -40% speed");
		Color = FSlateColor(FLinearColor(0.3f, 0.4f, 1.0f));
		break;
	case ESEEFrostbiteStage::Blackout:
		StageStr = TEXT("FROSTBITE - SEEK WARMTH");
		Color = FSlateColor(FLinearColor(0.8f, 0.2f, 1.0f));
		break;
	default:
		StageStr = TEXT("Cold Zone");
		Color = FSlateColor(FLinearColor(0.6f, 0.8f, 1.0f));
		break;
	}

	ColdText->SetText(FText::FromString(FString::Printf(TEXT("COLD: %.0f C - %s"), Temp, *StageStr)));
	ColdText->SetColorAndOpacity(Color);
}

void USEEGameHUDWidget::UpdateWeaponDisplay()
{
	if (!CombatComp.IsValid()) return;

	ASEEWeaponBase* Weapon = CombatComp->GetEquippedWeapon();
	bool bHasWeapon = Weapon != nullptr;

	if (WeaponNameText)
	{
		WeaponNameText->SetVisibility(bHasWeapon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bHasWeapon)
		{
			FString Name = Weapon->WeaponName.ToString();
			if (Weapon->IsBroken())
			{
				Name += TEXT(" [BROKEN]");
			}
			WeaponNameText->SetText(FText::FromString(Name));
		}
	}

	if (DurabilityBar)
	{
		DurabilityBar->SetVisibility(bHasWeapon ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		if (bHasWeapon)
		{
			float DurPercent = Weapon->GetDurabilityPercent();
			DurabilityBar->SetPercent(DurPercent);
			FLinearColor DurColor = DurPercent > 0.3f
				? FLinearColor(0.5f, 0.8f, 0.5f)
				: FLinearColor(1.0f, 0.3f, 0.0f);
			DurabilityBar->SetFillColorAndOpacity(DurColor);
		}
	}
}

void USEEGameHUDWidget::UpdateXPDisplay()
{
	if (!StatsComp.IsValid()) return;

	if (LevelText)
	{
		LevelText->SetText(FText::Format(
			NSLOCTEXT("HUD", "Level", "Lv. {0}"),
			FText::AsNumber(StatsComp->GetLevel())));
	}

	if (XPBar)
	{
		int32 XPNeeded = StatsComp->GetXPToNextLevel();
		float XPPercent = XPNeeded > 0
			? static_cast<float>(StatsComp->GetCurrentXP()) / static_cast<float>(XPNeeded)
			: 1.0f;
		XPBar->SetPercent(FMath::Clamp(XPPercent, 0.0f, 1.0f));
	}
}

void USEEGameHUDWidget::SetInteractionPrompt(const FText& Prompt, bool bVisible)
{
	if (!InteractionText) return;

	InteractionText->SetText(Prompt);
	InteractionText->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void USEEGameHUDWidget::ShowDamageDirection(FVector WorldDirection)
{
	LastDamageDir = WorldDirection;
	DamageVignetteAlpha = 1.0f;
}

void USEEGameHUDWidget::UpdateDamageVignette(float DeltaTime)
{
	if (DamageVignetteAlpha <= 0.0f) return;

	DamageVignetteAlpha = FMath::Max(0.0f, DamageVignetteAlpha - DeltaTime * 0.5f);

	auto SetVignette = [this](UImage* Img, float Alpha)
	{
		if (!Img) return;
		if (Alpha > 0.01f)
		{
			Img->SetVisibility(ESlateVisibility::HitTestInvisible);
			Img->SetColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f, Alpha));
		}
		else
		{
			Img->SetVisibility(ESlateVisibility::Collapsed);
		}
	};

	// Determine direction relative to camera
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		SetVignette(DamageVignetteLeft, 0.0f);
		SetVignette(DamageVignetteRight, 0.0f);
		SetVignette(DamageVignetteTop, 0.0f);
		SetVignette(DamageVignetteBottom, 0.0f);
		return;
	}

	FVector LocalDir = PC->GetControlRotation().UnrotateVector(LastDamageDir);
	float Alpha = DamageVignetteAlpha * 0.8f;

	SetVignette(DamageVignetteLeft, LocalDir.Y < -0.3f ? Alpha : 0.0f);
	SetVignette(DamageVignetteRight, LocalDir.Y > 0.3f ? Alpha : 0.0f);
	SetVignette(DamageVignetteTop, LocalDir.X > 0.3f ? Alpha : 0.0f);
	SetVignette(DamageVignetteBottom, LocalDir.X < -0.3f ? Alpha : 0.0f);
}

void USEEGameHUDWidget::ShowNotification(const FText& Message, float Duration)
{
	if (!NotificationText) return;

	NotificationText->SetText(Message);
	NotificationText->SetVisibility(ESlateVisibility::HitTestInvisible);
	NotificationTimer = Duration;

	OnNotificationReceived(Message);
}
