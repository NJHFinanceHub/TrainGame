// SSEECodexPanel.cpp - Codex/journal implementation
#include "SSEECodexPanel.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEECodexPanel::Construct(const FArguments& InArgs)
{
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
			.Padding(0, 0, 0, 8)
			[
				MakeCategoryTabs()
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				.Padding(0, 0, 8, 0)
				[
					MakeEntryList()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				[
					MakeEntryDetail()
				]
			]
		]
	];
}

void SSEECodexPanel::AddEntry(const FCodexEntry& Entry)
{
	AllEntries.Add(Entry);
}

void SSEECodexPanel::MarkRead(FName EntryID)
{
	for (FCodexEntry& Entry : AllEntries)
	{
		if (Entry.EntryID == EntryID)
		{
			Entry.bRead = true;
			break;
		}
	}
}

TSharedRef<SWidget> SSEECodexPanel::MakeHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Codex", "CODEX"))
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
				int32 Total = AllEntries.Num();
				int32 Read = 0;
				for (const FCodexEntry& E : AllEntries)
				{
					if (E.bRead) Read++;
				}
				return FText::Format(
					NSLOCTEXT("HUD", "CodexProgress", "{0} / {1} entries"),
					FText::AsNumber(Read), FText::AsNumber(Total));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
}

TSharedRef<SWidget> SSEECodexPanel::MakeCategoryTabs()
{
	auto MakeTab = [this](const FText& Label, ECodexCategory Cat) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, Cat]()
			{
				return ActiveCategory == Cat
					? FLinearColor(0.3f, 0.25f, 0.15f)
					: FLinearColor(0.1f, 0.1f, 0.12f);
			})
			.OnClicked_Lambda([this, Cat]()
			{
				ActiveCategory = Cat;
				SelectedEntryID = NAME_None;
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
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatLore", "Lore"), ECodexCategory::Lore) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatChars", "Characters"), ECodexCategory::Characters) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatLocs", "Locations"), ECodexCategory::Locations) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CatCollect", "Collectibles"), ECodexCategory::Collectibles) ];
}

TSharedRef<SWidget> SSEECodexPanel::MakeEntryList()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					TArray<FCodexEntry> Entries = GetFilteredEntries();
					if (Entries.Num() == 0)
					{
						return NSLOCTEXT("HUD", "NoEntries", "No entries discovered");
					}

					FString Result;
					for (const FCodexEntry& Entry : Entries)
					{
						FString ReadIndicator = Entry.bRead ? TEXT("  ") : TEXT("* ");
						Result += ReadIndicator + Entry.Title.ToString() + TEXT("\n");
					}
					return FText::FromString(Result);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SSEECodexPanel::MakeEntryDetail()
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
				.Text_Lambda([this]()
				{
					if (SelectedEntryID.IsNone())
					{
						return NSLOCTEXT("HUD", "SelectEntry", "Select an entry to read");
					}

					for (const FCodexEntry& Entry : AllEntries)
					{
						if (Entry.EntryID == SelectedEntryID)
						{
							FString Detail;
							Detail += Entry.Title.ToString() + TEXT("\n\n");
							Detail += Entry.Content.ToString();
							return FText::FromString(Detail);
						}
					}
					return FText::GetEmpty();
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
				.AutoWrapText(true)
			]
		];
}

TArray<FCodexEntry> SSEECodexPanel::GetFilteredEntries() const
{
	TArray<FCodexEntry> Filtered;
	for (const FCodexEntry& Entry : AllEntries)
	{
		if (Entry.Category == ActiveCategory)
		{
			Filtered.Add(Entry);
		}
	}
	return Filtered;
}
