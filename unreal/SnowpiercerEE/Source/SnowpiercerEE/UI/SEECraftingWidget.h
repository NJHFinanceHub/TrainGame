// SEECraftingWidget.h - Crafting screen: recipes, materials, station requirements
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SnowyEngine/Crafting/CraftingTypes.h"
#include "SEECraftingWidget.generated.h"

class UCraftingComponent;
class USEEInventoryComponent;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEECraftingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void InitCrafting(UCraftingComponent* InCrafting, USEEInventoryComponent* InInventory);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RefreshRecipes();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SelectRecipe(FName RecipeID);

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void CraftSelected();

	UFUNCTION(BlueprintPure, Category = "Crafting")
	TArray<FCraftingRecipe> GetAvailableRecipes() const;

	UFUNCTION(BlueprintPure, Category = "Crafting")
	FName GetSelectedRecipeID() const { return SelectedRecipeID; }

	UFUNCTION(BlueprintPure, Category = "Crafting")
	FText GetStationName(ECraftingStationType Station) const;

	UFUNCTION(BlueprintPure, Category = "Crafting")
	FText GetCraftResultMessage(ECraftResult Result) const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RecipeDescText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> IngredientsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OutputText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RequirementsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CraftStatusText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StationText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void OnRecipesRefreshed(const TArray<FCraftingRecipe>& Recipes);

	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void OnRecipeSelected(const FCraftingRecipe& Recipe, ECraftResult CanCraftResult);

	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void OnCraftAttempted(FName RecipeID, ECraftResult Result);

private:
	void UpdateRecipeDetails();

	TWeakObjectPtr<UCraftingComponent> CraftingComp;
	TWeakObjectPtr<USEEInventoryComponent> InventoryComp;
	FName SelectedRecipeID;
};
