// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CompanionComponent.h"

UCompanionComponent::UCompanionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Tick at 10Hz, not every frame
}

void UCompanionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCompanionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeathCause != EPermadeathCause::None)
	{
		return;
	}

	UpdateCombatSynergy();
}

// --- Behavior Mode ---

void UCompanionComponent::SetBehaviorMode(ECompanionBehavior NewBehavior)
{
	if (CurrentBehavior != NewBehavior)
	{
		CurrentBehavior = NewBehavior;
		OnBehaviorChanged.Broadcast(NewBehavior);
	}
}

// --- Loyalty System ---

void UCompanionComponent::ApplyLoyaltyEvent(const FLoyaltyEvent& Event)
{
	if (DeathCause != EPermadeathCause::None)
	{
		return;
	}

	ELoyaltyState OldState = GetLoyaltyState();

	float Multiplier = (Event.LoyaltyChange > 0) ? PositiveLoyaltyMultiplier : NegativeLoyaltyMultiplier;
	LoyaltyScore += FMath::RoundToInt(Event.LoyaltyChange * Multiplier);
	ClampLoyalty();

	ELoyaltyState NewState = GetLoyaltyState();

	if (OldState != NewState)
	{
		OnLoyaltyStateChanged.Broadcast(OldState, NewState);
	}

	CheckAbandonmentThreshold();
}

ELoyaltyState UCompanionComponent::GetLoyaltyState() const
{
	return CalculateLoyaltyState(LoyaltyScore);
}

bool UCompanionComponent::MeetsLoyaltyThreshold(ELoyaltyState RequiredState) const
{
	return static_cast<uint8>(GetLoyaltyState()) >= static_cast<uint8>(RequiredState);
}

// --- Formation ---

void UCompanionComponent::SetFormationPosition(EFormationPosition Position)
{
	CurrentFormation = Position;
}

void UCompanionComponent::RequestFormationSwap()
{
	// Swap front/rear. Implementation delegates to the party manager.
	// Stub: toggle between front and rear.
	if (CurrentFormation == EFormationPosition::Front)
	{
		CurrentFormation = EFormationPosition::Rear;
	}
	else if (CurrentFormation == EFormationPosition::Rear)
	{
		CurrentFormation = EFormationPosition::Front;
	}
}

// --- Recruitment & Death ---

void UCompanionComponent::Recruit()
{
	bRecruited = true;
	bInActiveParty = true;
}

void UCompanionComponent::DismissToRoster()
{
	bInActiveParty = false;
}

void UCompanionComponent::ActivateFromRoster()
{
	if (bRecruited && IsAlive())
	{
		bInActiveParty = true;
	}
}

void UCompanionComponent::TriggerPermadeath(EPermadeathCause Cause)
{
	if (DeathCause != EPermadeathCause::None)
	{
		return; // Already dead
	}

	DeathCause = Cause;
	bInActiveParty = false;
	OnCompanionDeath.Broadcast(Cause);
}

// --- Unique Ability ---

bool UCompanionComponent::TryUseUniqueAbility()
{
	if (!IsUniqueAbilityAvailable())
	{
		return false;
	}

	AbilityUsesThisEncounter++;

	// Ability execution is handled by subclass or companion-specific Blueprint
	return true;
}

bool UCompanionComponent::IsUniqueAbilityAvailable() const
{
	if (DeathCause != EPermadeathCause::None)
	{
		return false;
	}

	int32 MaxUses = MaxAbilityUsesPerEncounter;
	if (GetLoyaltyState() == ELoyaltyState::Bonded)
	{
		MaxUses += BondedAbilityUsesPerEncounter;
	}

	return AbilityUsesThisEncounter < MaxUses;
}

// --- Personal Quests ---

void UCompanionComponent::CompleteQuestStep(FName QuestID)
{
	for (FCompanionQuestStep& Step : PersonalQuestSteps)
	{
		if (Step.QuestID == QuestID && !Step.bCompleted)
		{
			Step.bCompleted = true;
			OnQuestStepCompleted.Broadcast(QuestID);

			// Completing a quest step is a major positive loyalty event
			FLoyaltyEvent QuestEvent;
			QuestEvent.EventType = ELoyaltyEventType::PersonalQuestComplete;
			QuestEvent.LoyaltyChange = 15;
			QuestEvent.EventDescription = FText::Format(
				FText::FromString(TEXT("Completed quest: {0}")),
				Step.QuestTitle
			);
			ApplyLoyaltyEvent(QuestEvent);
			break;
		}
	}
}

FCompanionQuestStep UCompanionComponent::GetCurrentQuestStep() const
{
	for (const FCompanionQuestStep& Step : PersonalQuestSteps)
	{
		if (!Step.bCompleted)
		{
			return Step;
		}
	}
	return FCompanionQuestStep();
}

// --- Romance ---

void UCompanionComponent::InitiateRomance()
{
	if (bRomanceable && MeetsLoyaltyThreshold(ELoyaltyState::Devoted))
	{
		bRomanceActive = true;
	}
}

void UCompanionComponent::EndRomance()
{
	bRomanceActive = false;
}

// --- Private ---

ELoyaltyState UCompanionComponent::CalculateLoyaltyState(int32 Score) const
{
	if (Score <= -60) return ELoyaltyState::Hostile;
	if (Score <= -30) return ELoyaltyState::Resentful;
	if (Score <= 0) return ELoyaltyState::Cold;
	if (Score <= 30) return ELoyaltyState::Neutral;
	if (Score <= 60) return ELoyaltyState::Friendly;
	if (Score <= 85) return ELoyaltyState::Devoted;
	return ELoyaltyState::Bonded;
}

void UCompanionComponent::ClampLoyalty()
{
	LoyaltyScore = FMath::Clamp(LoyaltyScore, -100, 100);
}

void UCompanionComponent::CheckAbandonmentThreshold()
{
	if (GetLoyaltyState() == ELoyaltyState::Hostile)
	{
		// Companion abandons unless Oathkeeper perk is active
		// Oathkeeper check is handled by the party subsystem
		// Broadcast event for party manager to handle
	}
}

void UCompanionComponent::UpdateCombatSynergy()
{
	// Combat synergy bonus based on loyalty state
	// Applied through the combat component modifier system
	// Stub: actual implementation connects to UCombatComponent
}
