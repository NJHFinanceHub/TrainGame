// FactionTypes.h - Core type definitions for the faction reputation system
#pragma once

#include "CoreMinimal.h"
#include "FactionTypes.generated.h"

// All factions on Snowpiercer. Canonical enum for game-wide use.
// Note: ESEEFaction in SEETypes.h is a parallel definition used by the
// SnowpiercerEE module for car data. These should be kept in sync.
UENUM(BlueprintType)
enum class EFaction : uint8
{
	None			UMETA(DisplayName = "None"),
	Tailies			UMETA(DisplayName = "Tailies"),
	ThirdClassUnion	UMETA(DisplayName = "Third Class Union"),
	Jackboots		UMETA(DisplayName = "Jackboots"),
	Bureaucracy		UMETA(DisplayName = "Bureaucracy"),
	FirstClassElite	UMETA(DisplayName = "First Class Elite"),
	OrderOfTheEngine UMETA(DisplayName = "Order of the Engine"),
	KronoleNetwork	UMETA(DisplayName = "Kronole Network"),
	TheThaw			UMETA(DisplayName = "The Thaw")
};

// Reputation standing tiers. Thresholds defined per-faction in FFactionConfig.
UENUM(BlueprintType)
enum class EReputationLevel : uint8
{
	Distrusted	UMETA(DisplayName = "Distrusted"),
	Unknown		UMETA(DisplayName = "Unknown"),
	Known		UMETA(DisplayName = "Known"),
	Respected	UMETA(DisplayName = "Respected"),
	Champion	UMETA(DisplayName = "Champion"),
	Legend		UMETA(DisplayName = "Legend")
};

// Faction disposition toward the player
UENUM(BlueprintType)
enum class EFactionDisposition : uint8
{
	Hostile		UMETA(DisplayName = "Hostile"),
	Unfriendly	UMETA(DisplayName = "Unfriendly"),
	Neutral		UMETA(DisplayName = "Neutral"),
	Friendly	UMETA(DisplayName = "Friendly"),
	Allied		UMETA(DisplayName = "Allied")
};

// Delegate for reputation value changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnReputationChanged,
	EFaction, Faction,
	float, OldValue,
	float, NewValue
);

// Delegate for reputation level threshold crossings
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnReputationLevelChanged,
	EFaction, Faction,
	EReputationLevel, OldLevel,
	EReputationLevel, NewLevel
);

// Delegate for faction perk unlock/lock
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFactionPerkChanged,
	EFaction, Faction,
	FName, PerkTag
);

// Threshold definition: maps a reputation level to a minimum rep value
USTRUCT(BlueprintType)
struct FReputationThreshold
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReputationLevel Level = EReputationLevel::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinReputation = 0.0f;
};

// Faction perk: unlocked at a reputation threshold
USTRUCT(BlueprintType)
struct FFactionPerk
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PerkTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EReputationLevel RequiredLevel = EReputationLevel::Known;

	// Gameplay effect magnitude (interpretation varies by perk)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectMagnitude = 0.0f;
};

// Per-faction configuration: thresholds, perks, starting rep, reputation currency name
USTRUCT(BlueprintType)
struct FFactionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFaction Faction = EFaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FactionName;

	// Name of this faction's reputation currency (e.g., "Solidarity" for Tailies)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReputationCurrency = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartingReputation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinReputation = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxReputation = 100.0f;

	// Sorted ascending by MinReputation
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FReputationThreshold> Thresholds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FFactionPerk> Perks;
};

// Snapshot of all faction reputations for save/load
USTRUCT(BlueprintType)
struct FFactionReputationSnapshot
{
	GENERATED_BODY()

	// Maps faction to current reputation value
	UPROPERTY(BlueprintReadWrite)
	TMap<EFaction, float> ReputationValues;

	// Active perks by tag
	UPROPERTY(BlueprintReadWrite)
	TArray<FName> ActivePerks;
};
