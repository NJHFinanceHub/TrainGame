// SEECraftingWidget.cpp
#include "SEECraftingWidget.h"
#include "Crafting/CraftingComponent.h"
#include "SEEInventoryComponent.h"
#include "Components/TextBlock.h"

void USEECraftingWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USEECraftingWidget::InitCrafting(UCraftingComponent* InCrafting, USEEInventoryComponent* InInventory)
{
	CraftingComp = InCrafting;
	InventoryComp = InInventory;

	if (StationText && CraftingComp.IsValid())
	{
		StationText->SetText(GetStationName(CraftingComp->GetCurrentStation()));
	}

	RefreshRecipes();
}

void USEECraftingWidget::RefreshRecipes()
{
	TArray<FCraftingRecipe> Recipes = GetAvailableRecipes();
	OnRecipesRefreshed(Recipes);

	if (!SelectedRecipeID.IsNone())
	{
		UpdateRecipeDetails();
	}
}

TArray<FCraftingRecipe> USEECraftingWidget::GetAvailableRecipes() const
{
	if (!CraftingComp.IsValid()) return {};
	return CraftingComp->GetAvailableRecipes();
}

void USEECraftingWidget::SelectRecipe(FName RecipeID)
{
	SelectedRecipeID = RecipeID;
	UpdateRecipeDetails();
}

void USEECraftingWidget::UpdateRecipeDetails()
{
	if (!CraftingComp.IsValid() || SelectedRecipeID.IsNone()) return;

	FCraftingRecipe Recipe;
	if (!CraftingComp->GetRecipe(SelectedRecipeID, Recipe)) return;

	ECraftResult CanCraft = CraftingComp->CanCraft(SelectedRecipeID);

	if (RecipeNameText)
	{
		RecipeNameText->SetText(Recipe.DisplayName);
	}

	if (RecipeDescText)
	{
		RecipeDescText->SetText(Recipe.Description);
	}

	if (IngredientsText)
	{
		FString IngStr;
		for (const FCraftingIngredient& Ing : Recipe.Ingredients)
		{
			int32 Have = InventoryComp.IsValid() ? InventoryComp->GetItemCount(Ing.ItemID) : 0;
			bool bEnough = Have >= Ing.Count;
			IngStr += FString::Printf(TEXT("%s%s x%d (%d/%d)\n"),
				bEnough ? TEXT("") : TEXT("! "),
				*Ing.ItemID.ToString(),
				Ing.Count,
				Have,
				Ing.Count);
		}
		if (Recipe.ScrapCost > 0)
		{
			IngStr += FString::Printf(TEXT("Scrap: %d\n"), Recipe.ScrapCost);
		}
		IngredientsText->SetText(FText::FromString(IngStr));
	}

	if (OutputText)
	{
		OutputText->SetText(FText::FromString(
			FString::Printf(TEXT("Output: %s x%d"), *Recipe.OutputItemID.ToString(), Recipe.OutputCount)));
	}

	if (RequirementsText)
	{
		FString ReqStr;
		ReqStr += FString::Printf(TEXT("Station: %s\n"), *GetStationName(Recipe.RequiredStation).ToString());

		for (const auto& Stat : Recipe.StatRequirements)
		{
			ReqStr += FString::Printf(TEXT("%s >= %.0f\n"), *Stat.Key.ToString(), Stat.Value);
		}

		if (Recipe.bRequiresBlueprint)
		{
			bool bHas = CraftingComp->HasDiscoveredBlueprint(SelectedRecipeID);
			ReqStr += bHas ? TEXT("Blueprint: Discovered\n") : TEXT("Blueprint: MISSING\n");
		}

		RequirementsText->SetText(FText::FromString(ReqStr));
	}

	if (CraftStatusText)
	{
		CraftStatusText->SetText(GetCraftResultMessage(CanCraft));
		FLinearColor StatusColor = CanCraft == ECraftResult::Success
			? FLinearColor::Green
			: FLinearColor(1.0f, 0.5f, 0.0f);
		CraftStatusText->SetColorAndOpacity(FSlateColor(StatusColor));
	}

	OnRecipeSelected(Recipe, CanCraft);
}

void USEECraftingWidget::CraftSelected()
{
	if (!CraftingComp.IsValid() || SelectedRecipeID.IsNone()) return;

	ECraftResult Result = CraftingComp->Craft(SelectedRecipeID);
	OnCraftAttempted(SelectedRecipeID, Result);
	RefreshRecipes();
}

FText USEECraftingWidget::GetStationName(ECraftingStationType Station) const
{
	switch (Station)
	{
	case ECraftingStationType::ImprovisedWorkbench: return NSLOCTEXT("Craft", "Improvised", "Improvised Workbench");
	case ECraftingStationType::MachineShop: return NSLOCTEXT("Craft", "Machine", "Machine Shop");
	case ECraftingStationType::ChemistryStation: return NSLOCTEXT("Craft", "Chemistry", "Chemistry Station");
	case ECraftingStationType::ElectronicsBench: return NSLOCTEXT("Craft", "Electronics", "Electronics Bench");
	case ECraftingStationType::ArmorersStation: return NSLOCTEXT("Craft", "Armorer", "Armorer's Station");
	case ECraftingStationType::TailorsStation: return NSLOCTEXT("Craft", "Tailor", "Tailor's Station");
	case ECraftingStationType::FieldCrafting: return NSLOCTEXT("Craft", "Field", "Field Crafting");
	default: return FText::GetEmpty();
	}
}

FText USEECraftingWidget::GetCraftResultMessage(ECraftResult Result) const
{
	switch (Result)
	{
	case ECraftResult::Success: return NSLOCTEXT("Craft", "Ready", "Ready to Craft");
	case ECraftResult::MissingIngredients: return NSLOCTEXT("Craft", "MissIng", "Missing Ingredients");
	case ECraftResult::MissingScrap: return NSLOCTEXT("Craft", "MissScrap", "Insufficient Scrap");
	case ECraftResult::WrongStation: return NSLOCTEXT("Craft", "WrongStn", "Wrong Station");
	case ECraftResult::MissingBlueprint: return NSLOCTEXT("Craft", "MissBP", "Blueprint Not Found");
	case ECraftResult::StatsTooLow: return NSLOCTEXT("Craft", "StatsLow", "Stats Too Low");
	case ECraftResult::InventoryFull: return NSLOCTEXT("Craft", "InvFull", "Inventory Full");
	default: return FText::GetEmpty();
	}
}
