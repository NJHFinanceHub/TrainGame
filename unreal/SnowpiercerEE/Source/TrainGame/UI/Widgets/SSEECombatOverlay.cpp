// SSEECombatOverlay.cpp - Combat overlay implementation
#include "SSEECombatOverlay.h"

#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/Images/SImage.h"

void SSEECombatOverlay::Construct(const FArguments& InArgs)
{
	CombatComp = InArgs._CombatComponent;
	WeaponComp = InArgs._WeaponComponent;

	ChildSlot
	[
		SNew(SOverlay)

		// Kronole mode full-screen flash
		+ SOverlay::Slot()
		[
			MakeKronoleFlash()
		]

		// Weapon display (bottom-right)
		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0, 0, 16, 80)
		[
			MakeWeaponDisplay()
		]

		// Stance indicator (top-center)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0, 16, 0, 0)
		[
			MakeStanceIndicator()
		]

		// Fatigue warning (center)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			MakeFatigueWarning()
		]
	];
}

void SSEECombatOverlay::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Animate Kronole flash
	if (CombatComp.IsValid() && CombatComp->IsInKronoleMode())
	{
		KronoleFlashAlpha = FMath::FInterpTo(KronoleFlashAlpha, 0.15f, InDeltaTime, 8.0f);
	}
	else
	{
		KronoleFlashAlpha = FMath::FInterpTo(KronoleFlashAlpha, 0.0f, InDeltaTime, 4.0f);
	}
}

TSharedRef<SWidget> SSEECombatOverlay::MakeWeaponDisplay()
{
	return SNew(SBox)
		.WidthOverride(180.0f)
		.Visibility_Lambda([this]()
		{
			return (WeaponComp.IsValid() && WeaponComp->HasWeaponEquipped())
				? EVisibility::SelfHitTestInvisible
				: EVisibility::Collapsed;
		})
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 0.75f))
			.Padding(8.0f)
			[
				SNew(SVerticalBox)

				// Weapon name
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 4)
				[
					SNew(STextBlock)
					.Text_Lambda([this]() { return GetWeaponNameText(); })
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
				]

				// Durability bar
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.HeightOverride(8.0f)
					[
						SNew(SProgressBar)
						.Percent_Lambda([this]() { return GetWeaponDurabilityPercent(); })
						.FillColorAndOpacity_Lambda([this]() { return GetDurabilityColor(); })
						.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
						.BorderPadding(FVector2D(0, 0))
					]
				]

				// "BROKEN" text when durability is 0
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2, 0, 0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "WeaponBroken", "BROKEN"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.2f, 0.1f)))
					.Visibility_Lambda([this]()
					{
						return (WeaponComp.IsValid() && WeaponComp->IsWeaponBroken())
							? EVisibility::SelfHitTestInvisible
							: EVisibility::Collapsed;
					})
				]
			]
		];
}

TSharedRef<SWidget> SSEECombatOverlay::MakeStanceIndicator()
{
	return SNew(SBox)
		.Visibility_Lambda([this]()
		{
			if (!CombatComp.IsValid())
			{
				return EVisibility::Collapsed;
			}
			ECombatStance Stance = CombatComp->GetCurrentStance();
			return (Stance != ECombatStance::Neutral)
				? EVisibility::SelfHitTestInvisible
				: EVisibility::Collapsed;
		})
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 0.6f))
			.Padding(FMargin(12, 4))
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return GetStanceText(); })
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity_Lambda([this]() { return GetStanceColor(); })
				.Justification(ETextJustify::Center)
			]
		];
}

TSharedRef<SWidget> SSEECombatOverlay::MakeKronoleFlash()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor_Lambda([this]()
		{
			return FLinearColor(0.3f, 0.8f, 0.2f, KronoleFlashAlpha);
		})
		.Visibility_Lambda([this]()
		{
			return KronoleFlashAlpha > 0.01f
				? EVisibility::HitTestInvisible
				: EVisibility::Collapsed;
		});
}

TSharedRef<SWidget> SSEECombatOverlay::MakeFatigueWarning()
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("HUD", "Fatigue", "FATIGUED"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
		.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.6f, 0.1f, 0.8f)))
		.Visibility_Lambda([this]()
		{
			if (!CombatComp.IsValid())
			{
				return EVisibility::Collapsed;
			}
			// Show when fatigue exceeds 50% of max stamina
			return CombatComp->GetFatigueLevel() > 50.0f
				? EVisibility::SelfHitTestInvisible
				: EVisibility::Collapsed;
		});
}

FText SSEECombatOverlay::GetWeaponNameText() const
{
	if (!WeaponComp.IsValid() || !WeaponComp->HasWeaponEquipped())
	{
		return FText::GetEmpty();
	}

	const FWeaponStats& Weapon = WeaponComp->GetCurrentWeapon();
	return FText::FromName(Weapon.WeaponName);
}

float SSEECombatOverlay::GetWeaponDurabilityPercent() const
{
	if (!WeaponComp.IsValid())
	{
		return 0.0f;
	}
	return WeaponComp->GetDurabilityPercent() / 100.0f; // Component returns 0-100, bar wants 0-1
}

FLinearColor SSEECombatOverlay::GetDurabilityColor() const
{
	float Percent = GetWeaponDurabilityPercent();

	if (Percent < 0.25f)
	{
		return FLinearColor(1.0f, 0.2f, 0.1f); // Red - about to break
	}
	if (Percent < 0.50f)
	{
		return FLinearColor(1.0f, 0.6f, 0.1f); // Orange - worn
	}
	return FLinearColor(0.5f, 0.8f, 0.3f); // Green - good condition
}

FText SSEECombatOverlay::GetStanceText() const
{
	if (!CombatComp.IsValid())
	{
		return FText::GetEmpty();
	}

	switch (CombatComp->GetCurrentStance())
	{
	case ECombatStance::Attacking:    return NSLOCTEXT("HUD", "StAttack", "ATTACKING");
	case ECombatStance::Blocking:     return NSLOCTEXT("HUD", "StBlock", "BLOCKING");
	case ECombatStance::Dodging:      return NSLOCTEXT("HUD", "StDodge", "DODGING");
	case ECombatStance::Staggered:    return NSLOCTEXT("HUD", "StStagger", "STAGGERED");
	case ECombatStance::Downed:       return NSLOCTEXT("HUD", "StDown", "DOWNED");
	case ECombatStance::KronoleMode:  return NSLOCTEXT("HUD", "StKronole", "KRONOLE MODE");
	default:                          return FText::GetEmpty();
	}
}

FSlateColor SSEECombatOverlay::GetStanceColor() const
{
	if (!CombatComp.IsValid())
	{
		return FSlateColor(FLinearColor::White);
	}

	switch (CombatComp->GetCurrentStance())
	{
	case ECombatStance::Attacking:    return FSlateColor(FLinearColor(1.0f, 0.4f, 0.2f));
	case ECombatStance::Blocking:     return FSlateColor(FLinearColor(0.3f, 0.6f, 1.0f));
	case ECombatStance::Dodging:      return FSlateColor(FLinearColor(0.9f, 0.9f, 0.3f));
	case ECombatStance::Staggered:    return FSlateColor(FLinearColor(1.0f, 0.2f, 0.2f));
	case ECombatStance::Downed:       return FSlateColor(FLinearColor(0.6f, 0.1f, 0.1f));
	case ECombatStance::KronoleMode:  return FSlateColor(FLinearColor(0.3f, 1.0f, 0.4f));
	default:                          return FSlateColor(FLinearColor::White);
	}
}
