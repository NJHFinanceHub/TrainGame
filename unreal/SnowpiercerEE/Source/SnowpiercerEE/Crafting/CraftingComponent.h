// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// CraftingComponent.h - Crafting system: recipe validation, blueprint discovery, crafting execution

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingTypes.h"
#include "CraftingComponent.generated.h"

class USEEInventoryComponent;

/**
 * UCraftingComponent
 *
 * Manages crafting recipes, blueprint discovery, and craft execution.
 * Works with USEEInventoryComponent to check/consume ingredients and deliver outputs.
 *
 * Recipes are defined in a data table. Some require discovering a blueprint item first.
 * Crafting requires being at the correct station type (or FieldCrafting for no station).
 */
UCLASS(ClassGroup=(Crafting), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingComponent();

	// --- Station Interaction ---

	/** Set the station type the player is currently at. FieldCrafting = no station. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SetCurrentStation(ECraftingStationType StationType);

	UFUNCTION(BlueprintPure, Category = "Crafting")
	ECraftingStationType GetCurrentStation() const { return CurrentStation; }

	// --- Recipe Queries ---

	/** Get all recipes available at the current station that the player knows. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	TArray<FCraftingRecipe> GetAvailableRecipes() const;

	/** Check if a specific recipe can be crafted right now (station, ingredients, stats). */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	ECraftResult CanCraft(FName RecipeID) const;

	/** Get the recipe definition by ID. */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool GetRecipe(FName RecipeID, FCraftingRecipe& OutRecipe) const;

	// --- Crafting Execution ---

	/** Attempt to craft a recipe. Consumes ingredients and produces output on success. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	ECraftResult Craft(FName RecipeID);

	// --- Blueprint Discovery ---

	/** Mark a blueprint recipe as discovered (unlocked for crafting). */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void DiscoverBlueprint(FName RecipeID);

	/** Check if a blueprint recipe has been discovered. */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	bool HasDiscoveredBlueprint(FName RecipeID) const;

	/** Get all discovered blueprint recipe IDs. */
	UFUNCTION(BlueprintPure, Category = "Crafting")
	const TSet<FName>& GetDiscoveredBlueprints() const { return DiscoveredBlueprints; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnCraftCompleted OnCraftCompleted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Crafting|Config")
	UDataTable* RecipeDataTable = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Crafting|Runtime")
	ECraftingStationType CurrentStation = ECraftingStationType::FieldCrafting;

	UPROPERTY(VisibleAnywhere, Category = "Crafting|Runtime")
	TSet<FName> DiscoveredBlueprints;

	UPROPERTY()
	USEEInventoryComponent* LinkedInventory = nullptr;

private:
	bool CheckIngredients(const FCraftingRecipe& Recipe) const;
	void ConsumeIngredients(const FCraftingRecipe& Recipe);
};
