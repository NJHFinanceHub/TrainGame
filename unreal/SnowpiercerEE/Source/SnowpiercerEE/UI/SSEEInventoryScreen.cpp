// SSEEInventoryScreen.cpp - "POSSESSIONS" inventory screen implementation
#include "SSEEInventoryScreen.h"

#include "SnowpiercerEE/SEEInventoryComponent.h"
#include "SnowpiercerEE/SEECombatComponent.h"
#include "SnowpiercerEE/SEEWeaponBase.h"
#include "Widgets/SSEEUIStyle.h"
#include "Widgets/SSEEPanelFrame.h"
#include "Widgets/SSEEMenuButton.h"
#include "Widgets/SSEEArmorSlotWidget.h"
#include "Widgets/SSEEArmorDragDropOp.h"
#include "Widgets/SSEEArmorDragHandle.h"
#include "Widgets/SSEEItemRowCell.h"

#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Input/DragAndDrop.h"
#include "Framework/Application/SlateApplication.h"

void SSEEInventoryScreen::Construct(const FArguments& InArgs)
{
	InventoryComp  = InArgs._SEEInventoryComponent;
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
				.WidthOverride(1360.0f)  // widened from 1140 to accommodate armor panel
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

					// Main content: item list (left) + detail panel (center) + armor panel (right)
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SNew(SHorizontalBox)

						// --- Item list ---
						+ SHorizontalBox::Slot()
						.FillWidth(0.45f)
						.Padding(0, 0, 10, 0)
						[
							// Rivet-line framed panel for clear region separation.
							SNew(SBorder)
							.BorderImage(SEEUIStyle::WhiteBrush())
							.BorderBackgroundColor(SEEUIStyle::RivetLine)
							.Padding(FMargin(1.0f))
							[
								SNew(SBorder)
								.BorderImage(SEEUIStyle::WhiteBrush())
								.BorderBackgroundColor(SEEUIStyle::SteelBlack)
								.Padding(FMargin(6.0f, 6.0f, 6.0f, 6.0f))
								[
									SNew(SVerticalBox)

									// Column header strip
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 4)
									[
										MakeListColumnHeader()
									]

									// Thin divider rule under the header
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(0, 0, 0, 4)
									[
										SNew(SBox)
										.HeightOverride(1.0f)
										[
											SNew(SBorder)
											.BorderImage(SEEUIStyle::WhiteBrush())
											.BorderBackgroundColor(SEEUIStyle::RivetLine)
											.Padding(FMargin(0.0f))
											[ SNullWidget::NullWidget ]
										]
									]

									// Scrolling item rows
									+ SVerticalBox::Slot()
									.FillHeight(1.0f)
									[
										SAssignNew(ListScrollBox, SScrollBox)
										+ SScrollBox::Slot()
										[
											SAssignNew(ListBox, SVerticalBox)
										]
									]
								]
							]
						]

						// --- Detail panel ---
						+ SHorizontalBox::Slot()
						.FillWidth(0.30f)
						.Padding(0, 0, 8, 0)
						[
							MakeDetailPanel()
						]

						// --- Paper-doll armor panel ---
						+ SHorizontalBox::Slot()
						.FillWidth(0.25f)
						[
							MakeArmorPanel()
						]
					]

					// Divider rule above the footer
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 10, 0, 0)
					[
						SNew(SBox)
						.HeightOverride(1.0f)
						[
							SNew(SBorder)
							.BorderImage(SEEUIStyle::WhiteBrush())
							.BorderBackgroundColor(SEEUIStyle::RivetLine)
							.Padding(FMargin(0.0f))
							[ SNullWidget::NullWidget ]
						]
					]

					// Footer: weight capacity bar (left) + scrip/items + key hints (right)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 8, 0, 0)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(0.45f)
						.Padding(0, 0, 8, 0)
						[
							MakeWeightBar()
						]

						+ SHorizontalBox::Slot()
						.FillWidth(0.55f)
						.VAlign(VAlign_Bottom)
						[
							SNew(SVerticalBox)

							// Scrip value + carried item count
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(0, 0, 0, 3)
							[
								SNew(SHorizontalBox)

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 16, 0)
								[
									SNew(STextBlock)
									.Text_Lambda([this]()
									{
										return FText::Format(
											NSLOCTEXT("HUD", "InvScrip", "Scrip value: {0}"),
											FText::AsNumber(GetTotalCarriedValue()));
									})
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
									.ColorAndOpacity(FSlateColor(SEEUIStyle::EngineAmber))
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(STextBlock)
									.Text_Lambda([this]()
									{
										const int32 MaxSlots = InventoryComp.IsValid() ? InventoryComp->GetSlotCount() : 0;
										return FText::Format(
											NSLOCTEXT("HUD", "InvItemsCarried", "Items: {0} / {1}"),
											FText::AsNumber(OccupiedSlotCount), FText::AsNumber(MaxSlots));
									})
									.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
									.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
								]
							]

							+ SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(STextBlock)
								.Text(NSLOCTEXT("HUD", "InvHints", "Up/Down select   Enter use   E equip   Del drop   Esc close"))
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
								.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
							]
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
	// Slot cell widgets update themselves via lambdas – no explicit Invalidate needed.
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
	if (Key == EKeys::E)
	{
		// Keyboard equip fallback: equip selected armor item to the inferred slot.
		if (HasSelection())
		{
			const FSEEItemData* Data = GetSelectedData();
			if (Data && Data->Category == ESEEItemCategory::Armor)
			{
				const FName ItemID = Entries[SelectedIndex].ItemID;
				EquipArmorItem(ItemID, InferSlotFromItemID(ItemID));
			}
		}
		return FReply::Handled();
	}

	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

// ---------------------------------------------------------------------------
// Armor panel
// ---------------------------------------------------------------------------

TSharedRef<SWidget> SSEEInventoryScreen::MakeArmorPanel()
{
	return SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::RivetLine)
		.Padding(FMargin(1.0f))
		[
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::Gunmetal)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			// Panel title + amber underline
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ArmorPanelTitle", "PAPER DOLL"))
				.Font(SEEUIStyle::CaptionFont(11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextHeader))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(0, 4, 0, 8)
			[
				SNew(SBox)
				.HeightOverride(2.0f)
				.WidthOverride(46.0f)
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor(SEEUIStyle::EngineAmber)
					.Padding(FMargin(0.0f))
					[ SNullWidget::NullWidget ]
				]
			]

			// Aggregate stats: DR% + Cold resist
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					const UArmorComponent* AC = GetArmorComponent();
					if (!AC) return FText::FromString(TEXT("DR — %  Cold —"));
					return FText::FromString(FString::Printf(
						TEXT("DR %.0f%%   Cold %.0f"),
						AC->GetTotalDamageReduction(),
						AC->GetTotalColdResistance()));
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::EngineAmber))
			]

			// HEAD slot
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 6)
			[
				SAssignNew(HeadSlotWidget, SSEEArmorSlotWidget)
				.Slot(EArmorSlot::Head)
				.ArmorComponent(GetArmorComponent())
				.OnDrop(FOnArmorSlotDrop::CreateLambda([this](EArmorSlot DropSlot, FName DropItemID)
				{
					EquipArmorItem(DropItemID, DropSlot); // delegate is (Slot, ItemID); method is (ItemID, Slot)
				}))
				.OnUnequip(FOnArmorSlotUnequip::CreateSP(this, &SSEEInventoryScreen::UnequipArmorSlot))
			]

			// TORSO slot
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 6)
			[
				SAssignNew(TorsoSlotWidget, SSEEArmorSlotWidget)
				.Slot(EArmorSlot::Torso)
				.ArmorComponent(GetArmorComponent())
				.OnDrop(FOnArmorSlotDrop::CreateLambda([this](EArmorSlot DropSlot, FName DropItemID)
				{
					EquipArmorItem(DropItemID, DropSlot); // delegate is (Slot, ItemID); method is (ItemID, Slot)
				}))
				.OnUnequip(FOnArmorSlotUnequip::CreateSP(this, &SSEEInventoryScreen::UnequipArmorSlot))
			]

			// SHIELD slot
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 10)
			[
				SAssignNew(ShieldSlotWidget, SSEEArmorSlotWidget)
				.Slot(EArmorSlot::Shield)
				.ArmorComponent(GetArmorComponent())
				.OnDrop(FOnArmorSlotDrop::CreateLambda([this](EArmorSlot DropSlot, FName DropItemID)
				{
					EquipArmorItem(DropItemID, DropSlot); // delegate is (Slot, ItemID); method is (ItemID, Slot)
				}))
				.OnUnequip(FOnArmorSlotUnequip::CreateSP(this, &SSEEInventoryScreen::UnequipArmorSlot))
			]

			// Drag hint
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ArmorDragHint", "Drag armor here  or  press E"))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
				.AutoWrapText(true)
			]
		]
		];
}

// ---------------------------------------------------------------------------
// Equip / Unequip logic (shared path)
// ---------------------------------------------------------------------------

void SSEEInventoryScreen::EquipArmorItem(FName ItemID, EArmorSlot Slot)
{
	if (ItemID.IsNone() || !InventoryComp.IsValid()) return;

	UArmorComponent* AC = GetArmorComponent();
	if (!AC) return;

	const FSEEItemData* Data = GetData(ItemID);
	if (!Data || Data->Category != ESEEItemCategory::Armor) return;

	// Equip via ArmorComponent – captures the previous piece.
	FEquippedArmor PrevPiece;
	const bool bOk = AC->EquipFromItem(ItemID, Slot, *Data, PrevPiece);
	if (!bOk) return;

	// Remove one copy of the newly equipped item from inventory.
	InventoryComp->RemoveItem(ItemID, 1);

	// If a previous piece was displaced, return it to inventory.
	if (PrevPiece.IsValid())
	{
		InventoryComp->AddItem(PrevPiece.ArmorItemID, 1);
	}

	Refresh();
}

void SSEEInventoryScreen::UnequipArmorSlot(EArmorSlot Slot)
{
	if (!InventoryComp.IsValid()) return;

	UArmorComponent* AC = GetArmorComponent();
	if (!AC || !AC->HasArmorInSlot(Slot)) return;

	const FEquippedArmor Removed = AC->UnequipArmor(Slot);
	if (Removed.IsValid())
	{
		InventoryComp->AddItem(Removed.ArmorItemID, 1);
	}

	Refresh();
}

EArmorSlot SSEEInventoryScreen::InferSlotFromItemID(FName ItemID)
{
	const FString Name = ItemID.ToString().ToLower();

	// Shield keywords
	if (Name.Contains(TEXT("shield")) || Name.Contains(TEXT("guard")))
		return EArmorSlot::Shield;

	// Head keywords
	if (Name.Contains(TEXT("helm"))  || Name.Contains(TEXT("helmet")) ||
	    Name.Contains(TEXT("mask"))  || Name.Contains(TEXT("hood"))   ||
	    Name.Contains(TEXT("cap"))   || Name.Contains(TEXT("hat"))    ||
	    Name.Contains(TEXT("beret")) || Name.Contains(TEXT("visor")))
		return EArmorSlot::Head;

	// Default to Torso
	return EArmorSlot::Torso;
}

UArmorComponent* SSEEInventoryScreen::GetArmorComponent() const
{
	if (!InventoryComp.IsValid()) return nullptr;
	const AActor* Owner = InventoryComp->GetOwner();
	return Owner ? Owner->FindComponentByClass<UArmorComponent>() : nullptr;
}

// ---------------------------------------------------------------------------
// Header / tabs / list
// ---------------------------------------------------------------------------

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

TSharedRef<SWidget> SSEEInventoryScreen::MakeListColumnHeader()
{
	const FSlateFontInfo HeaderFont = SEEUIStyle::CaptionFont(8);
	const FSlateColor    HeaderColor(SEEUIStyle::TextFaint);

	return SNew(SHorizontalBox)

		// Aligns with the row's category badge column (3px accent + paddings + 22px badge).
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(11, 0, 8, 0)
		[
			SNew(SBox)
			.WidthOverride(22.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ColType", "TYPE"))
				.Font(HeaderFont)
				.ColorAndOpacity(HeaderColor)
			]
		]

		// Name column
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "ColItem", "ITEM"))
			.Font(HeaderFont)
			.ColorAndOpacity(HeaderColor)
		]

		// Equip column
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
				.Text(NSLOCTEXT("HUD", "ColEquip", "EQ"))
				.Font(HeaderFont)
				.ColorAndOpacity(HeaderColor)
			]
		]

		// Quantity column
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, 12, 0)
		[
			SNew(SBox)
			.WidthOverride(28.0f)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ColQty", "QTY"))
				.Font(HeaderFont)
				.ColorAndOpacity(HeaderColor)
			]
		]

		// Weight column
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
				.Text(NSLOCTEXT("HUD", "ColWt", "WT"))
				.Font(HeaderFont)
				.ColorAndOpacity(HeaderColor)
			]
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
			Entry.ItemID    = Slot.ItemID;
			Entry.Quantity  = Slot.Quantity;
			Entries.Add(Entry);
		}
	}

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
		const FEntry& Entry    = Entries[EntryIndex];
		const FSEEItemData* Data = GetData(Entry.ItemID);

		const FText Name         = (Data && !Data->DisplayName.IsEmpty())
			? Data->DisplayName
			: FText::FromName(Entry.ItemID);
		const FLinearColor NameColor = Data ? GetRarityColor(Data->Rarity) : SEEUIStyle::RarityCommon;
		const float UnitWeight   = Data ? Data->Weight : 0.0f;
		const ESEEItemCategory Category = Data ? Data->Category : ESEEItemCategory::Junk;

		// Equip column: weapons matching the pawn's equipped weapon get a marker.
		const bool bIsWeapon  = (Category == ESEEItemCategory::Weapon);
		const bool bIsArmor   = (Category == ESEEItemCategory::Armor);
		const bool bEquipped  = bIsWeapon && !EquippedItemID.IsNone() && (EquippedItemID == Entry.ItemID);

		// Check if this armor is currently equipped in any slot.
		const UArmorComponent* AC = GetArmorComponent();
		bool bArmorEquipped = false;
		if (bIsArmor && AC)
		{
			for (EArmorSlot S : { EArmorSlot::Head, EArmorSlot::Torso, EArmorSlot::Shield })
			{
				FEquippedArmor Piece;
				if (AC->GetArmorInSlot(S, Piece) && Piece.ArmorItemID == Entry.ItemID)
				{
					bArmorEquipped = true;
					break;
				}
			}
		}

		// Capture for drag detection
		const FName CapturedItemID    = Entry.ItemID;
		const FText CapturedName      = Name;
		const bool  bCapturedIsArmor  = bIsArmor;

		const FLinearColor RowRarityColor = NameColor;

		TSharedRef<SWidget> Row =
			SNew(SSEEItemRowCell)
			.RarityColor(RowRarityColor)
			.RowHeight(36.0f)
			.IsSelected_Lambda([this, EntryIndex]() { return EntryIndex == SelectedIndex; })
			[
				// Wrap inner button in a detector so armor rows can be dragged.
				// SButton itself does not expose OnDragDetected so we overlay an
				// invisible detector on top when the item is armor.
				SNew(SOverlay)

				+ SOverlay::Slot()
				[
					// Hollow button: the SSEEItemRowCell frame draws the background
					// and selection state; this layer only provides click + hover feel.
					SNew(SButton)
					.ButtonStyle(&SEEUIStyle::GetHollowButtonStyle())
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
						.Padding(8, 0, 8, 0)
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
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
							.ColorAndOpacity(FSlateColor(NameColor))
							.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
						]

						// Equip indicator column
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
								.Text((bEquipped || bArmorEquipped)
									? NSLOCTEXT("HUD", "InvEquipped", "EQUIPPED")
									: FText::FromString(FString(TEXT("—"))))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
								.ColorAndOpacity(FSlateColor((bEquipped || bArmorEquipped)
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
				]

				// Drag-detector overlay for armor rows only — a custom handle widget
				// (UE 5.7 SBorder/SButton don't expose declarative drag args).
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.Visibility(bCapturedIsArmor ? EVisibility::Visible : EVisibility::Collapsed)
					[
						SNew(SSEEArmorDragHandle)
						.ItemID(CapturedItemID)
						.DisplayName(CapturedName)
						.OnPressed(FOnArmorDragHandlePressed::CreateLambda([this, EntryIndex]()
						{
							SelectEntry(EntryIndex);
						}))
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

	InventoryComp->UseItem(Entries[SelectedIndex].SlotIndex);
	Refresh();
}

void SSEEInventoryScreen::DropSelected()
{
	if (!HasSelection() || !InventoryComp.IsValid()) return;

	InventoryComp->DropItem(Entries[SelectedIndex].SlotIndex);
	Refresh();
}

// ---------------------------------------------------------------------------
// Detail panel
// ---------------------------------------------------------------------------

TSharedRef<SWidget> SSEEInventoryScreen::MakeDetailPanel()
{
	return SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::RivetLine)
		.Padding(FMargin(1.0f))
		[
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::Gunmetal)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			// Panel title + amber underline
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "ItemDetail", "ITEM DETAILS"))
				.Font(SEEUIStyle::CaptionFont(11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextHeader))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(0, 4, 0, 2)
			[
				SNew(SBox)
				.HeightOverride(2.0f)
				.WidthOverride(46.0f)
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor(SEEUIStyle::EngineAmber)
					.Padding(FMargin(0.0f))
					[ SNullWidget::NullWidget ]
				]
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

			// Rarity pill + category chip
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(SHorizontalBox)
				.Visibility_Lambda([this]()
				{
					return GetSelectedData() ? EVisibility::Visible : EVisibility::Collapsed;
				})

				// Rarity pill (rarity-tinted background, rarity-colored text)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0, 0, 6, 0)
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor_Lambda([this]() -> FLinearColor
					{
						const FSEEItemData* Data = GetSelectedData();
						return SEEUIStyle::Dim(Data ? GetRarityColor(Data->Rarity) : SEEUIStyle::TextDim, 0.20f);
					})
					.Padding(FMargin(7, 2))
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							const FSEEItemData* Data = GetSelectedData();
							return Data ? GetRarityText(Data->Rarity) : FText::GetEmpty();
						})
						.Font(SEEUIStyle::CaptionFont(8))
						.ColorAndOpacity_Lambda([this]() -> FSlateColor
						{
							const FSEEItemData* Data = GetSelectedData();
							return FSlateColor(Data ? GetRarityColor(Data->Rarity) : SEEUIStyle::TextDim);
						})
					]
				]

				// Category chip (category accent color)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor_Lambda([this]() -> FLinearColor
					{
						const FSEEItemData* Data = GetSelectedData();
						return SEEUIStyle::Dim(Data ? GetCategoryBadgeColor(Data->Category) : SEEUIStyle::TextDim, 0.20f);
					})
					.Padding(FMargin(7, 2))
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							const FSEEItemData* Data = GetSelectedData();
							return Data ? GetCategoryText(Data->Category) : FText::GetEmpty();
						})
						.Font(SEEUIStyle::CaptionFont(8))
						.ColorAndOpacity_Lambda([this]() -> FSlateColor
						{
							const FSEEItemData* Data = GetSelectedData();
							return FSlateColor(Data ? GetCategoryBadgeColor(Data->Category) : SEEUIStyle::TextDim);
						})
					]
				]
			]

			// Description + stats
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

					// "PROPERTIES" sub-label + divider rule
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 12, 0, 3)
					[
						SNew(STextBlock)
						.Visibility_Lambda([this]()
						{
							return GetSelectedData() ? EVisibility::Visible : EVisibility::Collapsed;
						})
						.Text(NSLOCTEXT("HUD", "DetailProps", "PROPERTIES"))
						.Font(SEEUIStyle::OverlineFont(8))
						.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 0, 0, 6)
					[
						SNew(SBox)
						.HeightOverride(1.0f)
						.Visibility_Lambda([this]()
						{
							return GetSelectedData() ? EVisibility::Visible : EVisibility::Collapsed;
						})
						[
							SNew(SBorder)
							.BorderImage(SEEUIStyle::WhiteBrush())
							.BorderBackgroundColor(SEEUIStyle::RivetLine)
							.Padding(FMargin(0.0f))
							[ SNullWidget::NullWidget ]
						]
					]

					// Stats block
					+ SVerticalBox::Slot()
					.AutoHeight()
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
							if (Data->Category == ESEEItemCategory::Armor)
							{
								const EArmorSlot InferredSlot = InferSlotFromItemID(Entries[SelectedIndex].ItemID);
								const TCHAR* SlotName =
									InferredSlot == EArmorSlot::Head   ? TEXT("Head") :
									InferredSlot == EArmorSlot::Shield ? TEXT("Shield") : TEXT("Torso");
								Lines += FString::Printf(TEXT("\nSlot: %s (inferred)"), SlotName);
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
		]
		];
}

TSharedRef<SWidget> SSEEInventoryScreen::MakeActionButtons()
{
	TSharedRef<SVerticalBox> ButtonColumn = SNew(SVerticalBox)

		// Row 1: USE + DROP
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			.Padding(0, 0, 4, 0)
			[
				SAssignNew(UseButton, SSEEMenuButton)
				.Text(NSLOCTEXT("HUD", "InvUse", "USE"))
				.Width(180.0f)
				.Height(38.0f)
				.OnClicked(FSimpleDelegate::CreateSP(this, &SSEEInventoryScreen::UseSelected))
			]

			+ SHorizontalBox::Slot()
			.FillWidth(0.5f)
			[
				SAssignNew(DropButton, SSEEMenuButton)
				.Text(NSLOCTEXT("HUD", "InvDrop", "DROP"))
				.Width(180.0f)
				.Height(38.0f)
				.TickColor(SEEUIStyle::BloodRed)
				.bUseDefaultTickColor(false)
				.OnClicked(FSimpleDelegate::CreateSP(this, &SSEEInventoryScreen::DropSelected))
			]
		]

		// Row 2: EQUIP (armor only)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SAssignNew(EquipButton, SSEEMenuButton)
			.Text(NSLOCTEXT("HUD", "InvEquipBtn", "EQUIP  (E)"))
			.Width(364.0f)
			.Height(38.0f)
			.TickColor(SEEUIStyle::AccentSteel)
			.bUseDefaultTickColor(false)
			.OnClicked(FSimpleDelegate::CreateLambda([this]()
			{
				if (HasSelection())
				{
					const FSEEItemData* Data = GetSelectedData();
					if (Data && Data->Category == ESEEItemCategory::Armor)
					{
						const FName ItemID = Entries[SelectedIndex].ItemID;
						EquipArmorItem(ItemID, InferSlotFromItemID(ItemID));
					}
				}
			}))
		];

	// Set enabled states after widgets are created.
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
			return !Data || Data->Category != ESEEItemCategory::Quest;
		}));
	}
	if (EquipButton.IsValid())
	{
		EquipButton->SetEnabled(TAttribute<bool>::CreateLambda([this]()
		{
			const FSEEItemData* Data = GetSelectedData();
			return Data && Data->Category == ESEEItemCategory::Armor && GetArmorComponent() != nullptr;
		}));
	}

	return ButtonColumn;
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

// ---------------------------------------------------------------------------
// Data helpers
// ---------------------------------------------------------------------------

const FSEEItemData* SSEEInventoryScreen::GetData(FName ItemID) const
{
	return InventoryComp.IsValid() ? InventoryComp->GetItemDataPtr(ItemID) : nullptr;
}

int32 SSEEInventoryScreen::GetTotalCarriedValue() const
{
	if (!InventoryComp.IsValid()) return 0;

	int32 Total = 0;
	const TArray<FSEEInventorySlot> Slots = InventoryComp->GetAllSlots();
	for (const FSEEInventorySlot& Slot : Slots)
	{
		if (Slot.IsEmpty()) continue;
		if (const FSEEItemData* Data = InventoryComp->GetItemDataPtr(Slot.ItemID))
		{
			Total += Data->Value * Slot.Quantity;
		}
	}
	return Total;
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
