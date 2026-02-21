// SEEFactionWidget.cpp
#include "SEEFactionWidget.h"
#include "SEEFactionManager.h"

void USEEFactionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshFactions();
}

USEEFactionManager* USEEFactionWidget::GetFactionManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USEEFactionManager>() : nullptr;
}

void USEEFactionWidget::RefreshFactions()
{
	USEEFactionManager* FM = GetFactionManager();
	if (!FM) return;

	FactionEntries.Empty();

	// All factions except Neutral
	static const ESEEFaction AllFactions[] = {
		ESEEFaction::Tailies,
		ESEEFaction::ThirdClassUnion,
		ESEEFaction::Jackboots,
		ESEEFaction::Bureaucracy,
		ESEEFaction::FirstClassElite,
		ESEEFaction::OrderOfTheEngine,
		ESEEFaction::KronoleNetwork,
		ESEEFaction::TheThaw
	};

	for (ESEEFaction F : AllFactions)
	{
		FSEEFactionDisplayEntry Entry;
		Entry.Faction = F;
		Entry.FactionName = GetFactionName(F);
		Entry.Reputation = FM->GetReputation(F);
		Entry.Standing = FM->GetStanding(F);
		Entry.StandingText = GetStandingDisplayName(Entry.Standing);
		Entry.StandingColor = GetStandingColor(Entry.Standing);
		FactionEntries.Add(Entry);
	}

	OnFactionsRefreshed(FactionEntries);
}

FText USEEFactionWidget::GetFactionName(ESEEFaction Faction) const
{
	switch (Faction)
	{
	case ESEEFaction::Tailies: return NSLOCTEXT("Fac", "Tailies", "Tailies");
	case ESEEFaction::ThirdClassUnion: return NSLOCTEXT("Fac", "ThirdUnion", "Third Class Union");
	case ESEEFaction::Jackboots: return NSLOCTEXT("Fac", "Jackboots", "Jackboots");
	case ESEEFaction::Bureaucracy: return NSLOCTEXT("Fac", "Bureau", "Bureaucracy");
	case ESEEFaction::FirstClassElite: return NSLOCTEXT("Fac", "FirstElite", "First Class Elite");
	case ESEEFaction::OrderOfTheEngine: return NSLOCTEXT("Fac", "OrderEngine", "Order of the Engine");
	case ESEEFaction::KronoleNetwork: return NSLOCTEXT("Fac", "Kronole", "Kronole Network");
	case ESEEFaction::TheThaw: return NSLOCTEXT("Fac", "TheThaw", "The Thaw");
	default: return NSLOCTEXT("Fac", "Neutral", "Neutral");
	}
}

FText USEEFactionWidget::GetStandingDisplayName(ESEEFactionStanding Standing) const
{
	switch (Standing)
	{
	case ESEEFactionStanding::Hostile: return NSLOCTEXT("Fac", "Hostile", "Hostile");
	case ESEEFactionStanding::Unfriendly: return NSLOCTEXT("Fac", "Unfriendly", "Unfriendly");
	case ESEEFactionStanding::Neutral: return NSLOCTEXT("Fac", "Neutral", "Neutral");
	case ESEEFactionStanding::Friendly: return NSLOCTEXT("Fac", "Friendly", "Friendly");
	case ESEEFactionStanding::Allied: return NSLOCTEXT("Fac", "Allied", "Allied");
	case ESEEFactionStanding::Devoted: return NSLOCTEXT("Fac", "Devoted", "Devoted");
	default: return FText::GetEmpty();
	}
}

FLinearColor USEEFactionWidget::GetStandingColor(ESEEFactionStanding Standing) const
{
	switch (Standing)
	{
	case ESEEFactionStanding::Hostile: return FLinearColor(0.8f, 0.1f, 0.1f);
	case ESEEFactionStanding::Unfriendly: return FLinearColor(0.8f, 0.4f, 0.2f);
	case ESEEFactionStanding::Neutral: return FLinearColor(0.7f, 0.7f, 0.7f);
	case ESEEFactionStanding::Friendly: return FLinearColor(0.3f, 0.7f, 0.3f);
	case ESEEFactionStanding::Allied: return FLinearColor(0.2f, 0.5f, 1.0f);
	case ESEEFactionStanding::Devoted: return FLinearColor(0.8f, 0.6f, 1.0f);
	default: return FLinearColor::White;
	}
}

float USEEFactionWidget::GetReputationPercent(int32 Reputation) const
{
	// Reputation ranges from -100 to 100, normalize to 0-1
	return FMath::Clamp((static_cast<float>(Reputation) + 100.0f) / 200.0f, 0.0f, 1.0f);
}
