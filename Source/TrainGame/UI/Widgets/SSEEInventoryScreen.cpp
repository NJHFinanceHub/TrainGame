// SSEEInventoryScreen.cpp - Inventory screen implementation
#include "SSEEInventoryScreen.h"

#include "SnowyEngine/Inventory/InventoryComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEEInventoryScreen::Construct(const FArguments& InArgs)
{
	InventoryComp = InArgs._InventoryComponent;

	ChildSlot
	[
		// Full-screen semi-transparent backdrop
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

			// Title bar
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				MakeHeader()
			]

			// Category tabs
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				MakeCategoryTabs()
			]

			// Main content: item list (left) + detail panel (right)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				// Item list
				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				.Padding(0, 0, 8, 0)
				[
					MakeItemList()
				]

				// Detail panel
				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				[
					MakeDetailPanel()
				]
			]

			// Footer: weight bar + resources
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				.Padding(0, 0, 8, 0)
				[
					MakeWeightBar()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				[
					MakeResourceCounters()
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Inventory", "INVENTORY"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		// Item count
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!InventoryComp.IsValid()) return FText::GetEmpty();
				int32 Count = InventoryComp->GetAllItems().Num();
				return FText::Format(NSLOCTEXT("HUD", "ItemCount", "{0} items"), FText::AsNumber(Count));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeCategoryTabs()
{
	struct FCategoryEntry
	{
		FText Label;
		uint8 Value; // 255 = All
	};

	// Define tab entries - stored as local data, captured by value in lambdas
	auto MakeTab = [this](const FText& Label, uint8 CategoryValue) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, CategoryValue]()
			{
				bool bActive = static_cast<uint8>(ActiveCategory) == CategoryValue;
				return bActive
					? FLinearColor(0.3f, 0.25f, 0.15f)
					: FLinearColor(0.1f, 0.1f, 0.12f);
			})
			.OnClicked_Lambda([this, CategoryValue]()
			{
				ActiveCategory = static_cast<EItemCategory>(CategoryValue);
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
				.Margin(FMargin(8, 4))
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatAll", "All"), 255) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatWeapon", "Weapons"), static_cast<uint8>(EItemCategory::Weapon)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatArmor", "Armor"), static_cast<uint8>(EItemCategory::Armor)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatConsume", "Consumables"), static_cast<uint8>(EItemCategory::Consumable)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatCraft", "Materials"), static_cast<uint8>(EItemCategory::Crafting)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatQuest", "Quest"), static_cast<uint8>(EItemCategory::Quest)) ];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeItemList()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(4.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				// Items are generated dynamically via the Tick/Paint cycle
				// using GetFilteredItems(). In production, this would use SListView
				// for virtualization. For this scaffold, we rebuild each frame.
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						TArray<FInventoryItem> Items = GetFilteredItems();
						if (Items.Num() == 0)
						{
							return NSLOCTEXT("HUD", "EmptyInv", "No items");
						}

						// Build a simple text list (production would use SListView with item widgets)
						FString Result;
						for (int32 i = 0; i < Items.Num(); ++i)
						{
							const FInventoryItem& Item = Items[i];
							FText Name = GetItemDisplayName(Item.ItemID);
							if (Item.StackCount > 1)
							{
								Result += FString::Printf(TEXT("%s x%d\n"), *Name.ToString(), Item.StackCount);
							}
							else
							{
								Result += Name.ToString() + TEXT("\n");
							}

							// Show durability inline if applicable
							if (Item.CurrentDurability >= 0.0f)
							{
								Result += FString::Printf(TEXT("  Durability: %.0f%%\n"), Item.CurrentDurability);
							}
						}
						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				]
			]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeItemRow(int32 ItemIndex)
{
	// Placeholder for production SListView item widget
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeDetailPanel()
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
				.Text(NSLOCTEXT("HUD", "ItemDetail", "ITEM DETAILS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "SelectItem", "Select an item to view details"))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeWeightBar()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!InventoryComp.IsValid()) return FText::GetEmpty();
				float Current = InventoryComp->GetCurrentWeight();
				float Max = InventoryComp->GetMaxWeight();
				return FText::Format(
					NSLOCTEXT("HUD", "Weight", "Weight: {0} / {1} kg"),
					FText::AsNumber(FMath::RoundToInt(Current)),
					FText::AsNumber(FMath::RoundToInt(Max)));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity_Lambda([this]()
			{
				if (InventoryComp.IsValid() && InventoryComp->IsOverweight())
				{
					return FSlateColor(FLinearColor(1.0f, 0.3f, 0.2f));
				}
				return FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));
			})
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.HeightOverride(8.0f)
			[
				SNew(SProgressBar)
				.Percent_Lambda([this]()
				{
					if (!InventoryComp.IsValid()) return 0.0f;
					return FMath::Clamp(InventoryComp->GetWeightPercent(), 0.0f, 1.0f);
				})
				.FillColorAndOpacity_Lambda([this]()
				{
					if (InventoryComp.IsValid() && InventoryComp->IsOverweight())
					{
						return FLinearColor(1.0f, 0.3f, 0.2f);
					}
					return FLinearColor(0.6f, 0.6f, 0.4f);
				})
				.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderPadding(FVector2D(0, 0))
			]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeResourceCounters()
{
	return SNew(SHorizontalBox)

		// Scrap
		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Scrap", "SCRAP"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.6f, 0.4f)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!InventoryComp.IsValid()) return FText::GetEmpty();
					return FText::AsNumber(InventoryComp->GetScrap());
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.5f)))
			]
		]

		// Influence
		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Influence", "INFLUENCE"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.5f, 0.7f)))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!InventoryComp.IsValid()) return FText::GetEmpty();
					return FText::AsNumber(FMath::RoundToInt(InventoryComp->GetInfluence()));
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.7f, 1.0f)))
			]
		];
}

TArray<FInventoryItem> SSEEInventoryScreen::GetFilteredItems() const
{
	if (!InventoryComp.IsValid())
	{
		return {};
	}

	const TArray<FInventoryItem>& AllItems = InventoryComp->GetAllItems();

	// "All" filter (255)
	if (static_cast<uint8>(ActiveCategory) == 255)
	{
		return AllItems;
	}

	TArray<FInventoryItem> Filtered;
	for (const FInventoryItem& Item : AllItems)
	{
		FItemDefinition Def;
		if (InventoryComp->GetItemDefinition(Item.ItemID, Def) && Def.Category == ActiveCategory)
		{
			Filtered.Add(Item);
		}
	}
	return Filtered;
}

void SSEEInventoryScreen::SelectItem(int32 ItemIndex)
{
	SelectedItemIndex = ItemIndex;
}

FText SSEEInventoryScreen::GetItemDisplayName(FName ItemID) const
{
	if (!InventoryComp.IsValid())
	{
		return FText::FromName(ItemID);
	}

	FItemDefinition Def;
	if (InventoryComp->GetItemDefinition(ItemID, Def))
	{
		return Def.DisplayName;
	}
	return FText::FromName(ItemID);
}

FText SSEEInventoryScreen::GetItemDescription(FName ItemID) const
{
	if (!InventoryComp.IsValid())
	{
		return FText::GetEmpty();
	}

	FItemDefinition Def;
	if (InventoryComp->GetItemDefinition(ItemID, Def))
	{
		return Def.Description;
	}
	return FText::GetEmpty();
}

FLinearColor SSEEInventoryScreen::GetRarityColor(FName ItemID) const
{
	if (!InventoryComp.IsValid())
	{
		return FLinearColor::White;
	}

	FItemDefinition Def;
	if (InventoryComp->GetItemDefinition(ItemID, Def))
	{
		switch (Def.Rarity)
		{
		case EItemRarity::Common:    return FLinearColor(0.7f, 0.7f, 0.7f);
		case EItemRarity::Uncommon:  return FLinearColor(0.3f, 0.8f, 0.3f);
		case EItemRarity::Rare:      return FLinearColor(0.3f, 0.5f, 1.0f);
		case EItemRarity::VeryRare:  return FLinearColor(0.7f, 0.3f, 0.9f);
		case EItemRarity::Legendary: return FLinearColor(1.0f, 0.7f, 0.1f);
		}
	}
	return FLinearColor::White;
}
