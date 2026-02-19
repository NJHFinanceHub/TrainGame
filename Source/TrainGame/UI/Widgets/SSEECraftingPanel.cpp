// SSEECraftingPanel.cpp - Crafting interface implementation
#include "SSEECraftingPanel.h"

#include "SnowyEngine/Crafting/CraftingComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEECraftingPanel::Construct(const FArguments& InArgs)
{
	CraftingComp = InArgs._CraftingComponent;
	InventoryComp = InArgs._InventoryComponent;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.88f))
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

			// Station header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				MakeStationHeader()
			]

			// Main content: recipe list (left) + detail (right)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				// Recipe list
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(0, 0, 8, 0)
				[
					MakeRecipeList()
				]

				// Recipe detail + craft button
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						MakeRecipeDetail()
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 8, 0, 0)
					[
						MakeCraftButton()
					]
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEECraftingPanel::MakeStationHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Crafting", "CRAFTING"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return GetStationName(); })
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.6f, 0.4f)))
		];
}

TSharedRef<SWidget> SSEECraftingPanel::MakeRecipeList()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (!CraftingComp.IsValid())
						{
							return NSLOCTEXT("HUD", "NoStation", "No crafting station");
						}

						TArray<FCraftingRecipe> Recipes = CraftingComp->GetAvailableRecipes();
						if (Recipes.Num() == 0)
						{
							return NSLOCTEXT("HUD", "NoRecipes", "No recipes available at this station");
						}

						FString Result;
						for (const FCraftingRecipe& Recipe : Recipes)
						{
							ECraftResult CanCraftResult = CraftingComp->CanCraft(Recipe.RecipeID);
							FString StatusIcon = (CanCraftResult == ECraftResult::Success) ? TEXT("[+]") : TEXT("[-]");

							Result += FString::Printf(TEXT("%s %s\n"),
								*StatusIcon,
								*Recipe.DisplayName.ToString());

							// Show scrap cost
							if (Recipe.ScrapCost > 0)
							{
								Result += FString::Printf(TEXT("    Scrap: %d"), Recipe.ScrapCost);
								if (InventoryComp.IsValid())
								{
									Result += FString::Printf(TEXT(" (have: %d)"), InventoryComp->GetScrap());
								}
								Result += TEXT("\n");
							}

							// Show ingredients
							for (const FCraftingIngredient& Ing : Recipe.Ingredients)
							{
								int32 Have = InventoryComp.IsValid() ? InventoryComp->GetItemCount(Ing.ItemID) : 0;
								FString ConsumedStr = Ing.bConsumed ? TEXT("") : TEXT(" [tool]");
								Result += FString::Printf(TEXT("    %s: %d/%d%s\n"),
									*Ing.ItemID.ToString(),
									Have, Ing.Count,
									*ConsumedStr);
							}
							Result += TEXT("\n");
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				]
			]
		];
}

TSharedRef<SWidget> SSEECraftingPanel::MakeRecipeDetail()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "RecipeDetail", "RECIPE DETAILS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (SelectedRecipeID.IsNone() || !CraftingComp.IsValid())
					{
						return NSLOCTEXT("HUD", "SelectRecipe", "Select a recipe to view details");
					}

					FCraftingRecipe Recipe;
					if (!CraftingComp->GetRecipe(SelectedRecipeID, Recipe))
					{
						return NSLOCTEXT("HUD", "RecipeNotFound", "Recipe not found");
					}

					FString Detail;
					Detail += Recipe.DisplayName.ToString() + TEXT("\n\n");
					Detail += Recipe.Description.ToString() + TEXT("\n\n");
					Detail += FString::Printf(TEXT("Output: %s x%d\n"), *Recipe.OutputItemID.ToString(), Recipe.OutputCount);

					if (Recipe.bRequiresBlueprint)
					{
						bool bHas = CraftingComp->HasDiscoveredBlueprint(Recipe.RecipeID);
						Detail += bHas ? TEXT("Blueprint: Discovered\n") : TEXT("Blueprint: NOT FOUND\n");
					}

					return FText::FromString(Detail);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SSEECraftingPanel::MakeCraftButton()
{
	return SNew(SButton)
		.IsEnabled_Lambda([this]()
		{
			if (SelectedRecipeID.IsNone() || !CraftingComp.IsValid())
			{
				return false;
			}
			return CraftingComp->CanCraft(SelectedRecipeID) == ECraftResult::Success;
		})
		.OnClicked_Lambda([this]()
		{
			if (CraftingComp.IsValid() && !SelectedRecipeID.IsNone())
			{
				CraftingComp->Craft(SelectedRecipeID);
			}
			return FReply::Handled();
		})
		.ButtonColorAndOpacity_Lambda([this]()
		{
			bool bCanCraft = CraftingComp.IsValid()
				&& !SelectedRecipeID.IsNone()
				&& CraftingComp->CanCraft(SelectedRecipeID) == ECraftResult::Success;
			return bCanCraft
				? FLinearColor(0.2f, 0.35f, 0.15f)
				: FLinearColor(0.12f, 0.12f, 0.12f);
		})
		[
			SNew(SBox)
			.HAlign(HAlign_Center)
			.Padding(FMargin(16, 8))
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "CraftBtn", "CRAFT"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
			]
		];
}

FText SSEECraftingPanel::GetStationName() const
{
	if (!CraftingComp.IsValid())
	{
		return FText::GetEmpty();
	}

	switch (CraftingComp->GetCurrentStation())
	{
	case ECraftingStationType::ImprovisedWorkbench: return NSLOCTEXT("HUD", "StImprov", "Improvised Workbench");
	case ECraftingStationType::MachineShop:         return NSLOCTEXT("HUD", "StMachine", "Machine Shop");
	case ECraftingStationType::ChemistryStation:    return NSLOCTEXT("HUD", "StChem", "Chemistry Station");
	case ECraftingStationType::ElectronicsBench:    return NSLOCTEXT("HUD", "StElec", "Electronics Bench");
	case ECraftingStationType::ArmorersStation:     return NSLOCTEXT("HUD", "StArmor", "Armorer's Station");
	case ECraftingStationType::TailorsStation:      return NSLOCTEXT("HUD", "StTailor", "Tailor's Station");
	case ECraftingStationType::FieldCrafting:       return NSLOCTEXT("HUD", "StField", "Field Crafting");
	default: return FText::GetEmpty();
	}
}

FText SSEECraftingPanel::GetCraftResultText(ECraftResult Result) const
{
	switch (Result)
	{
	case ECraftResult::Success:            return NSLOCTEXT("HUD", "CRSuccess", "Ready to craft");
	case ECraftResult::MissingIngredients: return NSLOCTEXT("HUD", "CRMissIng", "Missing ingredients");
	case ECraftResult::MissingScrap:       return NSLOCTEXT("HUD", "CRMissScrap", "Not enough scrap");
	case ECraftResult::WrongStation:       return NSLOCTEXT("HUD", "CRWrongSt", "Wrong station type");
	case ECraftResult::MissingBlueprint:   return NSLOCTEXT("HUD", "CRMissBP", "Blueprint not discovered");
	case ECraftResult::StatsTooLow:        return NSLOCTEXT("HUD", "CRStatLow", "Stats too low");
	case ECraftResult::InventoryFull:      return NSLOCTEXT("HUD", "CRInvFull", "Inventory full");
	default: return FText::GetEmpty();
	}
}
