// SSEECharacterScreen.h - Player character stats, skills, perks, equipment
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class USEEStatsComponent;
class UInventoryComponent;

/**
 * SSEECharacterScreen
 *
 * Full-screen character panel showing:
 * - Player level and XP progress
 * - Six core stats (STR, AGI, END, CUN, PER, CHA) with skill point allocation
 * - Equipped items summary (weapon, armor)
 * - Skill points available indicator
 *
 * Reads from USEEStatsComponent and UInventoryComponent on the player pawn.
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

	TWeakObjectPtr<USEEStatsComponent> StatsComp;
	TWeakObjectPtr<UInventoryComponent> InventoryComp;
};
