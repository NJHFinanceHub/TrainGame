// SSEESurvivalBars.cpp - Survival stat bars implementation
#include "SSEESurvivalBars.h"

#include "SnowyEngine/Survival/SurvivalComponent.h"
#include "SnowyEngine/Survival/KronoleComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEESurvivalBars::Construct(const FArguments& InArgs)
{
	SurvivalComp = InArgs._SurvivalComponent;
	KronoleComp = InArgs._KronoleComponent;

	// Initialize cached values
	CachedPercents.Add(ESurvivalStatType::Health, 1.0f);
	CachedPercents.Add(ESurvivalStatType::Stamina, 1.0f);
	CachedPercents.Add(ESurvivalStatType::Hunger, 1.0f);
	CachedPercents.Add(ESurvivalStatType::Cold, 1.0f);
	CachedPercents.Add(ESurvivalStatType::Morale, 1.0f);

	ChildSlot
	[
		// Anchor to bottom-left with padding
		SNew(SBox)
		.WidthOverride(220.0f)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(16.0f, 0.0f, 0.0f, 80.0f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
			.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.05f, 0.75f))
			.Padding(8.0f)
			[
				SNew(SVerticalBox)

				// Health
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					MakeStatBar(ESurvivalStatType::Health, NSLOCTEXT("HUD", "Health", "HP"), FLinearColor(0.8f, 0.15f, 0.1f))
				]

				// Stamina
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					MakeStatBar(ESurvivalStatType::Stamina, NSLOCTEXT("HUD", "Stamina", "STA"), FLinearColor(0.9f, 0.8f, 0.1f))
				]

				// Hunger
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					MakeStatBar(ESurvivalStatType::Hunger, NSLOCTEXT("HUD", "Hunger", "HGR"), FLinearColor(0.9f, 0.5f, 0.1f))
				]

				// Cold
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					MakeStatBar(ESurvivalStatType::Cold, NSLOCTEXT("HUD", "Cold", "CLD"), FLinearColor(0.2f, 0.7f, 0.9f))
				]

				// Morale
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 2)
				[
					MakeStatBar(ESurvivalStatType::Morale, NSLOCTEXT("HUD", "Morale", "MRL"), FLinearColor(0.6f, 0.3f, 0.8f))
				]

				// Kronole indicator (only visible when relevant)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 4, 0, 0)
				[
					MakeKronoleIndicator()
				]
			]
		]
	];
}

void SSEESurvivalBars::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	PulseTimer += InDeltaTime;

	if (SurvivalComp.IsValid())
	{
		for (auto& Pair : CachedPercents)
		{
			Pair.Value = SurvivalComp->GetStatPercent(Pair.Key);
		}
	}
}

TSharedRef<SWidget> SSEESurvivalBars::MakeStatBar(ESurvivalStatType StatType, const FText& Label, const FLinearColor& BarColor)
{
	return SNew(SHorizontalBox)

		// Label
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, 6, 0)
		[
			SNew(SBox)
			.WidthOverride(32.0f)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
			]
		]

		// Bar
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(12.0f)
			[
				SNew(SProgressBar)
				.Percent_Lambda([this, StatType]() { return GetStatPercent(StatType); })
				.FillColorAndOpacity_Lambda([this, StatType, BarColor]()
				{
					float Alpha = GetBarOpacity(StatType);
					return FLinearColor(BarColor.R, BarColor.G, BarColor.B, Alpha);
				})
				.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderPadding(FVector2D(0, 0))
			]
		];
}

TSharedRef<SWidget> SSEESurvivalBars::MakeKronoleIndicator()
{
	return SNew(SBox)
		.Visibility_Lambda([this]()
		{
			if (!KronoleComp.IsValid())
			{
				return EVisibility::Collapsed;
			}
			return KronoleComp->GetAddictionStage() != EKronoleAddictionStage::Clean
				? EVisibility::SelfHitTestInvisible
				: EVisibility::Collapsed;
		})
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 6, 0)
			[
				SNew(SBox)
				.WidthOverride(32.0f)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "Kronole", "KRN"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
					.ColorAndOpacity_Lambda([this]() { return GetKronoleStatusColor(); })
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return GetKronoleStatusText(); })
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity_Lambda([this]() { return GetKronoleStatusColor(); })
			]
		];
}

float SSEESurvivalBars::GetStatPercent(ESurvivalStatType StatType) const
{
	const float* Found = CachedPercents.Find(StatType);
	return Found ? *Found : 1.0f;
}

float SSEESurvivalBars::GetBarOpacity(ESurvivalStatType StatType) const
{
	float Percent = GetStatPercent(StatType);

	// Pulse effect when critical (below 15%)
	if (Percent < 0.15f)
	{
		float Pulse = (FMath::Sin(PulseTimer * 6.0f) + 1.0f) * 0.5f; // 0-1 oscillation
		return 0.5f + 0.5f * Pulse;
	}

	return 1.0f;
}

FText SSEESurvivalBars::GetKronoleStatusText() const
{
	if (!KronoleComp.IsValid())
	{
		return FText::GetEmpty();
	}

	if (KronoleComp->IsUnderEffect())
	{
		float Remaining = KronoleComp->GetRemainingBuffDuration();
		return FText::Format(NSLOCTEXT("HUD", "KronoleBuff", "ACTIVE {0}s"), FText::AsNumber(FMath::CeilToInt(Remaining)));
	}

	if (KronoleComp->IsInWithdrawal())
	{
		switch (KronoleComp->GetWithdrawalSeverity())
		{
		case EWithdrawalSeverity::Mild:     return NSLOCTEXT("HUD", "WithdrawalMild", "Withdrawal: Mild");
		case EWithdrawalSeverity::Moderate:  return NSLOCTEXT("HUD", "WithdrawalMod", "Withdrawal: Moderate");
		case EWithdrawalSeverity::Severe:    return NSLOCTEXT("HUD", "WithdrawalSev", "Withdrawal: Severe");
		case EWithdrawalSeverity::Critical:  return NSLOCTEXT("HUD", "WithdrawalCrit", "Withdrawal: CRITICAL");
		default: break;
		}
	}

	switch (KronoleComp->GetAddictionStage())
	{
	case EKronoleAddictionStage::Casual:    return NSLOCTEXT("HUD", "AddCasual", "Casual User");
	case EKronoleAddictionStage::Dependent: return NSLOCTEXT("HUD", "AddDep", "Dependent");
	case EKronoleAddictionStage::Addicted:  return NSLOCTEXT("HUD", "AddAddict", "Addicted");
	case EKronoleAddictionStage::Terminal:  return NSLOCTEXT("HUD", "AddTerm", "Terminal");
	default: break;
	}

	return FText::GetEmpty();
}

FSlateColor SSEESurvivalBars::GetKronoleStatusColor() const
{
	if (!KronoleComp.IsValid())
	{
		return FSlateColor(FLinearColor::White);
	}

	if (KronoleComp->IsUnderEffect())
	{
		// Bright green when buff active
		return FSlateColor(FLinearColor(0.2f, 1.0f, 0.3f));
	}

	if (KronoleComp->IsInWithdrawal())
	{
		// Red intensity scales with withdrawal severity
		float Severity = static_cast<float>(KronoleComp->GetWithdrawalSeverity()) / 4.0f;
		return FSlateColor(FLinearColor(1.0f, 0.3f * (1.0f - Severity), 0.1f));
	}

	// Neutral amber for addiction without withdrawal
	return FSlateColor(FLinearColor(0.9f, 0.7f, 0.2f));
}
