// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CompanionTypes.generated.h"

// ============================================================================
// Companion System Type Definitions
// Snowpiercer: Eternal Engine - Companion System
// ============================================================================

/** Companion AI behavior mode -- toggled by player in real-time */
UENUM(BlueprintType)
enum class ECompanionBehavior : uint8
{
	Aggressive		UMETA(DisplayName = "Aggressive"),
	Defensive		UMETA(DisplayName = "Defensive"),
	Passive			UMETA(DisplayName = "Passive"),
	HoldPosition	UMETA(DisplayName = "Hold Position")
};

/** Internal loyalty state derived from loyalty score */
UENUM(BlueprintType)
enum class ELoyaltyState : uint8
{
	Hostile		UMETA(DisplayName = "Hostile"),		// -100 to -60
	Resentful	UMETA(DisplayName = "Resentful"),	// -59 to -30
	Cold		UMETA(DisplayName = "Cold"),			// -29 to 0
	Neutral		UMETA(DisplayName = "Neutral"),		// 1 to 30
	Friendly	UMETA(DisplayName = "Friendly"),		// 31 to 60
	Devoted		UMETA(DisplayName = "Devoted"),		// 61 to 85
	Bonded		UMETA(DisplayName = "Bonded")		// 86 to 100
};

/** Companion combat role for AI decision-making */
UENUM(BlueprintType)
enum class ECompanionRole : uint8
{
	Medic		UMETA(DisplayName = "Medic"),
	MeleeDPS	UMETA(DisplayName = "Melee DPS"),
	Tank		UMETA(DisplayName = "Tank"),
	Stealth		UMETA(DisplayName = "Stealth"),
	Support		UMETA(DisplayName = "Support"),
	Utility		UMETA(DisplayName = "Utility"),
	Ranged		UMETA(DisplayName = "Ranged"),
	Social		UMETA(DisplayName = "Social"),
	Alchemist	UMETA(DisplayName = "Alchemist"),
	Scout		UMETA(DisplayName = "Scout"),
	Electronic	UMETA(DisplayName = "Electronic Warfare")
};

/** Corridor formation position */
UENUM(BlueprintType)
enum class EFormationPosition : uint8
{
	Front		UMETA(DisplayName = "Front"),
	Middle		UMETA(DisplayName = "Middle"),
	Rear		UMETA(DisplayName = "Rear")
};

/** Companion recruitment zone */
UENUM(BlueprintType)
enum class ERecruitmentZone : uint8
{
	Zone1_Tail			UMETA(DisplayName = "Zone 1: The Tail"),
	Zone2_ThirdClass	UMETA(DisplayName = "Zone 2: Third Class"),
	Zone3_SecondClass	UMETA(DisplayName = "Zone 3: Second Class"),
	Zone4_WorkingSpine	UMETA(DisplayName = "Zone 4: Working Spine"),
	Zone5_FirstClass	UMETA(DisplayName = "Zone 5: First Class"),
	Zone6_Engine		UMETA(DisplayName = "Zone 6: Engine & Exterior")
};

/** Events that affect loyalty */
UENUM(BlueprintType)
enum class ELoyaltyEventType : uint8
{
	// Positive
	PersonalQuestComplete	UMETA(DisplayName = "Personal Quest Complete"),
	AlignedChoice			UMETA(DisplayName = "Aligned Moral Choice"),
	LifeSaved				UMETA(DisplayName = "Life Saved"),
	ConflictSided			UMETA(DisplayName = "Sided in Conflict"),
	GiftGiven				UMETA(DisplayName = "Gift Given"),
	DialogueApproved		UMETA(DisplayName = "Dialogue Approved"),
	CombatProtected			UMETA(DisplayName = "Protected in Combat"),
	ResourceShared			UMETA(DisplayName = "Resource Shared"),
	PastAsked				UMETA(DisplayName = "Asked About Past"),

	// Negative
	ValuesViolated			UMETA(DisplayName = "Values Violated"),
	InnocentsDied			UMETA(DisplayName = "Innocents Died"),
	UnnecessaryKill			UMETA(DisplayName = "Unnecessary Kill"),
	ConflictOpposed			UMETA(DisplayName = "Opposed in Conflict"),
	KronoleUsed				UMETA(DisplayName = "Kronole Used"),
	DialogueDisapproved		UMETA(DisplayName = "Dialogue Disapproved"),
	CorpseLooted			UMETA(DisplayName = "Corpse Looted"),
	ViolentSolution			UMETA(DisplayName = "Violent Solution Chosen"),
	InjuryIgnored			UMETA(DisplayName = "Injury Ignored")
};

/** Companion permadeath cause */
UENUM(BlueprintType)
enum class EPermadeathCause : uint8
{
	None				UMETA(DisplayName = "Alive"),
	CombatDeath			UMETA(DisplayName = "Died in Combat"),
	StoryDeath			UMETA(DisplayName = "Story Choice Death"),
	BetrayalDeath		UMETA(DisplayName = "Killed After Betrayal"),
	SacrificeDeath		UMETA(DisplayName = "Sacrificed Self"),
	EnvironmentalDeath	UMETA(DisplayName = "Environmental Death"),
	Abandoned			UMETA(DisplayName = "Abandoned Party")
};

/** Trap type for Yuki's ability */
UENUM(BlueprintType)
enum class ETrapType : uint8
{
	Tripwire	UMETA(DisplayName = "Tripwire"),
	ShockPlate	UMETA(DisplayName = "Shock Plate"),
	Caltrops	UMETA(DisplayName = "Caltrops")
};

/** Chemical compound type for Min-jun's ability */
UENUM(BlueprintType)
enum class EChemicalCompound : uint8
{
	Stimulant		UMETA(DisplayName = "Stimulant"),
	Analgesic		UMETA(DisplayName = "Analgesic"),
	IrritantCloud	UMETA(DisplayName = "Irritant Cloud")
};

/** Train system type for ATLAS's ability */
UENUM(BlueprintType)
enum class ETrainSystem : uint8
{
	Doors			UMETA(DisplayName = "Door Locks"),
	Cameras			UMETA(DisplayName = "Surveillance Cameras"),
	Steam			UMETA(DisplayName = "Steam Vents"),
	Lighting		UMETA(DisplayName = "Lighting"),
	Alarms			UMETA(DisplayName = "Alarm Systems"),
	Sensors			UMETA(DisplayName = "Sensor Displays")
};

/** Loyalty event data */
USTRUCT(BlueprintType)
struct FLoyaltyEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELoyaltyEventType EventType = ELoyaltyEventType::DialogueApproved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoyaltyChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventDescription;
};

/** Companion stat block -- mirrors player stat system (1-20 scale) */
USTRUCT(BlueprintType)
struct FCompanionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Strength = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Endurance = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Cunning = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Perception = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Charisma = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1", ClampMax = "20"))
	int32 Survival = 5;
};

/** Personal quest step data */
USTRUCT(BlueprintType)
struct FCompanionQuestStep
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERecruitmentZone RequiredZone = ERecruitmentZone::Zone1_Tail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELoyaltyState MinLoyaltyRequired = ELoyaltyState::Neutral;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

/** Companion conflict data */
USTRUCT(BlueprintType)
struct FCompanionConflict
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompanionA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompanionB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ConflictDescription;

	/** CHA threshold for compromise option (0 = no compromise possible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompromiseCHAThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoyaltyPenaltyForLoser = -15;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompromisePenaltyReduction = 7;
};
