// SSEEInventoryScreen.cpp - Inventory screen implementation
#include "SSEEInventoryScreen.h"

#include "SnowpiercerEE/SEEInventoryComponent.h"
#include "Widgets/SSEEUIStyle.h"

#include "InputCoreTypes.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEEInventoryScreen::Construct(const FArguments& InArgs)
{
	InventoryComp = InArgs._SEEInventoryComponent;
	OnRequestClose = InArgs._OnRequestClose;

	ChildSlot
	[
		// Full-screen semi-transparent backdrop
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(SEEUIStyle::ScreenBackdrop)
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

				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				.Padding(0, 0, 8, 0)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderBackgroundColor(SEEUIStyle::PanelDark)
					.Padding(4.0f)
					[
						SAssignNew(ListScrollBox, SScrollBox)
						+ SScrollBox::Slot()
						[
							SAssignNew(ListBox, SVerticalBox)
						]
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				[
					MakeDetailPanel()
				]
			]

			// Footer: weight capacity bar + key hints
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
				.VAlign(VAlign_Bottom)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "InvHints", "Up/Down select   Enter use   Del drop   Esc close"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
				]
			]
		]
	];

	Refresh();
}

void SSEEInventoryScreen::Refresh()
{
	RebuildList();
}

FReply SSEEInventoryScreen::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Escape)
	{
		OnRequestClose.ExecuteIfBound();
		return FReply::Handled();
	}
	if (Key == EKeys::Up)
	{
		MoveSelection(-1);
		return FReply::Handled();
	}
	if (Key == EKeys::Down)
	{
		MoveSelection(+1);
		return FReply::Handled();
	}
	if (Key == EKeys::Enter)
	{
		UseSelected();
		return FReply::Handled();
	}
	if (Key == EKeys::Delete)
	{
		DropSelected();
		return FReply::Handled();
	}

	// Anything else (I, C, M, ...) falls through to the player controller's
	// toggle bindings via the game-and-UI input mode.
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
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
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextHeader))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				const int32 MaxSlots = InventoryComp.IsValid() ? InventoryComp->GetSlotCount() : 0;
				return FText::Format(NSLOCTEXT("HUD", "InvSlotCount", "{0} / {1} slots"),
					FText::AsNumber(OccupiedSlotCount), FText::AsNumber(MaxSlots));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeCategoryTabs()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatAll", "All"), 255) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatWeapon", "Weapons"), static_cast<uint8>(ESEEItemCategory::Weapon)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatArmor", "Armor"), static_cast<uint8>(ESEEItemCategory::Armor)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatConsume", "Consumables"), static_cast<uint8>(ESEEItemCategory::Consumable)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatCraft", "Materials"), static_cast<uint8>(ESEEItemCategory::Crafting)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatQuest", "Quest"), static_cast<uint8>(ESEEItemCategory::Quest)) ]
		+ SHorizontalBox::Slot().AutoWidth()[ MakeTab(NSLOCTEXT("HUD", "CatJunk", "Junk"), static_cast<uint8>(ESEEItemCategory::Junk)) ];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeTab(const FText& Label, uint8 CategoryValue)
{
	return SNew(SButton)
		.ButtonStyle(&SEEUIStyle::GetRowButtonStyle())
		.ButtonColorAndOpacity_Lambda([this, CategoryValue]()
		{
			return ActiveCategory == CategoryValue ? SEEUIStyle::TabActive : SEEUIStyle::TabInactive;
		})
		.OnClicked_Lambda([this, CategoryValue]()
		{
			ActiveCategory = CategoryValue;
			SelectedIndex = INDEX_NONE;
			RebuildList();
			return FReply::Handled();
		})
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextPrimary))
			.Margin(FMargin(8, 4))
		];
}

void SSEEInventoryScreen::RebuildList()
{
	const FName PreviousSelection = HasSelection() ? Entries[SelectedIndex].ItemID : NAME_None;

	Entries.Reset();
	RowWidgets.Reset();
	OccupiedSlotCount = 0;

	if (!ListBox.IsValid()) return;
	ListBox->ClearChildren();

	if (InventoryComp.IsValid())
	{
		const TArray<FSEEInventorySlot> Slots = InventoryComp->GetAllSlots();
		for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
		{
			const FSEEInventorySlot& Slot = Slots[SlotIndex];
			if (Slot.IsEmpty()) continue;

			++OccupiedSlotCount;

			if (ActiveCategory != 255)
			{
				const FSEEItemData* Data = GetData(Slot.ItemID);
				const ESEEItemCategory Category = Data ? Data->Category : ESEEItemCategory::Junk;
				if (static_cast<uint8>(Category) != ActiveCategory) continue;
			}

			FEntry Entry;
			Entry.SlotIndex = SlotIndex;
			Entry.ItemID = Slot.ItemID;
			Entry.Quantity = Slot.Quantity;
			Entries.Add(Entry);
		}
	}

	// Keep the selection in range (the selected item may have been consumed/dropped).
	if (Entries.Num() == 0)
	{
		SelectedIndex = INDEX_NONE;
	}
	else if (!Entries.IsValidIndex(SelectedIndex))
	{
		SelectedIndex = FMath::Clamp(SelectedIndex, 0, Entries.Num() - 1);
	}

	if (Entries.Num() == 0)
	{
		ListBox->AddSlot()
		.AutoHeight()
		.Padding(8.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "EmptyInv", "No items"))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
		];
		return;
	}

	for (int32 EntryIndex = 0; EntryIndex < Entries.Num(); ++EntryIndex)
	{
		const FEntry& Entry = Entries[EntryIndex];
		const FSEEItemData* Data = GetData(Entry.ItemID);

		const FText Name = (Data && !Data->DisplayName.IsEmpty())
			? Data->DisplayName
			: FText::FromName(Entry.ItemID);
		const FLinearColor NameColor = Data ? GetRarityColor(Data->Rarity) : SEEUIStyle::RarityCommon;
		const float UnitWeight = Data ? Data->Weight : 0.0f;

		TSharedRef<SWidget> Row =
			SNew(SBox)
			.HeightOverride(32.0f)
			.Padding(FMargin(0, 1))
			[
				SNew(SButton)
				.ButtonStyle(&SEEUIStyle::GetRowButtonStyle())
				.ButtonColorAndOpacity_Lambda([this, EntryIndex]()
				{
					return EntryIndex == SelectedIndex ? SEEUIStyle::RowSelected : SEEUIStyle::RowNormal;
				})
				.OnClicked_Lambda([this, EntryIndex]()
				{
					SelectEntry(EntryIndex);
					return FReply::Handled();
				})
				[
					SNew(SHorizontalBox)

					// Name
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					.Padding(8, 0, 0, 0)
					[
						SNew(STextBlock)
						.Text(Name)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FSlateColor(NameColor))
					]

					// Count
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 12, 0)
					[
						SNew(STextBlock)
						.Text(Entry.Quantity > 1
							? FText::Format(NSLOCTEXT("HUD", "InvStack", "x{0}"), FText::AsNumber(Entry.Quantity))
							: FText::GetEmpty())
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
					]

					// Stack weight
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 8, 0)
					[
						SNew(SBox)
						.WidthOverride(64.0f)
						.HAlign(HAlign_Right)
						[
							SNew(STextBlock)
							.Text(FText::FromString(FString::Printf(TEXT("%.1f kg"), UnitWeight * Entry.Quantity)))
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
							.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
						]
					]
				]
			];

		RowWidgets.Add(Row);
		ListBox->AddSlot().AutoHeight()[ Row ];
	}

	// If the previously selected item still exists, keep it selected.
	if (!PreviousSelection.IsNone())
	{
		for (int32 i = 0; i < Entries.Num(); ++i)
		{
			if (Entries[i].ItemID == PreviousSelection)
			{
				SelectedIndex = i;
				break;
			}
		}
	}
}

void SSEEInventoryScreen::MoveSelection(int32 Delta)
{
	if (Entries.Num() == 0) return;

	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = (Delta > 0) ? 0 : Entries.Num() - 1;
	}
	else
	{
		SelectedIndex = FMath::Clamp(SelectedIndex + Delta, 0, Entries.Num() - 1);
	}

	if (ListScrollBox.IsValid() && RowWidgets.IsValidIndex(SelectedIndex))
	{
		ListScrollBox->ScrollDescendantIntoView(RowWidgets[SelectedIndex], false);
	}
}

void SSEEInventoryScreen::SelectEntry(int32 EntryIndex)
{
	if (Entries.IsValidIndex(EntryIndex))
	{
		SelectedIndex = EntryIndex;
	}
}

void SSEEInventoryScreen::UseSelected()
{
	if (!HasSelection() || !InventoryComp.IsValid()) return;

	// UseItem applies HealthRestore through the owner's health component and
	// decrements the stack; OnInventoryChanged then triggers a Refresh via the
	// UI subsystem. Refresh here too in case no delegate is bound.
	InventoryComp->UseItem(Entries[SelectedIndex].SlotIndex);
	Refresh();
}

void SSEEInventoryScreen::DropSelected()
{
	if (!HasSelection() || !InventoryComp.IsValid()) return;

	InventoryComp->DropItem(Entries[SelectedIndex].SlotIndex); // quest items refuse internally
	Refresh();
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeDetailPanel()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(SEEUIStyle::PanelDark)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			// Panel title
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ItemDetail", "ITEM DETAILS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextHeader))
			]

			// Item name
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 10, 0, 2)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!HasSelection())
					{
						return NSLOCTEXT("HUD", "SelectItem", "Select an item to view details");
					}
					const FSEEItemData* Data = GetSelectedData();
					return (Data && !Data->DisplayName.IsEmpty())
						? Data->DisplayName
						: FText::FromName(Entries[SelectedIndex].ItemID);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity_Lambda([this]()
				{
					const FSEEItemData* Data = GetSelectedData();
					return FSlateColor(Data ? GetRarityColor(Data->Rarity) : SEEUIStyle::TextDim);
				})
				.AutoWrapText(true)
			]

			// Rarity / category line
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const FSEEItemData* Data = GetSelectedData();
					if (!Data) return FText::GetEmpty();
					return FText::Format(NSLOCTEXT("HUD", "InvRarityCat", "{0}  -  {1}"),
						GetRarityText(Data->Rarity), GetCategoryText(Data->Category));
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
			]

			// Description
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
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
							const FSEEItemData* Data = GetSelectedData();
							return Data ? Data->Description : FText::GetEmpty();
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(SEEUIStyle::TextPrimary))
						.AutoWrapText(true)
					]

					// Stats block
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 10, 0, 0)
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							const FSEEItemData* Data = GetSelectedData();
							if (!Data || !HasSelection()) return FText::GetEmpty();

							FString Lines;
							Lines += FString::Printf(TEXT("Weight: %.1f kg"), Data->Weight);
							Lines += FString::Printf(TEXT("\nValue: %d"), Data->Value);
							if (Entries[SelectedIndex].Quantity > 1)
							{
								Lines += FString::Printf(TEXT("\nQuantity: %d"), Entries[SelectedIndex].Quantity);
							}
							if (Data->HealthRestore > 0.0f)
							{
								Lines += FString::Printf(TEXT("\nRestores %0.f health"), Data->HealthRestore);
							}
							if (Data->StaminaRestore > 0.0f)
							{
								Lines += FString::Printf(TEXT("\nRestores %0.f stamina"), Data->StaminaRestore);
							}
							if (Data->BluntArmor > 0.0f || Data->BladedArmor > 0.0f || Data->PiercingArmor > 0.0f)
							{
								Lines += FString::Printf(TEXT("\nArmor B/S/P: %.0f / %.0f / %.0f"),
									Data->BluntArmor, Data->BladedArmor, Data->PiercingArmor);
							}
							if (Data->ColdResistance > 0.0f)
							{
								Lines += FString::Printf(TEXT("\nCold resistance: %.0f"), Data->ColdResistance);
							}
							return FText::FromString(Lines);
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(SEEUIStyle::AccentSteel))
					]
				]
			]

			// Action buttons
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 10, 0, 0)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(0, 0, 4, 0)
				[
					MakeActionButton(NSLOCTEXT("HUD", "InvUse", "USE"), false)
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					MakeActionButton(NSLOCTEXT("HUD", "InvDrop", "DROP"), true)
				]
			]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeActionButton(const FText& Label, bool bDanger)
{
	return SNew(SBox)
		.HeightOverride(38.0f)
		[
			SNew(SButton)
			.ButtonStyle(&SEEUIStyle::GetMenuButtonStyle())
			.IsEnabled_Lambda([this, bDanger]()
			{
				const FSEEItemData* Data = GetSelectedData();
				if (!Data) return false;
				if (bDanger) // DROP: anything except quest items
				{
					return Data->Category != ESEEItemCategory::Quest;
				}
				// USE: consumables only
				return Data->Category == ESEEItemCategory::Consumable;
			})
			.OnClicked_Lambda([this, bDanger]()
			{
				if (bDanger) { DropSelected(); } else { UseSelected(); }
				return FReply::Handled();
			})
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(bDanger ? SEEUIStyle::DangerRed : SEEUIStyle::OkGreen))
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
				return FText::Format(
					NSLOCTEXT("HUD", "Weight", "Weight: {0} / {1} kg"),
					FText::AsNumber(FMath::RoundToInt(InventoryComp->GetCurrentWeight())),
					FText::AsNumber(FMath::RoundToInt(InventoryComp->GetMaxWeight())));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity_Lambda([this]()
			{
				if (InventoryComp.IsValid() && InventoryComp->IsOverweight())
				{
					return FSlateColor(SEEUIStyle::DangerRed);
				}
				return FSlateColor(SEEUIStyle::TextDim);
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
					const float Max = InventoryComp->GetMaxWeight();
					return Max > 0.0f ? FMath::Clamp(InventoryComp->GetCurrentWeight() / Max, 0.0f, 1.0f) : 0.0f;
				})
				.FillColorAndOpacity_Lambda([this]()
				{
					if (InventoryComp.IsValid() && InventoryComp->IsOverweight())
					{
						return SEEUIStyle::DangerRed;
					}
					return SEEUIStyle::AccentBrass;
				})
				.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderPadding(FVector2D(0, 0))
			]
		];
}

const FSEEItemData* SSEEInventoryScreen::GetData(FName ItemID) const
{
	return InventoryComp.IsValid() ? InventoryComp->GetItemDataPtr(ItemID) : nullptr;
}

const FSEEItemData* SSEEInventoryScreen::GetSelectedData() const
{
	return HasSelection() ? GetData(Entries[SelectedIndex].ItemID) : nullptr;
}

FText SSEEInventoryScreen::GetCategoryText(ESEEItemCategory Category) const
{
	switch (Category)
	{
	case ESEEItemCategory::Weapon:     return NSLOCTEXT("HUD", "CatWeaponLbl", "Weapon");
	case ESEEItemCategory::Armor:      return NSLOCTEXT("HUD", "CatArmorLbl", "Armor");
	case ESEEItemCategory::Consumable: return NSLOCTEXT("HUD", "CatConsumeLbl", "Consumable");
	case ESEEItemCategory::Crafting:   return NSLOCTEXT("HUD", "CatCraftLbl", "Crafting Material");
	case ESEEItemCategory::Quest:      return NSLOCTEXT("HUD", "CatQuestLbl", "Quest Item");
	default:                           return NSLOCTEXT("HUD", "CatJunkLbl", "Junk");
	}
}

FText SSEEInventoryScreen::GetRarityText(ESEEItemRarity Rarity) const
{
	switch (Rarity)
	{
	case ESEEItemRarity::Uncommon:  return NSLOCTEXT("HUD", "RarityUncommon", "Uncommon");
	case ESEEItemRarity::Rare:      return NSLOCTEXT("HUD", "RarityRare", "Rare");
	case ESEEItemRarity::Legendary: return NSLOCTEXT("HUD", "RarityLegendary", "Legendary");
	default:                        return NSLOCTEXT("HUD", "RarityCommon", "Common");
	}
}

FLinearColor SSEEInventoryScreen::GetRarityColor(ESEEItemRarity Rarity) const
{
	switch (Rarity)
	{
	case ESEEItemRarity::Uncommon:  return SEEUIStyle::RarityUncommon;
	case ESEEItemRarity::Rare:      return SEEUIStyle::RarityRare;
	case ESEEItemRarity::Legendary: return SEEUIStyle::RarityLegendary;
	default:                        return SEEUIStyle::RarityCommon;
	}
}
