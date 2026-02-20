// SSEEFactionPanel.cpp - Faction reputation panel implementation
#include "SSEEFactionPanel.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEEFactionPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.88f))
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

			// Header
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				MakeHeader()
			]

			// Reputation list
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				MakeReputationList()
			]
		]
	];
}

void SSEEFactionPanel::UpdateReputations(const TArray<FFactionReputation>& InReputations)
{
	Reputations = InReputations;
}

TSharedRef<SWidget> SSEEFactionPanel::MakeHeader()
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("HUD", "Factions", "FACTION STANDINGS"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)));
}

TSharedRef<SWidget> SSEEFactionPanel::MakeReputationList()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					// Dynamic reputation entries rendered via lambda
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (Reputations.Num() == 0)
						{
							return NSLOCTEXT("HUD", "NoFactions", "No faction data available");
						}

						FString Result;
						for (const FFactionReputation& Faction : Reputations)
						{
							FText Standing = GetStandingText(Faction.Reputation);
							int32 RepInt = FMath::RoundToInt(Faction.Reputation);

							// Reputation bar: [-100 ====|====== 100]
							// Normalize to 0-20 scale (10 = neutral)
							int32 BarPos = FMath::Clamp(FMath::RoundToInt((Faction.Reputation + 100.0f) / 10.0f), 0, 20);
							FString Bar;
							for (int32 i = 0; i < 20; ++i)
							{
								if (i == 10) Bar += TEXT("|");
								else if (i == BarPos) Bar += TEXT("#");
								else if (i < BarPos) Bar += TEXT("=");
								else Bar += TEXT("-");
							}

							Result += FString::Printf(TEXT("%s\n  [%s] %+d (%s)\n\n"),
								*Faction.FactionName.ToString(),
								*Bar,
								RepInt,
								*Standing.ToString());
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				]
			]
		];
}

FText SSEEFactionPanel::GetStandingText(float Reputation) const
{
	if (Reputation <= -60.0f) return NSLOCTEXT("HUD", "Hostile", "Hostile");
	if (Reputation <= -20.0f) return NSLOCTEXT("HUD", "Unfriendly", "Unfriendly");
	if (Reputation < 20.0f)   return NSLOCTEXT("HUD", "Neutral", "Neutral");
	if (Reputation < 60.0f)   return NSLOCTEXT("HUD", "Friendly", "Friendly");
	return NSLOCTEXT("HUD", "Allied", "Allied");
}

FSlateColor SSEEFactionPanel::GetStandingColor(float Reputation) const
{
	if (Reputation <= -60.0f) return FSlateColor(FLinearColor(0.9f, 0.15f, 0.1f));   // Red
	if (Reputation <= -20.0f) return FSlateColor(FLinearColor(0.9f, 0.5f, 0.2f));    // Orange
	if (Reputation < 20.0f)   return FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));    // Grey
	if (Reputation < 60.0f)   return FSlateColor(FLinearColor(0.3f, 0.7f, 0.3f));    // Green
	return FSlateColor(FLinearColor(0.3f, 0.6f, 1.0f));                               // Blue (Allied)
}
