// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CompanionRosterSubsystem.h"
#include "CompanionComponent.h"

void UCompanionRosterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

// --- Party Management ---

bool UCompanionRosterSubsystem::AddToRoster(UCompanionComponent* Companion)
{
	if (!Companion || Roster.Num() >= MAX_ROSTER_SIZE)
	{
		return false;
	}

	if (Roster.Contains(Companion))
	{
		return false;
	}

	Roster.Add(Companion);
	Companion->Recruit();
	return true;
}

bool UCompanionRosterSubsystem::AddToActiveParty(UCompanionComponent* Companion)
{
	if (!Companion || !Companion->IsRecruited() || !Companion->IsAlive())
	{
		return false;
	}

	if (ActiveParty.Num() >= MAX_ACTIVE_PARTY_SIZE)
	{
		return false;
	}

	if (ActiveParty.Contains(Companion))
	{
		return false;
	}

	ActiveParty.Add(Companion);
	Companion->ActivateFromRoster();
	return true;
}

void UCompanionRosterSubsystem::RemoveFromActiveParty(UCompanionComponent* Companion)
{
	if (Companion && ActiveParty.Contains(Companion))
	{
		ActiveParty.Remove(Companion);
		Companion->DismissToRoster();
	}
}

TArray<UCompanionComponent*> UCompanionRosterSubsystem::GetBenchedCompanions() const
{
	TArray<UCompanionComponent*> Benched;
	for (UCompanionComponent* Comp : Roster)
	{
		if (Comp && Comp->IsAlive() && !Comp->IsInActiveParty())
		{
			Benched.Add(Comp);
		}
	}
	return Benched;
}

// --- Permadeath ---

TArray<UCompanionComponent*> UCompanionRosterSubsystem::GetDeadCompanions() const
{
	TArray<UCompanionComponent*> Dead;
	for (UCompanionComponent* Comp : Roster)
	{
		if (Comp && !Comp->IsAlive())
		{
			Dead.Add(Comp);
		}
	}
	return Dead;
}

bool UCompanionRosterSubsystem::IsCompanionDead(FName CompanionID) const
{
	for (const UCompanionComponent* Comp : Roster)
	{
		if (Comp && Comp->GetCompanionID() == CompanionID)
		{
			return !Comp->IsAlive();
		}
	}
	return false;
}

// --- Companion Conflicts ---

void UCompanionRosterSubsystem::TriggerConflict(const FCompanionConflict& Conflict)
{
	CurrentConflict = Conflict;
	bConflictActive = true;

	// Notify both companions
	for (UCompanionComponent* Comp : ActiveParty)
	{
		if (Comp)
		{
			Comp->OnCompanionConflict.Broadcast(Conflict.CompanionA, Conflict.CompanionB);
		}
	}
}

void UCompanionRosterSubsystem::ResolveConflict(FName WinnerID, bool bCompromise)
{
	if (!bConflictActive)
	{
		return;
	}

	FName LoserID = (WinnerID == CurrentConflict.CompanionA) ?
		CurrentConflict.CompanionB : CurrentConflict.CompanionA;

	int32 Penalty = CurrentConflict.LoyaltyPenaltyForLoser;
	if (bCompromise)
	{
		Penalty += CurrentConflict.CompromisePenaltyReduction; // Reduces magnitude
	}

	// Apply loyalty penalty to the losing companion
	for (UCompanionComponent* Comp : Roster)
	{
		if (Comp && Comp->GetCompanionID() == LoserID)
		{
			FLoyaltyEvent ConflictEvent;
			ConflictEvent.EventType = ELoyaltyEventType::ConflictOpposed;
			ConflictEvent.LoyaltyChange = Penalty;
			ConflictEvent.EventDescription = FText::FromString(TEXT("Lost a companion conflict"));
			Comp->ApplyLoyaltyEvent(ConflictEvent);
			break;
		}
	}

	bConflictActive = false;
}

// --- Synergy ---

float UCompanionRosterSubsystem::GetPairSynergyBonus(FName CompanionA, FName CompanionB) const
{
	// Synergy pairs are defined in data assets. Stub returns default.
	// See COMPANION_ROSTER.md for pair definitions.
	return 0.0f;
}

bool UCompanionRosterSubsystem::IsSynergyPairActive(FName CompanionA, FName CompanionB) const
{
	bool bFoundA = false;
	bool bFoundB = false;

	for (const UCompanionComponent* Comp : ActiveParty)
	{
		if (Comp)
		{
			if (Comp->GetCompanionID() == CompanionA) bFoundA = true;
			if (Comp->GetCompanionID() == CompanionB) bFoundB = true;
		}
	}

	return bFoundA && bFoundB;
}

// --- Romance ---

UCompanionComponent* UCompanionRosterSubsystem::GetRomancedCompanion() const
{
	for (UCompanionComponent* Comp : Roster)
	{
		if (Comp && Comp->IsRomanceActive())
		{
			return Comp;
		}
	}
	return nullptr;
}

bool UCompanionRosterSubsystem::HasActiveRomance() const
{
	return GetRomancedCompanion() != nullptr;
}

// --- Memorial ---

void UCompanionRosterSubsystem::TriggerMemorial(FName DeceasedCompanionID)
{
	// Stub: triggers the 15-second memorial cutscene specific to the deceased companion
	// Implementation loads companion-specific memorial sequence from data asset
}

// --- Perk Integration ---

void UCompanionRosterSubsystem::SetNaturalLeaderActive(bool bActive)
{
	float PositiveMultiplier = bActive ? 1.2f : 1.0f;
	float NegativeMultiplier = bActive ? 0.8f : 1.0f;

	for (UCompanionComponent* Comp : Roster)
	{
		if (Comp)
		{
			// Natural Leader: +20% positive loyalty gains, -20% negative hits
			// Set via the companion component's multiplier properties
		}
	}
}
