// SSEECharacterScreen.cpp - Character screen implementation
#include "SSEECharacterScreen.h"

#include "SnowpiercerEE/SEEStatsComponent.h"
#include "SnowyEngine/Inventory/InventoryComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEECharacterScreen::Construct(const FArguments& InArgs)
{
	StatsComp = InArgs._StatsComponent;
	InventoryComp = InArgs._InventoryComponent;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.88f))
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				MakeHeader()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				MakeLevelBar()
			]

			// Main content: stats (left) + equipment (right)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.55f)
				.Padding(0, 0, 8, 0)
				[
					MakeStatsPanel()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.45f)
				[
					MakeEquipmentPanel()
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEECharacterScreen::MakeHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Character", "CHARACTER"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!StatsComp.IsValid()) return FText::GetEmpty();
				int32 SP = StatsComp->GetSkillPoints();
				if (SP > 0)
				{
					return FText::Format(NSLOCTEXT("HUD", "SkillPts", "{0} skill points available"),
						FText::AsNumber(SP));
				}
				return FText::GetEmpty();
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.9f, 0.4f)))
		];
}

TSharedRef<SWidget> SSEECharacterScreen::MakeLevelBar()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 4)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!StatsComp.IsValid()) return FText::GetEmpty();
				return FText::Format(
					NSLOCTEXT("HUD", "LevelXP", "Level {0}   XP: {1} / {2}"),
					FText::AsNumber(StatsComp->GetLevel()),
					FText::AsNumber(StatsComp->GetCurrentXP()),
					FText::AsNumber(StatsComp->GetXPToNextLevel()));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
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
					if (!StatsComp.IsValid()) return 0.0f;
					int32 XPNeeded = StatsComp->GetXPToNextLevel();
					if (XPNeeded <= 0) return 1.0f;
					return FMath::Clamp(static_cast<float>(StatsComp->GetCurrentXP()) / XPNeeded, 0.0f, 1.0f);
				})
				.FillColorAndOpacity(FLinearColor(0.3f, 0.6f, 1.0f))
				.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderPadding(FVector2D(0, 0))
			]
		];
}

TSharedRef<SWidget> SSEECharacterScreen::MakeStatsPanel()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "CoreStats", "CORE STATS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (!StatsComp.IsValid())
						{
							return NSLOCTEXT("HUD", "NoStats", "No character data");
						}

						FString Result;
						int32 SP = StatsComp->GetSkillPoints();

						// Stats: 0=STR, 1=AGI, 2=END, 3=CUN, 4=PER, 5=CHA
						for (uint8 i = 0; i < 6; ++i)
						{
							ESEEStat Stat = static_cast<ESEEStat>(i);
							int32 Value = StatsComp->GetStat(Stat);
							FText Name = GetStatName(i);

							// Stat bar visualization
							FString Bar;
							for (int32 b = 0; b < 10; ++b)
							{
								Bar += (b < Value) ? TEXT("#") : TEXT("-");
							}

							Result += FString::Printf(TEXT("%-12s [%s] %d/10"),
								*Name.ToString(), *Bar, Value);

							if (SP > 0)
							{
								Result += TEXT("  [+]");
							}
							Result += TEXT("\n\n");
						}

						if (SP > 0)
						{
							Result += FString::Printf(TEXT("\nSkill Points: %d"), SP);
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				]
			]
		];
}

TSharedRef<SWidget> SSEECharacterScreen::MakeEquipmentPanel()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 12)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Equipment", "EQUIPMENT"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (!InventoryComp.IsValid())
						{
							return NSLOCTEXT("HUD", "NoEquip", "No equipment data");
						}

						FString Result;

						// Show equipped items by scanning inventory for weapon/armor categories
						const TArray<FInventoryItem>& Items = InventoryComp->GetAllItems();
						bool bFoundEquipped = false;

						for (const FInventoryItem& Item : Items)
						{
							FItemDefinition Def;
							if (InventoryComp->GetItemDefinition(Item.ItemID, Def))
							{
								if (Def.Category == EItemCategory::Weapon || Def.Category == EItemCategory::Armor)
								{
									FString Category = (Def.Category == EItemCategory::Weapon)
										? TEXT("Weapon") : TEXT("Armor");

									Result += FString::Printf(TEXT("[%s]\n  %s\n"),
										*Category, *Def.DisplayName.ToString());

									if (Item.CurrentDurability >= 0.0f)
									{
										Result += FString::Printf(TEXT("  Durability: %.0f%%\n"), Item.CurrentDurability);
									}
									Result += TEXT("\n");
									bFoundEquipped = true;
								}
							}
						}

						if (!bFoundEquipped)
						{
							Result = TEXT("No weapons or armor equipped");
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
					.AutoWrapText(true)
				]
			]
		];
}

FText SSEECharacterScreen::GetStatName(uint8 StatIndex) const
{
	switch (static_cast<ESEEStat>(StatIndex))
	{
	case ESEEStat::Strength:   return NSLOCTEXT("HUD", "STR", "Strength");
	case ESEEStat::Agility:    return NSLOCTEXT("HUD", "AGI", "Agility");
	case ESEEStat::Endurance:  return NSLOCTEXT("HUD", "END", "Endurance");
	case ESEEStat::Cunning:    return NSLOCTEXT("HUD", "CUN", "Cunning");
	case ESEEStat::Perception: return NSLOCTEXT("HUD", "PER", "Perception");
	case ESEEStat::Charisma:   return NSLOCTEXT("HUD", "CHA", "Charisma");
	}
	return FText::GetEmpty();
}

FLinearColor SSEECharacterScreen::GetStatBarColor(uint8 StatIndex) const
{
	switch (static_cast<ESEEStat>(StatIndex))
	{
	case ESEEStat::Strength:   return FLinearColor(0.9f, 0.3f, 0.2f);
	case ESEEStat::Agility:    return FLinearColor(0.3f, 0.9f, 0.4f);
	case ESEEStat::Endurance:  return FLinearColor(0.8f, 0.6f, 0.2f);
	case ESEEStat::Cunning:    return FLinearColor(0.6f, 0.3f, 0.9f);
	case ESEEStat::Perception: return FLinearColor(0.3f, 0.6f, 0.9f);
	case ESEEStat::Charisma:   return FLinearColor(0.9f, 0.7f, 0.3f);
	}
	return FLinearColor::White;
}
