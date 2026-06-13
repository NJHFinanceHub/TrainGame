#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEETypes.h"
#include "Factions/SEEFactionTypes.h"
#include "SEEFactionManager.generated.h"

class APawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactionRepChanged, ESEEFaction, Faction, int32, NewRep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFactionStandingChanged, ESEEFaction, Faction, ESEEFactionStanding, OldStanding, ESEEFactionStanding, NewStanding);

/**
 * USEEFactionManager
 *
 * Fallout-style faction standing ledger. Raw reputation is a clamped
 * -100..+100 integer per faction; GetStanding() buckets it into the seven
 * ESEEFactionStanding tiers (see Factions/SEEFactionTypes.h for the ranges).
 *
 * Reputation changes flow through two doors:
 *  - ModifyReputation/SetReputation: direct, single-faction (plus the legacy
 *    mutual-exclusivity nudge for the big rivalries).
 *  - ApplyReputationEvent: the data-driven cross-faction ripple table.
 *    Killing a Jackboot pleases the Tail and angers the front; murdering a
 *    civilian stains you with everyone who has a conscience.
 *
 * Consequence queries (polled by AI / economy / dialogue):
 *  - ShouldNPCRefuseDialogue: civilians stop talking below Unfriendly.
 *  - GetMerchantPriceMultiplier: Friendly+ = 0.9x, Unfriendly- = 1.25x.
 *  - AreJackbootsKillOnSight: Hated with the Jackboots = patrols ignore the
 *    vision cone and aggro on sight.
 *
 * Defaults: the player is Tail-born — Tailies start +20, Jackboots -10.
 */
UCLASS()
class SNOWPIERCEREE_API USEEFactionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Raw reputation ---

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void ModifyReputation(ESEEFaction Faction, int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetReputation(ESEEFaction Faction, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Faction")
	int32 GetReputation(ESEEFaction Faction) const;

	// --- Standing tiers ---

	UFUNCTION(BlueprintPure, Category = "Faction")
	ESEEFactionStanding GetStanding(ESEEFaction Faction) const;

	/** Tier display name ("Hated" ... "Revered"). */
	UFUNCTION(BlueprintPure, Category = "Faction")
	static FText GetStandingDisplayName(ESEEFactionStanding Standing);

	/** Tier color (Hated deep red -> Neutral grey -> Revered gold). */
	UFUNCTION(BlueprintPure, Category = "Faction")
	static FLinearColor GetStandingColor(ESEEFactionStanding Standing);

	/** Inclusive reputation range covered by a tier (e.g. Neutral = -4..+14). */
	UFUNCTION(BlueprintPure, Category = "Faction")
	static void GetStandingRange(ESEEFactionStanding Standing, int32& OutMinRep, int32& OutMaxRep);

	/** Bucket a raw -100..+100 value into its tier. */
	UFUNCTION(BlueprintPure, Category = "Faction")
	static ESEEFactionStanding StandingForReputation(int32 Reputation);

	// --- Reputation events (cross-faction ripple) ---

	/**
	 * Apply a high-level event through the faction delta table.
	 * ContextFaction is only read by faction-targeted events
	 * (CompletedQuestFor, DialogueFlagGoodwill, TradedWithMerchant).
	 * Instigator, when supplied, must be the player (or player-controlled) —
	 * reputation tracks the player's deeds, not NPC-on-NPC violence.
	 */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	void ApplyReputationEvent(ESEEReputationEvent Event, ESEEFaction ContextFaction = ESEEFaction::Neutral, AActor* Instigator = nullptr);

	/** Classify a dead NPC pawn (jackboot/merchant/civilian) and apply the matching kill event. */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	void NotifyNPCKilled(APawn* DeadNPC, AActor* Killer = nullptr);

	/** Static one-liner for AI code: routes a dead pawn into NotifyNPCKilled. Safe on null/world-less pawns. */
	static void NotifyNPCKilled(const UObject* WorldContextObject, APawn* DeadNPC, AActor* Killer = nullptr);

	/** Heuristic faction for an NPC pawn (mirrors USEENPCBrainSubsystem's class/label classification). */
	static ESEEFaction ClassifyNPCFaction(const APawn* NPCPawn);

	// --- Consequence queries ---

	/** True when NPCs of this faction refuse to open dialogue (standing below Unfriendly). */
	UFUNCTION(BlueprintPure, Category = "Faction")
	bool ShouldNPCRefuseDialogue(ESEEFaction NPCFaction) const;

	/** Static one-liner for AI code: does this pawn's faction currently refuse dialogue? */
	static bool ShouldPawnRefuseDialogue(const APawn* NPCPawn);

	/** Barter multiplier vs. the merchant's faction: Friendly+ 0.9, Unfriendly or worse 1.25, else 1.0. */
	UFUNCTION(BlueprintPure, Category = "Faction")
	float GetMerchantPriceMultiplier(ESEEFaction MerchantFaction = ESEEFaction::KronoleNetwork) const;

	/** Hated by the Jackboots: patrols aggro on sight, vision cone be damned. */
	UFUNCTION(BlueprintPure, Category = "Faction")
	bool AreJackbootsKillOnSight() const;

	// --- Legacy queries (kept for existing callers) ---

	UFUNCTION(BlueprintPure, Category = "Faction")
	bool IsHostile(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	bool IsFriendly(ESEEFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "Faction")
	float GetPriceModifier(ESEEFaction Faction) const;

	// --- Persistence (SaveGame-tagged snapshot; see SEESaveGameSubsystem) ---

	UFUNCTION(BlueprintPure, Category = "Faction|Save")
	FSEEFactionSaveState GetSaveState() const;

	UFUNCTION(BlueprintCallable, Category = "Faction|Save")
	void SetSaveState(const FSEEFactionSaveState& InState);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Faction")
	FOnFactionRepChanged OnFactionRepChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction")
	FOnFactionStandingChanged OnFactionStandingChanged;

private:
	TMap<ESEEFaction, int32> FactionReputations;

	/**
	 * CO-OP: faction reputation is GLOBAL session state — the player party's
	 * standing with each faction, shared by everyone in the session. It is
	 * HOST-AUTHORITATIVE: only the host (listen server) mutates the ledger.
	 *
	 * USEEFactionManager is a UGameInstanceSubsystem, not a replicated actor, so it
	 * can't replicate directly and has no client->server RPC channel. Rather than
	 * add a GameState/PlayerState carrier (those files are out of scope this pass),
	 * mutations are gated to authority: on a client ModifyReputation/SetReputation/
	 * ApplyReputationEvent become no-ops, so clients never desync the ledger with
	 * a local-only change. The reputation snapshot itself reaches clients through
	 * the host-authoritative save (SetSaveState on load) — see the deferred note
	 * for live in-session replication of mid-game rep changes to clients.
	 *
	 * Standalone is authority, so every mutation runs exactly as before
	 * (single-player faction behavior is byte-for-byte unchanged).
	 *
	 * True on the host (listen server / dedicated) and in standalone; false on a
	 * connected client. SetSaveState bypasses this so a host-authored save still
	 * restores onto a client's local manager.
	 */
	bool HasReputationAuthority() const;

	/** Clamp + store + broadcast, with no rivalry side effects (the event table owns ripples). */
	void ApplyDeltaInternal(ESEEFaction Faction, int32 Delta);
	void ApplyMutualExclusivity(ESEEFaction Faction, int32 Delta);
	ESEEFactionStanding RepToStanding(int32 Rep) const;

	/** True when the actor may earn/lose reputation (null or player-controlled). */
	static bool IsPlayerInstigator(const AActor* Instigator);
};
