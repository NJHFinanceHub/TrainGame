// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// CraftingComponent.cpp - Crafting system implementation

#include "CraftingComponent.h"
#include "SnowpiercerEE/SEEInventoryComponent.h"

UCraftingComponent::UCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find the inventory component on the same actor
	if (AActor* Owner = GetOwner())
	{
		LinkedInventory = Owner->FindComponentByClass<USEEInventoryComponent>();
	}
}

// --- Station Interaction ---

void UCraftingComponent::SetCurrentStation(ECraftingStationType StationType)
{
	CurrentStation = StationType;
}

// --- Recipe Queries ---

TArray<FCraftingRecipe> UCraftingComponent::GetAvailableRecipes() const
{
	TArray<FCraftingRecipe> Result;
	if (!RecipeDataTable) return Result;

	TArray<FName> RowNames = RecipeDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FCraftingRecipe* Recipe = RecipeDataTable->FindRow<FCraftingRecipe>(RowName, TEXT("GetAvailableRecipes"));
		if (!Recipe) continue;

		// Must match current station (or FieldCrafting recipes are always available)
		if (Recipe->RequiredStation != ECraftingStationType::FieldCrafting &&
			Recipe->RequiredStation != CurrentStation)
		{
			continue;
		}

		// Must have discovered blueprint if required
		if (Recipe->bRequiresBlueprint && !HasDiscoveredBlueprint(Recipe->RecipeID))
		{
			continue;
		}

		Result.Add(*Recipe);
	}

	return Result;
}

ECraftResult UCraftingComponent::CanCraft(FName RecipeID) const
{
	FCraftingRecipe Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		return ECraftResult::MissingIngredients;
	}

	// Station check
	if (Recipe.RequiredStation != ECraftingStationType::FieldCrafting &&
		Recipe.RequiredStation != CurrentStation)
	{
		return ECraftResult::WrongStation;
	}

	// Blueprint check
	if (Recipe.bRequiresBlueprint && !HasDiscoveredBlueprint(RecipeID))
	{
		return ECraftResult::MissingBlueprint;
	}

	if (!LinkedInventory)
	{
		return ECraftResult::MissingIngredients;
	}

	// Ingredient check
	if (!CheckIngredients(Recipe))
	{
		return ECraftResult::MissingIngredients;
	}

	// Weight check for output
	FSEEItemData OutputDef;
	if (LinkedInventory->GetItemData(Recipe.OutputItemID, OutputDef))
	{
		float NewWeight = LinkedInventory->GetCurrentWeight() + (OutputDef.Weight * Recipe.OutputCount);
		if (NewWeight > LinkedInventory->GetMaxWeight())
		{
			return ECraftResult::InventoryFull;
		}
	}

	return ECraftResult::Success;
}

bool UCraftingComponent::GetRecipe(FName RecipeID, FCraftingRecipe& OutRecipe) const
{
	if (!RecipeDataTable) return false;

	TArray<FName> RowNames = RecipeDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FCraftingRecipe* Recipe = RecipeDataTable->FindRow<FCraftingRecipe>(RowName, TEXT("GetRecipe"));
		if (Recipe && Recipe->RecipeID == RecipeID)
		{
			OutRecipe = *Recipe;
			return true;
		}
	}
	return false;
}

// --- Crafting Execution ---

ECraftResult UCraftingComponent::Craft(FName RecipeID)
{
	ECraftResult PreCheck = CanCraft(RecipeID);
	if (PreCheck != ECraftResult::Success)
	{
		OnCraftCompleted.Broadcast(RecipeID, PreCheck);
		return PreCheck;
	}

	FCraftingRecipe Recipe;
	if (!GetRecipe(RecipeID, Recipe))
	{
		return ECraftResult::MissingIngredients;
	}

	// Consume ingredients
	ConsumeIngredients(Recipe);

	// Produce output
	LinkedInventory->AddItem(Recipe.OutputItemID, Recipe.OutputCount);

	OnCraftCompleted.Broadcast(RecipeID, ECraftResult::Success);
	return ECraftResult::Success;
}

// --- Blueprint Discovery ---

void UCraftingComponent::DiscoverBlueprint(FName RecipeID)
{
	DiscoveredBlueprints.Add(RecipeID);
}

bool UCraftingComponent::HasDiscoveredBlueprint(FName RecipeID) const
{
	return DiscoveredBlueprints.Contains(RecipeID);
}

// --- Private ---

bool UCraftingComponent::CheckIngredients(const FCraftingRecipe& Recipe) const
{
	if (!LinkedInventory) return false;

	for (const FCraftingIngredient& Ingredient : Recipe.Ingredients)
	{
		if (!LinkedInventory->HasItem(Ingredient.ItemID, Ingredient.Count))
		{
			return false;
		}
	}
	return true;
}

void UCraftingComponent::ConsumeIngredients(const FCraftingRecipe& Recipe)
{
	if (!LinkedInventory) return;

	for (const FCraftingIngredient& Ingredient : Recipe.Ingredients)
	{
		if (Ingredient.bConsumed)
		{
			LinkedInventory->RemoveItem(Ingredient.ItemID, Ingredient.Count);
		}
	}
}
