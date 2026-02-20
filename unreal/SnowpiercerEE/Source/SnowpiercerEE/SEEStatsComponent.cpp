#include "SEEStatsComponent.h"

USEEStatsComponent::USEEStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseStats.Add(ESEEStat::Strength, 5);
	BaseStats.Add(ESEEStat::Agility, 5);
	BaseStats.Add(ESEEStat::Endurance, 5);
	BaseStats.Add(ESEEStat::Cunning, 5);
	BaseStats.Add(ESEEStat::Perception, 5);
	BaseStats.Add(ESEEStat::Charisma, 5);
}

int32 USEEStatsComponent::GetStat(ESEEStat Stat) const
{
	int32 Base = BaseStats.Contains(Stat) ? BaseStats[Stat] : 5;
	int32 Mod = StatModifiers.Contains(Stat) ? StatModifiers[Stat] : 0;
	return FMath::Clamp(Base + Mod, 1, MaxStatValue);
}

void USEEStatsComponent::SetStat(ESEEStat Stat, int32 Value)
{
	BaseStats.FindOrAdd(Stat) = FMath::Clamp(Value, 1, MaxStatValue);
	OnStatChanged.Broadcast(Stat, GetStat(Stat));
}

void USEEStatsComponent::ModifyStat(ESEEStat Stat, int32 Delta)
{
	StatModifiers.FindOrAdd(Stat) += Delta;
	OnStatChanged.Broadcast(Stat, GetStat(Stat));
}

float USEEStatsComponent::GetStatModifier(ESEEStat Stat) const
{
	int32 Value = GetStat(Stat);
	return (Value - 5) * 0.1f;
}

void USEEStatsComponent::AddXP(int32 Amount)
{
	CurrentXP += Amount;
	int32 Required = GetXPToNextLevel();
	while (CurrentXP >= Required && CurrentLevel < 50)
	{
		CurrentXP -= Required;
		CurrentLevel++;
		AvailableSkillPoints += 2;
		OnLevelUp.Broadcast(CurrentLevel, AvailableSkillPoints);
		Required = GetXPToNextLevel();
	}
}

int32 USEEStatsComponent::GetXPToNextLevel() const
{
	return FMath::RoundToInt32(XPPerLevel * FMath::Pow(XPScaleFactor, CurrentLevel - 1));
}

bool USEEStatsComponent::SpendSkillPoint(ESEEStat Stat)
{
	if (AvailableSkillPoints <= 0) return false;
	int32 Current = BaseStats.Contains(Stat) ? BaseStats[Stat] : 5;
	if (Current >= MaxStatValue) return false;

	AvailableSkillPoints--;
	BaseStats.FindOrAdd(Stat) = Current + 1;
	OnStatChanged.Broadcast(Stat, GetStat(Stat));
	return true;
}

bool USEEStatsComponent::PassesSkillCheck(ESEEStat Stat, int32 Difficulty) const
{
	return GetStat(Stat) >= Difficulty;
}
