// SSEECombatOverlay.h - Slate widget for combat HUD (weapon, stance, kronole mode)
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/Core/CombatTypes.h"

class UCombatComponent;
class UWeaponComponent;

/**
 * SSEECombatOverlay
 *
 * Displays combat-relevant information overlaid on the viewport:
 * - Weapon name and durability bar (bottom-right)
 * - Current combat stance indicator
 * - Kronole mode flash overlay when active
 * - Hit direction indicator (brief flash showing attack direction)
 * - Fatigue warning when fatigue is high
 *
 * This widget is always present but many elements are conditionally visible
 * (e.g., stance only shows during combat, weapon only when armed).
 */
class SSEECombatOverlay : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECombatOverlay)
		: _CombatComponent(nullptr)
		, _WeaponComponent(nullptr)
	{}
		SLATE_ARGUMENT(UCombatComponent*, CombatComponent)
		SLATE_ARGUMENT(UWeaponComponent*, WeaponComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	TSharedRef<SWidget> MakeWeaponDisplay();
	TSharedRef<SWidget> MakeStanceIndicator();
	TSharedRef<SWidget> MakeKronoleFlash();
	TSharedRef<SWidget> MakeFatigueWarning();

	FText GetWeaponNameText() const;
	float GetWeaponDurabilityPercent() const;
	FLinearColor GetDurabilityColor() const;
	FText GetStanceText() const;
	FSlateColor GetStanceColor() const;

	TWeakObjectPtr<UCombatComponent> CombatComp;
	TWeakObjectPtr<UWeaponComponent> WeaponComp;

	float KronoleFlashAlpha = 0.0f;
};
