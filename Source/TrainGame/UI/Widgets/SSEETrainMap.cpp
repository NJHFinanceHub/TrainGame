// SSEETrainMap.cpp - Train map implementation
#include "SSEETrainMap.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

// Forward-declared struct reference for car data
#include "SnowpiercerEE/SEETypes.h"

void SSEETrainMap::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f))
		.Padding(30.0f)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				MakeMapHeader()
			]

			// Car strip (the actual train diagram)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				MakeCarStrip()
			]

			// Selected car info
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 16, 0, 0)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (CurrentCarIndex >= 0 && CurrentCarIndex < CarEntries.Num())
					{
						const FCarDisplayEntry& Entry = CarEntries[CurrentCarIndex];
						return FText::Format(
							NSLOCTEXT("HUD", "CurrentCar", "Current Location: {0}"),
							Entry.Name);
					}
					return NSLOCTEXT("HUD", "NoLocation", "Location Unknown");
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
			]

			// Legend
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 16, 0, 0)
			[
				MakeLegend()
			]
		]
	];
}

void SSEETrainMap::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	PulseTimer += InDeltaTime;
}

void SSEETrainMap::SetCarData(const TArray<FSEECarData>& Cars)
{
	CarEntries.Empty();
	CarEntries.Reserve(Cars.Num());

	for (const FSEECarData& Car : Cars)
	{
		FCarDisplayEntry Entry;
		Entry.Name = FText::FromName(Car.CarName);
		Entry.ZoneIndex = static_cast<int32>(Car.Zone);
		Entry.bVisited = false;
		Entry.bCompleted = false;
		CarEntries.Add(Entry);
	}
}

void SSEETrainMap::UpdateCarState(int32 CarIndex, bool bVisited, bool bCompleted)
{
	if (CarEntries.IsValidIndex(CarIndex))
	{
		CarEntries[CarIndex].bVisited = bVisited;
		CarEntries[CarIndex].bCompleted = bCompleted;
	}
}

void SSEETrainMap::SetCurrentCar(int32 CarIndex)
{
	CurrentCarIndex = CarIndex;
}

TSharedRef<SWidget> SSEETrainMap::MakeMapHeader()
{
	return SNew(SHorizontalBox)

		// Engine label (left side)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "EngineDir", "< ENGINE"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
		]

		// Title (center)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "TrainMap", "TRAIN DIAGRAM"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		// Tail label (right side)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "TailDir", "TAIL >"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)))
		];
}

TSharedRef<SWidget> SSEETrainMap::MakeCarStrip()
{
	return SNew(SScrollBox)
		.Orientation(Orient_Horizontal)
		+ SScrollBox::Slot()
		[
			SNew(SHorizontalBox)
			// Car rectangles are built dynamically via lambda in a text-based representation.
			// In production, each car would be an interactive SButton/SBorder.
			// For scaffold, we render a simple text-based train diagram.
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (CarEntries.Num() == 0)
					{
						return NSLOCTEXT("HUD", "NoCars", "No car data loaded");
					}

					// Build a text-based train diagram
					// Cars displayed right-to-left (Tail on right, Engine on left)
					FString TopLine;    // ┌──────┐
					FString MidLine;    // │ NAME │
					FString BotLine;    // └──────┘
					FString StatLine;   // Status indicators

					for (int32 i = CarEntries.Num() - 1; i >= 0; --i)
					{
						const FCarDisplayEntry& Entry = CarEntries[i];

						FString ShortName = Entry.Name.ToString().Left(6);
						while (ShortName.Len() < 6)
						{
							ShortName += TEXT(" ");
						}

						bool bIsCurrent = (i == CurrentCarIndex);

						if (bIsCurrent)
						{
							TopLine += TEXT("[======]");
							MidLine += FString::Printf(TEXT("[%s]"), *ShortName);
							BotLine += TEXT("[======]");
						}
						else if (Entry.bVisited)
						{
							TopLine += TEXT("+------+");
							MidLine += FString::Printf(TEXT("|%s|"), *ShortName);
							BotLine += TEXT("+------+");
						}
						else
						{
							TopLine += TEXT("+------+");
							MidLine += TEXT("|  ??  |");
							BotLine += TEXT("+------+");
						}

						// Connector between cars
						if (i > 0)
						{
							TopLine += TEXT("  ");
							MidLine += TEXT("--");
							BotLine += TEXT("  ");
						}
					}

					return FText::FromString(
						TopLine + TEXT("\n") + MidLine + TEXT("\n") + BotLine);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Mono", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
			]
		];
}

TSharedRef<SWidget> SSEETrainMap::MakeLegend()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 16, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "LegCurrent", "[====] Current"))
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.8f, 0.3f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 16, 0)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "LegVisited", "|Name | Visited"))
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "LegUnknown", "| ??  | Unknown"))
			.Font(FCoreStyle::GetDefaultFontStyle("Mono", 9))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.4f, 0.4f, 0.4f)))
		];
}

FLinearColor SSEETrainMap::GetZoneColor(int32 ZoneIndex) const
{
	// ESEETrainZone: Tail=0, ThirdClass=1, SecondClass=2, WorkingSpine=3, FirstClass=4, Sanctum=5, Engine=6
	switch (ZoneIndex)
	{
	case 0: return FLinearColor(0.25f, 0.25f, 0.3f);  // Tail - dark grey
	case 1: return FLinearColor(0.5f, 0.35f, 0.2f);   // Third Class - brown
	case 2: return FLinearColor(0.6f, 0.5f, 0.35f);   // Second Class - tan
	case 3: return FLinearColor(0.4f, 0.45f, 0.35f);  // Working Spine - industrial green
	case 4: return FLinearColor(0.7f, 0.6f, 0.4f);    // First Class - gold
	case 5: return FLinearColor(0.5f, 0.4f, 0.6f);    // Sanctum - purple
	case 6: return FLinearColor(0.8f, 0.3f, 0.15f);   // Engine - orange-red
	default: return FLinearColor(0.4f, 0.4f, 0.4f);
	}
}
