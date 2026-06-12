#include "SEEFactionManager.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

// ============================================================================
// Reputation event matrix
//
// Fixed-delta events (kills, helping civilians) ripple across every faction
// with an opinion. Faction-targeted events (quests, goodwill, trade) reward
// the context faction and nick its rivals. All deltas applied raw — the
// table IS the ripple, so no mutual-exclusivity double-dipping.
// ============================================================================

namespace
{
	struct FFactionDelta
	{
		ESEEFaction Faction;
		int32 Delta;
	};

	using FDeltaList = TArray<FFactionDelta, TInlineAllocator<8>>;

	const FDeltaList* GetFixedEventDeltas(ESEEReputationEvent Event)
	{
		// Killing a Jackboot: the enforcers and the front despise it; the Tail
		// passes the word that one less baton walks the corridors.
		static const FDeltaList KilledJackboot = {
			{ ESEEFaction::Jackboots,        -8 },
			{ ESEEFaction::Bureaucracy,      -4 },
			{ ESEEFaction::FirstClassElite,  -3 },
			{ ESEEFaction::OrderOfTheEngine, -2 },
			{ ESEEFaction::Tailies,          +5 },
			{ ESEEFaction::ThirdClassUnion,  +2 },
			{ ESEEFaction::TheThaw,          +3 },
		};

		// Murdering a civilian: murder matters. The Tail above all — solidarity
		// is their only currency. The Jackboots don't care; one less mouth.
		static const FDeltaList KilledCivilian = {
			{ ESEEFaction::Tailies,         -15 },
			{ ESEEFaction::ThirdClassUnion,  -8 },
			{ ESEEFaction::Bureaucracy,      -5 },
			{ ESEEFaction::TheThaw,          -4 },
			{ ESEEFaction::KronoleNetwork,   -3 },
			{ ESEEFaction::FirstClassElite,  -2 },
		};

		// Killing a merchant: the trade lanes remember. The Network takes it
		// personally — dead dealers are bad for business.
		static const FDeltaList KilledMerchant = {
			{ ESEEFaction::KronoleNetwork,  -12 },
			{ ESEEFaction::ThirdClassUnion, -10 },
			{ ESEEFaction::Tailies,          -6 },
			{ ESEEFaction::Bureaucracy,      -4 },
		};

		// Helping a civilian (bandages, food, a found child): word travels
		// down-train fast. The Jackboots see softness.
		static const FDeltaList HelpedCivilian = {
			{ ESEEFaction::Tailies,         +4 },
			{ ESEEFaction::ThirdClassUnion, +2 },
			{ ESEEFaction::Jackboots,       -1 },
		};

		switch (Event)
		{
		case ESEEReputationEvent::KilledJackboot:  return &KilledJackboot;
		case ESEEReputationEvent::KilledCivilian:  return &KilledCivilian;
		case ESEEReputationEvent::KilledMerchant:  return &KilledMerchant;
		case ESEEReputationEvent::HelpedCivilian:  return &HelpedCivilian;
		default:                                   return nullptr; // context-targeted event
		}
	}

	/** Rivals nicked when a faction-targeted event rewards their enemy. */
	void GetRivals(ESEEFaction Faction, FDeltaList& OutRivals, int32 RivalDelta)
	{
		auto Add = [&OutRivals, RivalDelta](ESEEFaction F) { OutRivals.Add({ F, RivalDelta }); };
		switch (Faction)
		{
		case ESEEFaction::Tailies:          Add(ESEEFaction::Jackboots); Add(ESEEFaction::FirstClassElite); break;
		case ESEEFaction::Jackboots:        Add(ESEEFaction::Tailies); break;
		case ESEEFaction::FirstClassElite:  Add(ESEEFaction::Tailies); break;
		case ESEEFaction::OrderOfTheEngine: Add(ESEEFaction::TheThaw); break;
		case ESEEFaction::TheThaw:          Add(ESEEFaction::OrderOfTheEngine); break;
		case ESEEFaction::Bureaucracy:      Add(ESEEFaction::KronoleNetwork); break;
		case ESEEFaction::KronoleNetwork:   Add(ESEEFaction::Bureaucracy); break;
		default: break;
		}
	}
}

// ============================================================================
// Lifecycle
// ============================================================================

void USEEFactionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Starting standings: the player is Tail-born (Accepted with their own),
	// and the Jackboots already have a file on them.
	FactionReputations.Add(ESEEFaction::Tailies, 20);
	FactionReputations.Add(ESEEFaction::ThirdClassUnion, 0);
	FactionReputations.Add(ESEEFaction::Jackboots, -10);
	FactionReputations.Add(ESEEFaction::Bureaucracy, 0);
	FactionReputations.Add(ESEEFaction::FirstClassElite, 0);
	FactionReputations.Add(ESEEFaction::OrderOfTheEngine, 0);
	FactionReputations.Add(ESEEFaction::KronoleNetwork, 0);
	FactionReputations.Add(ESEEFaction::TheThaw, 0);
}

// ============================================================================
// Raw reputation
// ============================================================================

void USEEFactionManager::ApplyDeltaInternal(ESEEFaction Faction, int32 Delta)
{
	if (Delta == 0 || Faction == ESEEFaction::Neutral) return;

	int32& Rep = FactionReputations.FindOrAdd(Faction);
	const ESEEFactionStanding OldStanding = RepToStanding(Rep);

	Rep = FMath::Clamp(Rep + Delta, -100, 100);
	OnFactionRepChanged.Broadcast(Faction, Rep);

	const ESEEFactionStanding NewStanding = RepToStanding(Rep);
	if (OldStanding != NewStanding)
	{
		OnFactionStandingChanged.Broadcast(Faction, OldStanding, NewStanding);
	}
}

void USEEFactionManager::ModifyReputation(ESEEFaction Faction, int32 Delta)
{
	ApplyDeltaInternal(Faction, Delta);
	ApplyMutualExclusivity(Faction, Delta);
}

void USEEFactionManager::SetReputation(ESEEFaction Faction, int32 Value)
{
	ApplyDeltaInternal(Faction, FMath::Clamp(Value, -100, 100) - GetReputation(Faction));
}

int32 USEEFactionManager::GetReputation(ESEEFaction Faction) const
{
	const int32* Rep = FactionReputations.Find(Faction);
	return Rep ? *Rep : 0;
}

void USEEFactionManager::ApplyMutualExclusivity(ESEEFaction Faction, int32 Delta)
{
	if (Delta <= 0) return;

	// Opposing factions lose rep when one gains (direct ModifyReputation only;
	// ApplyReputationEvent encodes its ripples in the table instead).
	const int32 OpposingDelta = -Delta / 3;
	if (OpposingDelta == 0) return;

	if (Faction == ESEEFaction::Jackboots)
	{
		ApplyDeltaInternal(ESEEFaction::Tailies, OpposingDelta);
	}
	else if (Faction == ESEEFaction::Tailies)
	{
		ApplyDeltaInternal(ESEEFaction::Jackboots, OpposingDelta);
	}

	if (Faction == ESEEFaction::OrderOfTheEngine)
	{
		ApplyDeltaInternal(ESEEFaction::TheThaw, OpposingDelta);
	}
	else if (Faction == ESEEFaction::TheThaw)
	{
		ApplyDeltaInternal(ESEEFaction::OrderOfTheEngine, OpposingDelta);
	}
}

// ============================================================================
// Standing tiers
// ============================================================================

ESEEFactionStanding USEEFactionManager::GetStanding(ESEEFaction Faction) const
{
	return RepToStanding(GetReputation(Faction));
}

ESEEFactionStanding USEEFactionManager::RepToStanding(int32 Rep) const
{
	return StandingForReputation(Rep);
}

ESEEFactionStanding USEEFactionManager::StandingForReputation(int32 Reputation)
{
	if (Reputation <= -60) return ESEEFactionStanding::Hated;
	if (Reputation <= -25) return ESEEFactionStanding::Hostile;
	if (Reputation <= -5)  return ESEEFactionStanding::Unfriendly;
	if (Reputation <= 14)  return ESEEFactionStanding::Neutral;
	if (Reputation <= 39)  return ESEEFactionStanding::Accepted;
	if (Reputation <= 69)  return ESEEFactionStanding::Friendly;
	return ESEEFactionStanding::Revered;
}

FText USEEFactionManager::GetStandingDisplayName(ESEEFactionStanding Standing)
{
	switch (Standing)
	{
	case ESEEFactionStanding::Hated:      return NSLOCTEXT("Fac", "TierHated", "Hated");
	case ESEEFactionStanding::Hostile:    return NSLOCTEXT("Fac", "TierHostile", "Hostile");
	case ESEEFactionStanding::Unfriendly: return NSLOCTEXT("Fac", "TierUnfriendly", "Unfriendly");
	case ESEEFactionStanding::Neutral:    return NSLOCTEXT("Fac", "TierNeutral", "Neutral");
	case ESEEFactionStanding::Accepted:   return NSLOCTEXT("Fac", "TierAccepted", "Accepted");
	case ESEEFactionStanding::Friendly:   return NSLOCTEXT("Fac", "TierFriendly", "Friendly");
	case ESEEFactionStanding::Revered:    return NSLOCTEXT("Fac", "TierRevered", "Revered");
	default:                              return NSLOCTEXT("Fac", "TierUnknown", "Unknown");
	}
}

FLinearColor USEEFactionManager::GetStandingColor(ESEEFactionStanding Standing)
{
	switch (Standing)
	{
	case ESEEFactionStanding::Hated:      return FLinearColor(0.48f, 0.05f, 0.05f); // deep red
	case ESEEFactionStanding::Hostile:    return FLinearColor(0.85f, 0.20f, 0.12f); // red
	case ESEEFactionStanding::Unfriendly: return FLinearColor(0.90f, 0.50f, 0.20f); // orange
	case ESEEFactionStanding::Neutral:    return FLinearColor(0.65f, 0.65f, 0.70f); // steel grey
	case ESEEFactionStanding::Accepted:   return FLinearColor(0.60f, 0.75f, 0.45f); // pale green
	case ESEEFactionStanding::Friendly:   return FLinearColor(0.32f, 0.75f, 0.35f); // green
	case ESEEFactionStanding::Revered:    return FLinearColor(1.00f, 0.78f, 0.25f); // gold
	default:                              return FLinearColor::White;
	}
}

void USEEFactionManager::GetStandingRange(ESEEFactionStanding Standing, int32& OutMinRep, int32& OutMaxRep)
{
	switch (Standing)
	{
	case ESEEFactionStanding::Hated:      OutMinRep = -100; OutMaxRep = -60; break;
	case ESEEFactionStanding::Hostile:    OutMinRep = -59;  OutMaxRep = -25; break;
	case ESEEFactionStanding::Unfriendly: OutMinRep = -24;  OutMaxRep = -5;  break;
	case ESEEFactionStanding::Neutral:    OutMinRep = -4;   OutMaxRep = 14;  break;
	case ESEEFactionStanding::Accepted:   OutMinRep = 15;   OutMaxRep = 39;  break;
	case ESEEFactionStanding::Friendly:   OutMinRep = 40;   OutMaxRep = 69;  break;
	case ESEEFactionStanding::Revered:    OutMinRep = 70;   OutMaxRep = 100; break;
	default:                              OutMinRep = 0;    OutMaxRep = 0;   break;
	}
}

// ============================================================================
// Reputation events
// ============================================================================

bool USEEFactionManager::IsPlayerInstigator(const AActor* Instigator)
{
	if (!Instigator) return true; // unattributed events are assumed player-driven

	if (const APawn* AsPawn = Cast<APawn>(Instigator))
	{
		return AsPawn->IsPlayerControlled();
	}
	return Instigator->GetInstigatorController() && Instigator->GetInstigatorController()->IsPlayerController();
}

void USEEFactionManager::ApplyReputationEvent(ESEEReputationEvent Event, ESEEFaction ContextFaction, AActor* Instigator)
{
	if (!IsPlayerInstigator(Instigator)) return;

	// Fixed-matrix events (kills, helping civilians).
	if (const FDeltaList* Deltas = GetFixedEventDeltas(Event))
	{
		for (const FFactionDelta& Entry : *Deltas)
		{
			ApplyDeltaInternal(Entry.Faction, Entry.Delta);
		}
		return;
	}

	// Faction-targeted events.
	if (ContextFaction == ESEEFaction::Neutral)
	{
		// Trades default to the Kronole Network (the Zone 1 stall); other
		// targeted events without a faction are a no-op.
		if (Event != ESEEReputationEvent::TradedWithMerchant) return;
		ContextFaction = ESEEFaction::KronoleNetwork;
	}

	switch (Event)
	{
	case ESEEReputationEvent::CompletedQuestFor:
	{
		ApplyDeltaInternal(ContextFaction, +10);
		FDeltaList Rivals;
		GetRivals(ContextFaction, Rivals, -3);
		for (const FFactionDelta& Rival : Rivals)
		{
			ApplyDeltaInternal(Rival.Faction, Rival.Delta);
		}
		break;
	}
	case ESEEReputationEvent::DialogueFlagGoodwill:
		ApplyDeltaInternal(ContextFaction, +3);
		break;

	case ESEEReputationEvent::TradedWithMerchant:
		ApplyDeltaInternal(ContextFaction, +2);
		if (ContextFaction != ESEEFaction::ThirdClassUnion)
		{
			ApplyDeltaInternal(ESEEFaction::ThirdClassUnion, +1); // trade keeps Third Class fed
		}
		break;

	default:
		break;
	}
}

ESEEFaction USEEFactionManager::ClassifyNPCFaction(const APawn* NPCPawn)
{
	if (!NPCPawn) return ESEEFaction::Neutral;

	// Mirror USEENPCBrainSubsystem::ConfigureController's identity heuristics:
	// blueprint class name plus the population-script actor label.
	const FString Identity = NPCPawn->GetActorNameOrLabel() + TEXT("|") + NPCPawn->GetClass()->GetName();

	if (Identity.Contains(TEXT("Jackboot")) || Identity.Contains(TEXT("Boss")))
	{
		return ESEEFaction::Jackboots;
	}
	if (Identity.Contains(TEXT("Kronole")) || Identity.Contains(TEXT("Dealer")) || Identity.Contains(TEXT("Smuggler")))
	{
		return ESEEFaction::KronoleNetwork;
	}
	if (Identity.Contains(TEXT("Merchant")) || Identity.Contains(TEXT("Breachman")) ||
		Identity.Contains(TEXT("Mechanic")) || Identity.Contains(TEXT("Workshop")))
	{
		return ESEEFaction::ThirdClassUnion;
	}
	if (Identity.Contains(TEXT("FirstClass")))
	{
		return ESEEFaction::FirstClassElite;
	}

	// Zone 1 default: unmarked civilians are Tailies.
	return ESEEFaction::Tailies;
}

void USEEFactionManager::NotifyNPCKilled(APawn* DeadNPC, AActor* Killer)
{
	if (!DeadNPC) return;

	const FString Identity = DeadNPC->GetActorNameOrLabel() + TEXT("|") + DeadNPC->GetClass()->GetName();
	const ESEEFaction Faction = ClassifyNPCFaction(DeadNPC);

	ESEEReputationEvent Event;
	if (Faction == ESEEFaction::Jackboots)
	{
		Event = ESEEReputationEvent::KilledJackboot;
	}
	else if (Identity.Contains(TEXT("Merchant")) || Identity.Contains(TEXT("Dealer")))
	{
		Event = ESEEReputationEvent::KilledMerchant;
	}
	else
	{
		Event = ESEEReputationEvent::KilledCivilian;
	}

	ApplyReputationEvent(Event, ESEEFaction::Neutral, Killer);
}

void USEEFactionManager::NotifyNPCKilled(const UObject* WorldContextObject, APawn* DeadNPC, AActor* Killer)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (USEEFactionManager* Manager = GI ? GI->GetSubsystem<USEEFactionManager>() : nullptr)
	{
		Manager->NotifyNPCKilled(DeadNPC, Killer);
	}
}

// ============================================================================
// Consequence queries
// ============================================================================

bool USEEFactionManager::ShouldNPCRefuseDialogue(ESEEFaction NPCFaction) const
{
	if (NPCFaction == ESEEFaction::Neutral) return false;

	// Below Unfriendly (i.e. Hostile or Hated) they want nothing to do with you.
	const ESEEFactionStanding Standing = GetStanding(NPCFaction);
	return Standing == ESEEFactionStanding::Hostile || Standing == ESEEFactionStanding::Hated;
}

bool USEEFactionManager::ShouldPawnRefuseDialogue(const APawn* NPCPawn)
{
	const UWorld* World = NPCPawn ? NPCPawn->GetWorld() : nullptr;
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const USEEFactionManager* Manager = GI ? GI->GetSubsystem<USEEFactionManager>() : nullptr;
	return Manager && Manager->ShouldNPCRefuseDialogue(ClassifyNPCFaction(NPCPawn));
}

float USEEFactionManager::GetMerchantPriceMultiplier(ESEEFaction MerchantFaction) const
{
	const ESEEFactionStanding Standing = GetStanding(MerchantFaction);
	switch (Standing)
	{
	case ESEEFactionStanding::Friendly:
	case ESEEFactionStanding::Revered:
		return 0.9f;
	case ESEEFactionStanding::Unfriendly:
	case ESEEFactionStanding::Hostile:
	case ESEEFactionStanding::Hated:
		return 1.25f;
	default:
		return 1.0f;
	}
}

bool USEEFactionManager::AreJackbootsKillOnSight() const
{
	return GetStanding(ESEEFaction::Jackboots) == ESEEFactionStanding::Hated;
}

// ============================================================================
// Legacy queries
// ============================================================================

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
	const int32 Rep = GetReputation(Faction);
	// -100 rep = 150% prices, 0 = 100%, +100 = 75%
	return 1.0f - (Rep / 400.0f);
}

// ============================================================================
// Persistence
// ============================================================================

FSEEFactionSaveState USEEFactionManager::GetSaveState() const
{
	FSEEFactionSaveState State;
	State.Reputations = FactionReputations;
	return State;
}

void USEEFactionManager::SetSaveState(const FSEEFactionSaveState& InState)
{
	if (InState.IsEmpty()) return; // pre-faction save: keep the Initialize() defaults

	for (const TPair<ESEEFaction, int32>& Pair : InState.Reputations)
	{
		SetReputation(Pair.Key, Pair.Value); // routes through broadcasts so UI refreshes
	}
}
