// SEECharacterWidget.cpp
#include "SEECharacterWidget.h"
#include "SEECharacter.h"
#include "SEEHealthComponent.h"
#include "SEEInventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void USEECharacterWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void USEECharacterWidget::InitCharacter(ASEECharacter* Character)
{
	if (!Character) return;

	CachedCharacter = Character;
	StatsComp = Character->FindComponentByClass<USEEStatsComponent>();
	HealthComp = Character->FindComponentByClass<USEEHealthComponent>();

	RefreshStats();
}

void USEECharacterWidget::RefreshStats()
{
	if (!StatsComp.IsValid()) return;

	UpdateStatText(StrengthText, ESEEStat::Strength);
	UpdateStatText(AgilityText, ESEEStat::Agility);
	UpdateStatText(EnduranceText, ESEEStat::Endurance);
	UpdateStatText(CunningText, ESEEStat::Cunning);
	UpdateStatText(PerceptionText, ESEEStat::Perception);
	UpdateStatText(CharismaText, ESEEStat::Charisma);

	if (LevelText)
	{
		LevelText->SetText(FText::Format(
			NSLOCTEXT("Char", "Level", "Level {0}"),
			FText::AsNumber(StatsComp->GetLevel())));
	}

	if (XPText)
	{
		XPText->SetText(FText::Format(
			NSLOCTEXT("Char", "XP", "XP: {0} / {1}"),
			FText::AsNumber(StatsComp->GetCurrentXP()),
			FText::AsNumber(StatsComp->GetXPToNextLevel())));
	}

	if (XPBar)
	{
		int32 XPNeeded = StatsComp->GetXPToNextLevel();
		float Percent = XPNeeded > 0
			? static_cast<float>(StatsComp->GetCurrentXP()) / static_cast<float>(XPNeeded)
			: 1.0f;
		XPBar->SetPercent(FMath::Clamp(Percent, 0.0f, 1.0f));
	}

	if (SkillPointsText)
	{
		int32 SP = StatsComp->GetSkillPoints();
		SkillPointsText->SetText(FText::Format(
			NSLOCTEXT("Char", "SP", "Skill Points: {0}"),
			FText::AsNumber(SP)));
		SkillPointsText->SetColorAndOpacity(
			SP > 0 ? FSlateColor(FLinearColor(0.2f, 1.0f, 0.2f)) : FSlateColor(FLinearColor::White));
	}

	// Health summary
	if (HealthComp.IsValid() && HealthSummaryText)
	{
		HealthSummaryText->SetText(FText::Format(
			NSLOCTEXT("Char", "HP", "HP: {0} / {1}"),
			FText::AsNumber(FMath::RoundToInt32(HealthComp->GetCurrentHealth())),
			FText::AsNumber(FMath::RoundToInt32(HealthComp->GetMaxHealth()))));
	}

	if (HealthComp.IsValid() && InjurySummaryText)
	{
		TArray<FSEEInjury> Injuries = HealthComp->GetActiveInjuries();
		if (Injuries.Num() > 0)
		{
			FString InjuryStr;
			for (const FSEEInjury& Inj : Injuries)
			{
				switch (Inj.Type)
				{
				case ESEEInjuryType::BrokenArm: InjuryStr += TEXT("Broken Arm (-30% melee)\n"); break;
				case ESEEInjuryType::LegWound: InjuryStr += TEXT("Leg Wound (-40% speed)\n"); break;
				case ESEEInjuryType::Concussion: InjuryStr += TEXT("Concussion\n"); break;
				case ESEEInjuryType::DeepCut: InjuryStr += TEXT("Deep Cut\n"); break;
				case ESEEInjuryType::InternalBleeding: InjuryStr += TEXT("Internal Bleeding\n"); break;
				default: break;
				}
			}
			InjurySummaryText->SetText(FText::FromString(InjuryStr));
			InjurySummaryText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			InjurySummaryText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	OnStatsRefreshed();
}

bool USEECharacterWidget::SpendSkillPoint(ESEEStat Stat)
{
	if (!StatsComp.IsValid()) return false;

	bool bResult = StatsComp->SpendSkillPoint(Stat);
	if (bResult)
	{
		RefreshStats();
	}
	return bResult;
}

int32 USEECharacterWidget::GetStatValue(ESEEStat Stat) const
{
	if (!StatsComp.IsValid()) return 0;
	return StatsComp->GetStat(Stat);
}

FText USEECharacterWidget::GetStatDisplayName(ESEEStat Stat) const
{
	switch (Stat)
	{
	case ESEEStat::Strength: return NSLOCTEXT("Char", "STR", "Strength");
	case ESEEStat::Agility: return NSLOCTEXT("Char", "AGI", "Agility");
	case ESEEStat::Endurance: return NSLOCTEXT("Char", "END", "Endurance");
	case ESEEStat::Cunning: return NSLOCTEXT("Char", "CUN", "Cunning");
	case ESEEStat::Perception: return NSLOCTEXT("Char", "PER", "Perception");
	case ESEEStat::Charisma: return NSLOCTEXT("Char", "CHA", "Charisma");
	default: return FText::GetEmpty();
	}
}

void USEECharacterWidget::UpdateStatText(UTextBlock* Block, ESEEStat Stat)
{
	if (!Block || !StatsComp.IsValid()) return;

	int32 Value = StatsComp->GetStat(Stat);
	Block->SetText(FText::Format(
		NSLOCTEXT("Char", "StatFmt", "{0}: {1}"),
		GetStatDisplayName(Stat),
		FText::AsNumber(Value)));
}
