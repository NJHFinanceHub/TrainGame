// SSEEFactionPanel.cpp - Fallout-style faction standing screen
#include "SSEEFactionPanel.h"
#include "SEEFactionManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace
{
	const FLinearColor PanelBack(0.043f, 0.051f, 0.063f, 0.96f);
	const FLinearColor CardBack(0.086f, 0.102f, 0.122f, 1.0f);
	const FLinearColor CardSelected(0.12f, 0.145f, 0.17f, 1.0f);
	const FLinearColor RuleColor(0.165f, 0.192f, 0.227f, 1.0f);
	const FLinearColor BoneText(0.847f, 0.831f, 0.784f, 1.0f);
	const FLinearColor DimText(0.55f, 0.55f, 0.52f, 1.0f);
	const FLinearColor Amber(0.91f, 0.64f, 0.24f, 1.0f);

	constexpr float StandingBarWidth = 340.0f;

	FSlateFontInfo TitleFont()   { return FCoreStyle::GetDefaultFontStyle("Bold", 24); }
	FSlateFontInfo HeadFont()    { return FCoreStyle::GetDefaultFontStyle("Bold", 15); }
	FSlateFontInfo BodyFont()    { return FCoreStyle::GetDefaultFontStyle("Regular", 11); }
	FSlateFontInfo CaptionFont() { return FCoreStyle::GetDefaultFontStyle("Regular", 9); }
}

/** Static per-faction display data: identity, lore, and what moves the needle. */
struct SSEEFactionPanel::FFactionDisplayInfo
{
	ESEEFaction Faction;
	const TCHAR* Name;
	FLinearColor Color;
	const TCHAR* Lore;
	const TCHAR* Raises;
	const TCHAR* Lowers;
};

TArrayView<const SSEEFactionPanel::FFactionDisplayInfo> SSEEFactionPanel::GetFactionInfo()
{
	static const FFactionDisplayInfo Infos[] =
	{
		{ ESEEFaction::Tailies, TEXT("THE TAILIES"),
		  FLinearColor(0.62f, 0.55f, 0.45f, 1.0f),
		  TEXT("The unticketed. Eighteen years in the dark on protein blocks and stubbornness."),
		  TEXT("Helping Tail folk, advancing the cause, mercy for the wounded"),
		  TEXT("Killing civilians, siding with the front, hoarding") },
		{ ESEEFaction::ThirdClassUnion, TEXT("THIRD CLASS UNION"),
		  FLinearColor(0.45f, 0.55f, 0.65f, 1.0f),
		  TEXT("Labor keeps the train alive, and labor remembers who stood with it."),
		  TEXT("Worker solidarity, fair trades, fixing what is broken"),
		  TEXT("Strikebreaking, theft from the shops") },
		{ ESEEFaction::Jackboots, TEXT("THE JACKBOOTS"),
		  FLinearColor(0.70f, 0.20f, 0.15f, 1.0f),
		  TEXT("Wilford's order, enforced in black. The boot does not ask why it stamps."),
		  TEXT("Compliance, informing, walking away"),
		  TEXT("Dead patrolmen, breached gates, talk of revolt") },
		{ ESEEFaction::Bureaucracy, TEXT("THE BUREAUCRACY"),
		  FLinearColor(0.55f, 0.50f, 0.60f, 1.0f),
		  TEXT("Second class runs the ledgers. Every ration, every birth, every disappearance — filed."),
		  TEXT("Paperwork honored, quotas met"),
		  TEXT("Forged manifests, dead clerks") },
		{ ESEEFaction::FirstClassElite, TEXT("FIRST CLASS"),
		  FLinearColor(0.85f, 0.75f, 0.45f, 1.0f),
		  TEXT("They bought their tickets before the freeze. They believe that still matters."),
		  TEXT("Service, spectacle, knowing your place"),
		  TEXT("Existing loudly; the Tail moving forward") },
		{ ESEEFaction::OrderOfTheEngine, TEXT("ORDER OF THE ENGINE"),
		  FLinearColor(0.40f, 0.65f, 0.70f, 1.0f),
		  TEXT("The Engine is eternal. The Engine provides. Blessed be its turning."),
		  TEXT("Reverence, pilgrimage, sacrifice"),
		  TEXT("Blasphemy against the sacred machine") },
		{ ESEEFaction::KronoleNetwork, TEXT("KRONOLE NETWORK"),
		  FLinearColor(0.55f, 0.35f, 0.65f, 1.0f),
		  TEXT("Industrial waste that numbs the cold. Somebody has to move it. Everybody pays."),
		  TEXT("Trade, discretion, debts paid"),
		  TEXT("Busted couriers, burned product") },
		{ ESEEFaction::TheThaw, TEXT("THE THAW"),
		  FLinearColor(0.50f, 0.75f, 0.55f, 1.0f),
		  TEXT("Heretics who whisper the ice is melting. The train does not want to hear it."),
		  TEXT("Evidence of the warming world, kept secrets"),
		  TEXT("Betraying believers to the front") },
	};
	return TArrayView<const FFactionDisplayInfo>(Infos, UE_ARRAY_COUNT(Infos));
}

void SSEEFactionPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(PanelBack)
		.Padding(28.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 14)
			[
				MakeHeader()
			]

			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().FillWidth(0.62f).Padding(0, 0, 14, 0)
				[
					MakeFactionList()
				]

				+ SHorizontalBox::Slot().FillWidth(0.38f)
				[
					MakeDetailPane()
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEEFactionPanel::MakeHeader()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("SEE", "FactionTitle", "FACTIONS OF THE TRAIN"))
			.Font(TitleFont())
			.ColorAndOpacity(BoneText)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
		[
			SNew(SBox).HeightOverride(2.0f)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Amber)
			]
		];
}

TSharedRef<SWidget> SSEEFactionPanel::MakeFactionList()
{
	TSharedRef<SScrollBox> List = SNew(SScrollBox);
	const int32 Count = GetFactionInfo().Num();
	for (int32 i = 0; i < Count; ++i)
	{
		List->AddSlot().Padding(0, 0, 0, 8)
		[
			MakeFactionRow(i)
		];
	}
	return List;
}

TSharedRef<SWidget> SSEEFactionPanel::MakeFactionRow(int32 InfoIndex)
{
	const FFactionDisplayInfo& Info = GetFactionInfo()[InfoIndex];

	// Live standing marker position across the -100..+100 axis
	auto MarkerPadding = [this, InfoIndex]() -> FMargin
	{
		const float Alpha = (FMath::Clamp(GetRep(InfoIndex), -100, 100) + 100) / 200.0f;
		return FMargin(Alpha * (StandingBarWidth - 6.0f), 0, 0, 0);
	};

	// Tier segments across the axis, widths proportional to tier span
	TSharedRef<SHorizontalBox> Segments = SNew(SHorizontalBox);
	static const ESEEFactionStanding Tiers[] =
	{
		ESEEFactionStanding::Hated, ESEEFactionStanding::Hostile,
		ESEEFactionStanding::Unfriendly, ESEEFactionStanding::Neutral,
		ESEEFactionStanding::Accepted, ESEEFactionStanding::Friendly,
		ESEEFactionStanding::Revered
	};
	for (ESEEFactionStanding Tier : Tiers)
	{
		int32 MinRep = 0, MaxRep = 0;
		USEEFactionManager::GetStandingRange(Tier, MinRep, MaxRep);
		const float Span = float(MaxRep - MinRep + 1) / 201.0f;
		FLinearColor SegColor = USEEFactionManager::GetStandingColor(Tier);
		SegColor.A = 0.35f;
		Segments->AddSlot().FillWidth(Span).Padding(0.5f, 0)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(SegColor)
		];
	}

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor_Lambda([this, InfoIndex]()
		{
			return InfoIndex == SelectedIndex ? CardSelected : CardBack;
		})
		.Padding(FMargin(14, 10))
		.OnMouseButtonDown_Lambda([this, InfoIndex](const FGeometry&, const FPointerEvent&)
		{
			SelectIndex(InfoIndex);
			return FReply::Handled();
		})
		[
			SNew(SVerticalBox)

			// Name + standing tier + raw value
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Info.Name))
					.Font(HeadFont())
					.ColorAndOpacity(Info.Color)
				]
				+ SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)
				[
					SNew(STextBlock)
					.Text_Lambda([this, InfoIndex]()
					{
						const ESEEFactionStanding S = GetStandingAt(InfoIndex);
						return FText::Format(
							NSLOCTEXT("SEE", "StandingFmt", "{0}  {1}"),
							USEEFactionManager::GetStandingDisplayName(S),
							FText::AsNumber(GetRep(InfoIndex)));
					})
					.Font(HeadFont())
					.ColorAndOpacity_Lambda([this, InfoIndex]()
					{
						return USEEFactionManager::GetStandingColor(GetStandingAt(InfoIndex));
					})
				]
			]

			// Lore line
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 6)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Info.Lore))
				.Font(BodyFont())
				.ColorAndOpacity(DimText)
				.AutoWrapText(true)
			]

			// Standing bar: tier segments + live marker
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBox).WidthOverride(StandingBarWidth).HeightOverride(10.0f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()[ Segments ]
					+ SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Fill)
					[
						SNew(SBorder)
						.Padding(TAttribute<FMargin>::CreateLambda(MarkerPadding))
						.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
						[
							SNew(SBox).WidthOverride(6.0f)
							[
								SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(BoneText)
							]
						]
					]
				]
			]

			// Active consequences
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
			[
				SNew(STextBlock)
				.Text_Lambda([this, InfoIndex]() { return GetEffectsText(InfoIndex); })
				.Font(CaptionFont())
				.ColorAndOpacity(Amber)
			]
		];
}

TSharedRef<SWidget> SSEEFactionPanel::MakeDetailPane()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(CardBack)
		.Padding(16.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(GetFactionInfo()[SelectedIndex].Name);
				})
				.Font(TitleFont())
				.ColorAndOpacity_Lambda([this]()
				{
					return GetFactionInfo()[SelectedIndex].Color;
				})
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 10)
			[
				SNew(SSeparator).ColorAndOpacity(RuleColor)
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(GetFactionInfo()[SelectedIndex].Lore);
				})
				.Font(BodyFont())
				.ColorAndOpacity(BoneText)
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 14, 0, 2)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("SEE", "Raises", "GAINS STANDING"))
				.Font(CaptionFont())
				.ColorAndOpacity(FLinearColor(0.45f, 0.75f, 0.50f, 1.0f))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(GetFactionInfo()[SelectedIndex].Raises);
				})
				.Font(BodyFont())
				.ColorAndOpacity(BoneText)
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 2)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("SEE", "Lowers", "LOSES STANDING"))
				.Font(CaptionFont())
				.ColorAndOpacity(FLinearColor(0.80f, 0.35f, 0.30f, 1.0f))
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(GetFactionInfo()[SelectedIndex].Lowers);
				})
				.Font(BodyFont())
				.ColorAndOpacity(BoneText)
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot().FillHeight(1.0f)
			[
				SNew(SSpacer)
			]

			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("SEE", "FactionHint", "W/S BROWSE   ·   ESC CLOSE"))
				.Font(CaptionFont())
				.ColorAndOpacity(DimText)
			]
		];
}

void SSEEFactionPanel::UpdateReputations(const TArray<FFactionReputation>& InReputations)
{
	Reputations = InReputations;
}

USEEFactionManager* SSEEFactionPanel::GetFactionManager() const
{
	if (CachedManager.IsValid())
	{
		return CachedManager.Get();
	}
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType != EWorldType::Game && Context.WorldType != EWorldType::PIE)
			{
				continue;
			}
			if (UWorld* World = Context.World())
			{
				if (UGameInstance* GI = World->GetGameInstance())
				{
					if (USEEFactionManager* Manager = GI->GetSubsystem<USEEFactionManager>())
					{
						CachedManager = Manager;
						return Manager;
					}
				}
			}
		}
	}
	return nullptr;
}

int32 SSEEFactionPanel::GetRep(int32 InfoIndex) const
{
	const TArrayView<const FFactionDisplayInfo> Infos = GetFactionInfo();
	if (!Infos.IsValidIndex(InfoIndex))
	{
		return 0;
	}
	if (USEEFactionManager* Manager = GetFactionManager())
	{
		return Manager->GetReputation(Infos[InfoIndex].Faction);
	}
	// Legacy pushed-values fallback (index-aligned where provided)
	return Reputations.IsValidIndex(InfoIndex)
		? FMath::RoundToInt(Reputations[InfoIndex].Reputation) : 0;
}

ESEEFactionStanding SSEEFactionPanel::GetStandingAt(int32 InfoIndex) const
{
	return USEEFactionManager::StandingForReputation(GetRep(InfoIndex));
}

FText SSEEFactionPanel::GetEffectsText(int32 InfoIndex) const
{
	const TArrayView<const FFactionDisplayInfo> Infos = GetFactionInfo();
	if (!Infos.IsValidIndex(InfoIndex))
	{
		return FText::GetEmpty();
	}
	USEEFactionManager* Manager = GetFactionManager();
	if (!Manager)
	{
		return FText::GetEmpty();
	}

	const ESEEFaction Faction = Infos[InfoIndex].Faction;
	TArray<FString> Effects;

	const float Price = Manager->GetMerchantPriceMultiplier(Faction);
	if (!FMath::IsNearlyEqual(Price, 1.0f))
	{
		Effects.Add(FString::Printf(TEXT("Prices %+d%%"),
			FMath::RoundToInt((Price - 1.0f) * 100.0f)));
	}
	if (Manager->ShouldNPCRefuseDialogue(Faction))
	{
		Effects.Add(TEXT("They refuse to speak with you"));
	}
	if (Faction == ESEEFaction::Jackboots && Manager->AreJackbootsKillOnSight())
	{
		Effects.Add(TEXT("KILL ON SIGHT"));
	}

	return Effects.Num() > 0
		? FText::FromString(FString::Join(Effects, TEXT("  ·  ")))
		: NSLOCTEXT("SEE", "NoEffects", "No active effects");
}

void SSEEFactionPanel::SelectIndex(int32 NewIndex)
{
	SelectedIndex = FMath::Clamp(NewIndex, 0, GetFactionInfo().Num() - 1);
}

FReply SSEEFactionPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Up || Key == EKeys::W)
	{
		SelectIndex(SelectedIndex - 1);
		return FReply::Handled();
	}
	if (Key == EKeys::Down || Key == EKeys::S)
	{
		SelectIndex(SelectedIndex + 1);
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}
