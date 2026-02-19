// SSEECraftingPanel.h - Crafting interface with recipe list and station filtering
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SnowyEngine/Crafting/CraftingTypes.h"

class UCraftingComponent;
class UInventoryComponent;

/**
 * SSEECraftingPanel
 *
 * Full-screen crafting interface showing:
 * - Current station type indicator
 * - Recipe list (filtered by station, showing only known recipes)
 * - Ingredient requirements with have/need counts
 * - Scrap cost
 * - Craft button (enabled when CanCraft returns Success)
 * - Blueprint discovery status for gated recipes
 *
 * Opens when the player interacts with a crafting station.
 */
class SSEECraftingPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECraftingPanel)
		: _CraftingComponent(nullptr)
		, _InventoryComponent(nullptr)
	{}
		SLATE_ARGUMENT(UCraftingComponent*, CraftingComponent)
		SLATE_ARGUMENT(UInventoryComponent*, InventoryComponent)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> MakeStationHeader();
	TSharedRef<SWidget> MakeRecipeList();
	TSharedRef<SWidget> MakeRecipeDetail();
	TSharedRef<SWidget> MakeCraftButton();

	FText GetStationName() const;
	FText GetCraftResultText(ECraftResult Result) const;

	TWeakObjectPtr<UCraftingComponent> CraftingComp;
	TWeakObjectPtr<UInventoryComponent> InventoryComp;

	FName SelectedRecipeID;
};
