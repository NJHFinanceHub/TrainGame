// SSEECharacterScreen.h - Character stats, skills, level, and equipment overview
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USEEStatsComponent;
class UInventoryComponent;

/**
 * SSEECharacterScreen
 *
 * Full-screen character overview showing:
 * - Level and XP progress bar
 * - Core stats (Strength, Agility, Endurance, Cunning, Perception, Charisma)
 * - Available skill points indicator with spend buttons
 * - Equipped weapon/armor summary (from inventory)
 *
 * Toggled via ASEEGameHUD::ToggleCharacter(). Only one full-screen
 * panel can be open at a time.
 */
class SSEECharacterScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECharacterScreen)
		: _StatsComponent(nullptr)
		, _InventoryComponent(nullptr)
	{}
		SLATE_ARGUMENT(USEEStatsComponent*, StatsComponent)
		SLATE_ARGUMENT(UInventoryComponent*, InventoryComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeLevelBar();
	TSharedRef<SWidget> MakeStatsPanel();
	TSharedRef<SWidget> MakeEquipmentPanel();

	FText GetStatName(uint8 StatIndex) const;
	FLinearColor GetStatBarColor(uint8 StatIndex) const;

	TWeakObjectPtr<USEEStatsComponent> StatsComp;
	TWeakObjectPtr<UInventoryComponent> InventoryComp;
};
