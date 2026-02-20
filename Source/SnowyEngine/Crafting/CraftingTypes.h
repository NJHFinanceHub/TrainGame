// CraftingTypes.h - Crafting recipe and station type definitions
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CraftingTypes.generated.h"

// Types of crafting stations found throughout the train
UENUM(BlueprintType)
enum class ECraftingStationType : uint8
{
	ImprovisedWorkbench	UMETA(DisplayName = "Improvised Workbench"),  // Tail
	MachineShop			UMETA(DisplayName = "Machine Shop"),           // Third Class
	ChemistryStation	UMETA(DisplayName = "Chemistry Station"),      // Spine
	ElectronicsBench	UMETA(DisplayName = "Electronics Bench"),      // Spine/Second
	ArmorersStation		UMETA(DisplayName = "Armorer's Station"),      // Armory
	TailorsStation		UMETA(DisplayName = "Tailor's Station"),       // First Class
	FieldCrafting		UMETA(DisplayName = "Field Crafting")          // No station needed
};

// A single ingredient requirement in a recipe
USTRUCT(BlueprintType)
struct FCraftingIngredient
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;

	// If true, the item is consumed. If false, it must be present but isn't used up (tools).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bConsumed = true;
};

// A crafting recipe definition loaded from data tables
USTRUCT(BlueprintType)
struct FCraftingRecipe : public FTableRowBase
{
	GENERATED_BODY()

	// Unique recipe identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RecipeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Required crafting station type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECraftingStationType RequiredStation = ECraftingStationType::FieldCrafting;

	// Ingredients needed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FCraftingIngredient> Ingredients;

	// Scrap cost (consumed from the Scrap resource counter)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScrapCost = 0;

	// Output item
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OutputItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OutputCount = 1;

	// Minimum skill/stat requirements (optional, keyed by stat name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> StatRequirements;

	// Does the player need to have discovered this blueprint first?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresBlueprint = false;

	// Category tag for UI filtering
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CategoryTag;
};

// Result of a craft attempt
UENUM(BlueprintType)
enum class ECraftResult : uint8
{
	Success			UMETA(DisplayName = "Success"),
	MissingIngredients	UMETA(DisplayName = "Missing Ingredients"),
	MissingScrap		UMETA(DisplayName = "Insufficient Scrap"),
	WrongStation		UMETA(DisplayName = "Wrong Station"),
	MissingBlueprint	UMETA(DisplayName = "Blueprint Not Found"),
	StatsTooLow			UMETA(DisplayName = "Stats Too Low"),
	InventoryFull		UMETA(DisplayName = "Inventory Full")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCraftCompleted, FName, RecipeID, ECraftResult, Result);
