#include "SEEFactionManager.h"

void USEEFactionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize all factions to neutral (0)
	FactionReputations.Add(ESEEFaction::Tailies, 0);
	FactionReputations.Add(ESEEFaction::ThirdClassUnion, 0);
	FactionReputations.Add(ESEEFaction::Jackboots, 0);
	FactionReputations.Add(ESEEFaction::Bureaucracy, 0);
	FactionReputations.Add(ESEEFaction::FirstClassElite, 0);
	FactionReputations.Add(ESEEFaction::OrderOfTheEngine, 0);
	FactionReputations.Add(ESEEFaction::KronoleNetwork, 0);
	FactionReputations.Add(ESEEFaction::TheThaw, 0);
}

void USEEFactionManager::ModifyReputation(ESEEFaction Faction, int32 Delta)
{
	ESEEFactionStanding OldStanding = GetStanding(Faction);

	int32& Rep = FactionReputations.FindOrAdd(Faction);
	Rep = FMath::Clamp(Rep + Delta, -100, 100);

	OnFactionRepChanged.Broadcast(Faction, Rep);

	ESEEFactionStanding NewStanding = GetStanding(Faction);
	if (OldStanding != NewStanding)
	{
		OnFactionStandingChanged.Broadcast(Faction, NewStanding);
	}

	// Apply mutual exclusivity
	ApplyMutualExclusivity(Faction, Delta);
}

void USEEFactionManager::SetReputation(ESEEFaction Faction, int32 Value)
{
	int32& Rep = FactionReputations.FindOrAdd(Faction);
	ESEEFactionStanding OldStanding = GetStanding(Faction);

	Rep = FMath::Clamp(Value, -100, 100);
	OnFactionRepChanged.Broadcast(Faction, Rep);

	ESEEFactionStanding NewStanding = GetStanding(Faction);
	if (OldStanding != NewStanding)
	{
		OnFactionStandingChanged.Broadcast(Faction, NewStanding);
	}
}

int32 USEEFactionManager::GetReputation(ESEEFaction Faction) const
{
	const int32* Rep = FactionReputations.Find(Faction);
	return Rep ? *Rep : 0;
}

ESEEFactionStanding USEEFactionManager::GetStanding(ESEEFaction Faction) const
{
	return RepToStanding(GetReputation(Faction));
}

bool USEEFactionManager::IsHostile(ESEEFaction Faction) const
{
	return GetReputation(Faction) <= -50;
}

bool USEEFactionManager::IsFriendly(ESEEFaction Faction) const
{
	return GetReputation(Faction) >= 25;
}

float USEEFactionManager::GetPriceModifier(ESEEFaction Faction) const
{
	int32 Rep = GetReputation(Faction);
	// -100 rep = 150% prices, 0 = 100%, +100 = 75%
	return 1.0f - (Rep / 400.0f);
}

void USEEFactionManager::ApplyMutualExclusivity(ESEEFaction Faction, int32 Delta)
{
	if (Delta == 0) return;

	// Opposing factions lose rep when one gains
	int32 OpposingDelta = -FMath::Abs(Delta) / 3;

	if (Faction == ESEEFaction::Jackboots && Delta > 0)
	{
		ModifyReputation(ESEEFaction::Tailies, OpposingDelta);
	}
	else if (Faction == ESEEFaction::Tailies && Delta > 0)
	{
		ModifyReputation(ESEEFaction::Jackboots, OpposingDelta);
	}

	if (Faction == ESEEFaction::OrderOfTheEngine && Delta > 0)
	{
		ModifyReputation(ESEEFaction::TheThaw, OpposingDelta);
	}
	else if (Faction == ESEEFaction::TheThaw && Delta > 0)
	{
		ModifyReputation(ESEEFaction::OrderOfTheEngine, OpposingDelta);
	}
}

ESEEFactionStanding USEEFactionManager::RepToStanding(int32 Rep) const
{
	if (Rep <= -75) return ESEEFactionStanding::Hostile;
	if (Rep <= -25) return ESEEFactionStanding::Unfriendly;
	if (Rep < 25) return ESEEFactionStanding::Neutral;
	if (Rep < 50) return ESEEFactionStanding::Friendly;
	if (Rep < 75) return ESEEFactionStanding::Allied;
	return ESEEFactionStanding::Devoted;
}
