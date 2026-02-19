// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueTypes.generated.h"

// ============================================================================
// Dialogue System Type Definitions
// Snowpiercer: Eternal Engine - Dialogue System
// ============================================================================

/** The type of dialogue node in the hub-and-spoke system */
UENUM(BlueprintType)
enum class EDialogueNodeType : uint8
{
	Hub			UMETA(DisplayName = "Hub"),
	Spoke		UMETA(DisplayName = "Spoke"),
	Branch		UMETA(DisplayName = "Branch"),
	Bark		UMETA(DisplayName = "Bark"),
	Interrupt	UMETA(DisplayName = "Interrupt")
};

/** Stat used for dialogue skill checks */
UENUM(BlueprintType)
enum class EDialogueStat : uint8
{
	None		UMETA(DisplayName = "None"),
	Charisma	UMETA(DisplayName = "Charisma"),
	Cunning		UMETA(DisplayName = "Cunning"),
	Perception	UMETA(DisplayName = "Perception"),
	Strength	UMETA(DisplayName = "Strength"),
	Engineer	UMETA(DisplayName = "Engineer")
};

/** How a skill check is presented to the player */
UENUM(BlueprintType)
enum class ESkillCheckType : uint8
{
	Passive		UMETA(DisplayName = "Passive"),
	Active		UMETA(DisplayName = "Active"),
	Contested	UMETA(DisplayName = "Contested")
};

/** Color-coded difficulty hint for active checks */
UENUM(BlueprintType)
enum class ESkillCheckDifficulty : uint8
{
	Guaranteed	UMETA(DisplayName = "Guaranteed"),   // Stat exceeds by 3+
	Uncertain	UMETA(DisplayName = "Uncertain"),    // Stat within ±2
	Unlikely	UMETA(DisplayName = "Unlikely"),     // Stat below by 3+
	Impossible	UMETA(DisplayName = "Impossible")    // Stat below by 8+
};

/** Social approach type for persuasion/intimidation/deception */
UENUM(BlueprintType)
enum class ESocialApproach : uint8
{
	Persuasion		UMETA(DisplayName = "Persuasion"),
	Intimidation	UMETA(DisplayName = "Intimidation"),
	Deception		UMETA(DisplayName = "Deception")
};

/** Timer urgency for timed dialogue decisions */
UENUM(BlueprintType)
enum class EDialogueUrgency : uint8
{
	None		UMETA(DisplayName = "None"),
	Low			UMETA(DisplayName = "Low"),        // 15 seconds
	Medium		UMETA(DisplayName = "Medium"),     // 8 seconds
	High		UMETA(DisplayName = "High"),       // 4 seconds
	Critical	UMETA(DisplayName = "Critical")    // 2 seconds
};

/** NPC disposition bracket */
UENUM(BlueprintType)
enum class ENPCDisposition : uint8
{
	Hostile			UMETA(DisplayName = "Hostile"),        // -100 to -50
	Antagonistic	UMETA(DisplayName = "Antagonistic"),   // -49 to -20
	Neutral			UMETA(DisplayName = "Neutral"),        // -19 to +19
	Friendly		UMETA(DisplayName = "Friendly"),       // +20 to +49
	Loyal			UMETA(DisplayName = "Loyal")           // +50 to +100
};

/** Memory category for the NPC memory system */
UENUM(BlueprintType)
enum class EMemoryCategory : uint8
{
	Personal	UMETA(DisplayName = "Personal"),
	Factional	UMETA(DisplayName = "Factional"),
	Witnessed	UMETA(DisplayName = "Witnessed"),
	Rumor		UMETA(DisplayName = "Rumor"),
	Mood		UMETA(DisplayName = "Mood")
};

/** Interruption source type */
UENUM(BlueprintType)
enum class EDialogueInterruptSource : uint8
{
	Combat			UMETA(DisplayName = "Combat"),
	TrainEvent		UMETA(DisplayName = "Train Event"),
	ThirdParty		UMETA(DisplayName = "Third Party NPC"),
	TimerExpired	UMETA(DisplayName = "Timer Expired"),
	PlayerWalkAway	UMETA(DisplayName = "Player Walk Away")
};

/** NPC archetype for social combat resistance matrix */
UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
	Guard		UMETA(DisplayName = "Guard"),
	Trader		UMETA(DisplayName = "Trader"),
	Engineer	UMETA(DisplayName = "Engineer"),
	Politician	UMETA(DisplayName = "Politician"),
	CultMember	UMETA(DisplayName = "Cult Member"),
	Child		UMETA(DisplayName = "Child"),
	Boss		UMETA(DisplayName = "Boss")
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** A skill check attached to a dialogue option */
USTRUCT(BlueprintType)
struct FDialogueSkillCheck
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillCheckType CheckType = ESkillCheckType::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDialogueStat Stat = EDialogueStat::None;

	/** Threshold the player must meet or exceed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Threshold = 5;

	/** For contested checks: the NPC's opposing stat value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NPCContestValue = 0;

	/** Node to jump to on success */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SuccessNode = NAME_None;

	/** Node to jump to on failure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FailureNode = NAME_None;
};

/** A single dialogue option presented to the player */
USTRUCT(BlueprintType)
struct FDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	/** Node this option leads to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetNode = NAME_None;

	/** Optional skill check gate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDialogueSkillCheck SkillCheck;

	/** Whether this option has a skill check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSkillCheck = false;

	/** Condition tag — option only visible if this gameplay tag is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ConditionTag = NAME_None;

	/** If true, this option can only be selected once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOneShot = false;

	/** Social approach type (if applicable) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESocialApproach SocialApproach = ESocialApproach::Persuasion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSocialApproach = false;
};

/** A single node in the dialogue graph */
USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDialogueNodeType NodeType = EDialogueNodeType::Hub;

	/** Speaker name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Speaker = NAME_None;

	/** The NPC's line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	/** Available responses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueOption> Options;

	/** Timer urgency (None = no timer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDialogueUrgency Urgency = EDialogueUrgency::None;

	/** Which option triggers on timer expiry (index into Options, -1 = worst outcome) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimeoutDefaultOption = -1;

	/** Event to fire when this node is entered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OnEnterEvent = NAME_None;

	/** Event to fire when this node is exited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OnExitEvent = NAME_None;
};

/** A memory entry stored by an NPC */
USTRUCT(BlueprintType)
struct FNPCMemory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMemoryCategory Category = EMemoryCategory::Personal;

	/** Tag identifying what was remembered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MemoryTag = NAME_None;

	/** Disposition change when this memory was formed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DispositionDelta = 0;

	/** Game time when the memory was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.f;

	/** How many cars this rumor has traveled (for Rumor category) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RumorDistance = 0;

	/** Decay rate for Mood memories (disposition delta per minute) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoodDecayRate = 0.f;
};

/** Social combat resistance modifiers per NPC archetype */
USTRUCT(BlueprintType)
struct FSocialResistance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersuasionModifier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 IntimidationModifier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DeceptionModifier = 0;
};

/** Rumor propagation data */
USTRUCT(BlueprintType)
struct FRumorData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RumorTag = NAME_None;

	/** Car index where the rumor originated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OriginCar = 0;

	/** Current furthest car reached */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentReach = 0;

	/** Game time when the rumor was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CreationTime = 0.f;

	/** Propagation speed: cars per minute of game time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PropagationRate = 0.067f; // ~1 car per 15 min
};
