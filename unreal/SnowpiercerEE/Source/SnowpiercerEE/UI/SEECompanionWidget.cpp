// SEECompanionWidget.cpp
#include "SEECompanionWidget.h"
#include "TrainGame/Companions/CompanionComponent.h"
#include "TrainGame/Companions/CompanionRosterSubsystem.h"
#include "Components/TextBlock.h"

void USEECompanionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshCompanions();
}

UCompanionRosterSubsystem* USEECompanionWidget::GetRosterSubsystem() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UCompanionRosterSubsystem>() : nullptr;
}

UCompanionComponent* USEECompanionWidget::FindCompanion(FName CompanionID) const
{
	UCompanionRosterSubsystem* Roster = GetRosterSubsystem();
	if (!Roster) return nullptr;

	for (UCompanionComponent* Comp : Roster->GetFullRoster())
	{
		if (Comp && Comp->GetCompanionID() == CompanionID)
		{
			return Comp;
		}
	}
	return nullptr;
}

void USEECompanionWidget::RefreshCompanions()
{
	UCompanionRosterSubsystem* Roster = GetRosterSubsystem();
	if (!Roster) return;

	RosterEntries.Empty();

	for (UCompanionComponent* Comp : Roster->GetFullRoster())
	{
		if (!Comp) continue;

		FSEECompanionDisplayEntry Entry;
		Entry.CompanionID = Comp->GetCompanionID();
		Entry.Name = Comp->GetCompanionName();
		Entry.Role = Comp->GetRole();
		Entry.LoyaltyState = Comp->GetLoyaltyState();
		Entry.Behavior = Comp->GetBehaviorMode();
		Entry.Stats = Comp->GetStats();
		Entry.bInActiveParty = Comp->IsInActiveParty();
		Entry.bAlive = Comp->IsAlive();
		Entry.bRomanceActive = Comp->IsRomanceActive();
		Entry.CurrentQuest = Comp->GetCurrentQuestStep();

		RosterEntries.Add(Entry);
	}

	// Sort: active party first, then alive, then dead
	RosterEntries.Sort([](const FSEECompanionDisplayEntry& A, const FSEECompanionDisplayEntry& B)
	{
		if (A.bInActiveParty != B.bInActiveParty) return A.bInActiveParty;
		if (A.bAlive != B.bAlive) return A.bAlive;
		return false;
	});

	OnRosterRefreshed(RosterEntries);

	if (!SelectedCompanionID.IsNone())
	{
		SelectCompanion(SelectedCompanionID);
	}
}

void USEECompanionWidget::SelectCompanion(FName CompanionID)
{
	SelectedCompanionID = CompanionID;

	const FSEECompanionDisplayEntry* Found = nullptr;
	for (const FSEECompanionDisplayEntry& Entry : RosterEntries)
	{
		if (Entry.CompanionID == CompanionID)
		{
			Found = &Entry;
			break;
		}
	}

	if (!Found) return;

	if (CompanionNameText) CompanionNameText->SetText(Found->Name);

	if (LoyaltyText)
	{
		LoyaltyText->SetText(GetLoyaltyDisplayName(Found->LoyaltyState));
		LoyaltyText->SetColorAndOpacity(FSlateColor(GetLoyaltyColor(Found->LoyaltyState)));
	}

	if (RoleText) RoleText->SetText(GetRoleDisplayName(Found->Role));

	if (StatsText)
	{
		const FCompanionStats& S = Found->Stats;
		FString Stats = FString::Printf(
			TEXT("STR: %d  END: %d  CUN: %d\nPER: %d  CHA: %d  SUR: %d"),
			S.Strength, S.Endurance, S.Cunning, S.Perception, S.Charisma, S.Survival);
		StatsText->SetText(FText::FromString(Stats));
	}

	if (QuestText)
	{
		const FCompanionQuestStep& Q = Found->CurrentQuest;
		if (!Q.QuestID.IsNone())
		{
			FString QStr = FString::Printf(TEXT("%s\n%s"),
				*Q.QuestTitle.ToString(),
				*Q.QuestDescription.ToString());
			QuestText->SetText(FText::FromString(QStr));
			QuestText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			QuestText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (PartyStatusText)
	{
		if (!Found->bAlive)
		{
			PartyStatusText->SetText(NSLOCTEXT("Comp", "Dead", "DECEASED"));
			PartyStatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
		else if (Found->bInActiveParty)
		{
			PartyStatusText->SetText(NSLOCTEXT("Comp", "Active", "In Party"));
			PartyStatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
		else
		{
			PartyStatusText->SetText(NSLOCTEXT("Comp", "Benched", "Benched"));
			PartyStatusText->SetColorAndOpacity(FSlateColor(FLinearColor::Gray));
		}
	}

	OnCompanionSelected(*Found);
}

void USEECompanionWidget::SetBehavior(FName CompanionID, ECompanionBehavior Behavior)
{
	UCompanionComponent* Comp = FindCompanion(CompanionID);
	if (Comp)
	{
		Comp->SetBehaviorMode(Behavior);
		RefreshCompanions();
	}
}

void USEECompanionWidget::AddToParty(FName CompanionID)
{
	UCompanionRosterSubsystem* Roster = GetRosterSubsystem();
	UCompanionComponent* Comp = FindCompanion(CompanionID);
	if (Roster && Comp)
	{
		Roster->AddToActiveParty(Comp);
		RefreshCompanions();
	}
}

void USEECompanionWidget::RemoveFromParty(FName CompanionID)
{
	UCompanionRosterSubsystem* Roster = GetRosterSubsystem();
	UCompanionComponent* Comp = FindCompanion(CompanionID);
	if (Roster && Comp)
	{
		Roster->RemoveFromActiveParty(Comp);
		RefreshCompanions();
	}
}

FText USEECompanionWidget::GetLoyaltyDisplayName(ELoyaltyState State) const
{
	switch (State)
	{
	case ELoyaltyState::Hostile: return NSLOCTEXT("Comp", "Hostile", "Hostile");
	case ELoyaltyState::Resentful: return NSLOCTEXT("Comp", "Resentful", "Resentful");
	case ELoyaltyState::Cold: return NSLOCTEXT("Comp", "Cold", "Cold");
	case ELoyaltyState::Neutral: return NSLOCTEXT("Comp", "Neutral", "Neutral");
	case ELoyaltyState::Friendly: return NSLOCTEXT("Comp", "Friendly", "Friendly");
	case ELoyaltyState::Devoted: return NSLOCTEXT("Comp", "Devoted", "Devoted");
	case ELoyaltyState::Bonded: return NSLOCTEXT("Comp", "Bonded", "Bonded");
	default: return FText::GetEmpty();
	}
}

FLinearColor USEECompanionWidget::GetLoyaltyColor(ELoyaltyState State) const
{
	switch (State)
	{
	case ELoyaltyState::Hostile: return FLinearColor(0.8f, 0.1f, 0.1f);
	case ELoyaltyState::Resentful: return FLinearColor(0.8f, 0.4f, 0.2f);
	case ELoyaltyState::Cold: return FLinearColor(0.5f, 0.5f, 0.7f);
	case ELoyaltyState::Neutral: return FLinearColor(0.7f, 0.7f, 0.7f);
	case ELoyaltyState::Friendly: return FLinearColor(0.3f, 0.7f, 0.3f);
	case ELoyaltyState::Devoted: return FLinearColor(0.2f, 0.5f, 1.0f);
	case ELoyaltyState::Bonded: return FLinearColor(1.0f, 0.7f, 0.9f);
	default: return FLinearColor::White;
	}
}

FText USEECompanionWidget::GetRoleDisplayName(ECompanionRole Role) const
{
	switch (Role)
	{
	case ECompanionRole::Medic: return NSLOCTEXT("Comp", "Medic", "Medic");
	case ECompanionRole::MeleeDPS: return NSLOCTEXT("Comp", "MeleeDPS", "Melee DPS");
	case ECompanionRole::Tank: return NSLOCTEXT("Comp", "Tank", "Tank");
	case ECompanionRole::Stealth: return NSLOCTEXT("Comp", "Stealth", "Stealth");
	case ECompanionRole::Support: return NSLOCTEXT("Comp", "Support", "Support");
	case ECompanionRole::Utility: return NSLOCTEXT("Comp", "Utility", "Utility");
	case ECompanionRole::Ranged: return NSLOCTEXT("Comp", "Ranged", "Ranged");
	case ECompanionRole::Social: return NSLOCTEXT("Comp", "Social", "Social");
	case ECompanionRole::Alchemist: return NSLOCTEXT("Comp", "Alchemist", "Alchemist");
	case ECompanionRole::Scout: return NSLOCTEXT("Comp", "Scout", "Scout");
	case ECompanionRole::Electronic: return NSLOCTEXT("Comp", "EW", "Electronic Warfare");
	default: return FText::GetEmpty();
	}
}
