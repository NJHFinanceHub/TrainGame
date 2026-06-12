// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
//
// SEEFactionTypes.h - Shared faction-standing types: the Fallout-style standing
// tier ladder, reputation event ids and the faction save-state blob.
// Pure data/helpers; the logic lives in USEEFactionManager.

#pragma once

#include "CoreMinimal.h"
#include "SEETypes.h"
#include "SEEFactionTypes.generated.h"

class APawn;

/**
 * Standing tiers over the -100..+100 reputation axis (Fallout-style):
 *
 *   Hated      -100 .. -60   kill-on-sight territory
 *   Hostile     -59 .. -25   open contempt
 *   Unfriendly  -24 ..  -5   doors close, prices climb
 *   Neutral      -4 .. +14   nobody cares who you are
 *   Accepted    +15 .. +39   a known, tolerated face
 *   Friendly    +40 .. +69   favors, discounts, open doors
 *   Revered     +70 .. +100  they tell stories about you
 */
UENUM(BlueprintType)
enum class ESEEFactionStanding : uint8
{
	Hated		UMETA(DisplayName = "Hated"),
	Hostile		UMETA(DisplayName = "Hostile"),
	Unfriendly	UMETA(DisplayName = "Unfriendly"),
	Neutral		UMETA(DisplayName = "Neutral"),
	Accepted	UMETA(DisplayName = "Accepted"),
	Friendly	UMETA(DisplayName = "Friendly"),
	Revered		UMETA(DisplayName = "Revered"),

	// --- Deprecated tiers kept only so legacy UI code (SEEFactionWidget) still
	// compiles. Never returned by USEEFactionManager::GetStanding(). ---
	Allied		UMETA(Hidden, DisplayName = "Allied (deprecated)"),
	Devoted		UMETA(Hidden, DisplayName = "Devoted (deprecated)")
};

/**
 * High-level reputation events. Each maps to a data-driven table of
 * cross-faction deltas inside USEEFactionManager::ApplyReputationEvent —
 * killing a Jackboot doesn't just anger the Jackboots, it ripples.
 */
UENUM(BlueprintType)
enum class ESEEReputationEvent : uint8
{
	KilledJackboot		UMETA(DisplayName = "Killed Jackboot"),
	KilledCivilian		UMETA(DisplayName = "Killed Civilian"),
	KilledMerchant		UMETA(DisplayName = "Killed Merchant"),
	HelpedCivilian		UMETA(DisplayName = "Helped Civilian"),
	CompletedQuestFor	UMETA(DisplayName = "Completed Quest For Faction"),
	DialogueFlagGoodwill UMETA(DisplayName = "Dialogue Goodwill"),
	TradedWithMerchant	UMETA(DisplayName = "Traded With Merchant")
};

/**
 * Serializable snapshot of every faction reputation. Mirrors how the save
 * subsystem stores other blobs (UPROPERTY(SaveGame) on a plain struct) so
 * USEESaveGameData can embed it later without touching the manager again.
 */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEFactionSaveState
{
	GENERATED_BODY()

	/** Raw reputation per faction (-100..100). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Faction")
	TMap<ESEEFaction, int32> Reputations;

	bool IsEmpty() const { return Reputations.Num() == 0; }
};
