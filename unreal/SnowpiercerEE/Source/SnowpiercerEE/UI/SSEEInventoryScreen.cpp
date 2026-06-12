// SSEEInventoryScreen.cpp - "POSSESSIONS" inventory screen implementation
#include "SSEEInventoryScreen.h"

#include "SnowpiercerEE/SEEInventoryComponent.h"
#include "SnowpiercerEE/SEECombatComponent.h"
#include "SnowpiercerEE/SEEWeaponBase.h"
#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEPanelFrame.h"
#include "Widgets/SSEEMenuButton.h"

#include "GameFramework/Actor.h"
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
		// Full-screen darkened steel backdrop
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::ScreenBackdrop)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(24.0f)
		[
			SNew(SSEEPanelFrame)
			.Title(NSLOCTEXT("HUD", "Possessions", "POSSESSIONS"))
			.ContentPadding(FMargin(24.0f, 16.0f, 24.0f, 20.0f))
			[
				SNew(SBox)
				.WidthOverride(1140.0f)
				.HeightOverride(600.0f)
				[
					SNew(SVerticalBox)

					// Slot count strip
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
							.BorderImage(SEEUIStyle::WhiteBrush())
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
					.Padding(0, 10, 0, 0)
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
			]
		]
	];

	Refresh();
}

void SSEEInventoryScreen::Refresh()
{
	RefreshEquippedItemID();
	RebuildList();
}

void SSEEInventoryScreen::RefreshEquippedItemID()
{
	EquippedItemID = NAME_None;

	if (!InventoryComp.IsValid()) return;

	const AActor* Owner = InventoryComp->GetOwner();
	if (!Owner) return;

	// Read-only peek at the pawn's combat component for the equipped weapon.
	const USEECombatComponent* Combat = Owner->FindComponentByClass<USEECombatComponent>();
	if (!Combat) return;

	if (const ASEEWeaponBase* Weapon = Combat->GetEquippedWeapon())
	{
		EquippedItemID = Weapon->GetSourceItemID();
	}
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
			.Text(NSLOCTEXT("HUD", "InvSubtitle", "EVERYTHING YOU OWN FITS IN YOUR HANDS"))
			.Font(SEEUIStyle::OverlineFont(10))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
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
			.Font(SEEUIStyle::CaptionFont(10))
			.ColorAndOpacity_Lambda([this, CategoryValue]()
			{
				return FSlateColor(ActiveCategory == CategoryValue
					? SEEUIStyle::EngineAmber
					: SEEUIStyle::TextPrimary);
			})
			.Margin(FMargin(10, 5))
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
			.Text(NSLOCTEXT("HUD", "EmptyInv", "Nothing. Out here, that can be a death sentence."))
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
		const ESEEItemCategory Category = Data ? Data->Category : ESEEItemCategory::Junk;

		// Equip column: weapons matching the pawn's equipped weapon get a marker.
		const bool bIsWeapon = (Category == ESEEItemCategory::Weapon);
		const bool bEquipped = bIsWeapon && !EquippedItemID.IsNone() && (EquippedItemID == Entry.ItemID);

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

					// Category letter badge (W/A/C/M/Q/J)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(6, 0, 8, 0)
					[
						SNew(SBox)
						.WidthOverride(22.0f)
						.HeightOverride(22.0f)
						[
							SNew(SBorder)
							.BorderImage(SEEUIStyle::WhiteBrush())
							.BorderBackgroundColor(SEEUIStyle::Dim(GetCategoryBadgeColor(Category), 0.22f))
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Padding(FMargin(0))
							[
								SNew(STextBlock)
								.Text(GetCategoryBadgeLetter(Category))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								.ColorAndOpacity(FSlateColor(GetCategoryBadgeColor(Category)))
							]
						]
					]

					// Name
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Name)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(FSlateColor(NameColor))
					]

					// Equip indicator column (weapons only show the marker)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					.Padding(0, 0, 10, 0)
					[
						SNew(SBox)
						.WidthOverride(58.0f)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.Text(bEquipped
								? NSLOCTEXT("HUD", "InvEquipped", "EQUIPPED")
								: FText::FromString(FString(TEXT("—")))) // em dash
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
							.ColorAndOpacity(FSlateColor(bEquipped
								? SEEUIStyle::EngineAmber
								: SEEUIStyle::Dim(SEEUIStyle::TextFaint, 0.6f)))
						]
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
		.BorderImage(SEEUIStyle::WhiteBrush())
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
				.Font(SEEUIStyle::CaptionFont(11))
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
						.ColorAndOpacity(FSlateColor(SEEUIStyle::BoneText))
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
							if (Data->HungerRestore > 0.0f)
							{
								Lines += FString::Printf(TEXT("\nRestores %0.f hunger"), Data->HungerRestore);
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
				MakeActionButtons()
			]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeActionButtons()
{
	// Chrome buttons: USE (amber tick) / DROP (blood-red tick). Enabled state
	// tracks the current selection via attributes (set after Construct, which
	// would otherwise overwrite them with the bEnabled argument).
	TSharedRef<SHorizontalBox> Buttons = SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		.Padding(0, 0, 4, 0)
		[
			SAssignNew(UseButton, SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "InvUse", "USE"))
			.Width(180.0f)
			.Height(42.0f)
			.OnClicked(FSimpleDelegate::CreateSP(this, &SSEEInventoryScreen::UseSelected))
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			SAssignNew(DropButton, SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "InvDrop", "DROP"))
			.Width(180.0f)
			.Height(42.0f)
			.TickColor(SEEUIStyle::BloodRed)
			.bUseDefaultTickColor(false)
			.OnClicked(FSimpleDelegate::CreateSP(this, &SSEEInventoryScreen::DropSelected))
		];

	if (UseButton.IsValid())
	{
		UseButton->SetEnabled(TAttribute<bool>::CreateLambda([this]()
		{
			const FSEEItemData* Data = GetSelectedData();
			return Data && Data->Category == ESEEItemCategory::Consumable;
		}));
	}
	if (DropButton.IsValid())
	{
		DropButton->SetEnabled(TAttribute<bool>::CreateLambda([this]()
		{
			if (!HasSelection()) return false;
			const FSEEItemData* Data = GetSelectedData();
			// Unknown items can always be dropped; quest items never.
			return !Data || Data->Category != ESEEItemCategory::Quest;
		}));
	}

	return Buttons;
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
					return FSlateColor(SEEUIStyle::BloodRed);
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
					// Engine-amber shading toward blood-red as the pack fills.
					float Pct = 0.0f;
					if (InventoryComp.IsValid())
					{
						const float Max = InventoryComp->GetMaxWeight();
						Pct = Max > 0.0f ? FMath::Clamp(InventoryComp->GetCurrentWeight() / Max, 0.0f, 1.0f) : 0.0f;
					}
					const float RedBlend = FMath::Clamp((Pct - 0.5f) / 0.5f, 0.0f, 1.0f);
					return FMath::Lerp(SEEUIStyle::EngineAmber, SEEUIStyle::BloodRed, RedBlend);
				})
				.BackgroundImage(SEEUIStyle::WhiteBrush())
				.FillImage(SEEUIStyle::WhiteBrush())
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

FText SSEEInventoryScreen::GetCategoryBadgeLetter(ESEEItemCategory Category) const
{
	switch (Category)
	{
	case ESEEItemCategory::Weapon:     return NSLOCTEXT("HUD", "BadgeW", "W");
	case ESEEItemCategory::Armor:      return NSLOCTEXT("HUD", "BadgeA", "A");
	case ESEEItemCategory::Consumable: return NSLOCTEXT("HUD", "BadgeC", "C");
	case ESEEItemCategory::Crafting:   return NSLOCTEXT("HUD", "BadgeM", "M");
	case ESEEItemCategory::Quest:      return NSLOCTEXT("HUD", "BadgeQ", "Q");
	default:                           return NSLOCTEXT("HUD", "BadgeJ", "J");
	}
}

FLinearColor SSEEInventoryScreen::GetCategoryBadgeColor(ESEEItemCategory Category) const
{
	switch (Category)
	{
	case ESEEItemCategory::Weapon:     return SEEUIStyle::BloodRed;
	case ESEEItemCategory::Armor:      return SEEUIStyle::AccentSteel;
	case ESEEItemCategory::Consumable: return SEEUIStyle::OkGreen;
	case ESEEItemCategory::Crafting:   return SEEUIStyle::FrostBlue;
	case ESEEItemCategory::Quest:      return SEEUIStyle::EngineAmber;
	default:                           return SEEUIStyle::TextDim;
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
