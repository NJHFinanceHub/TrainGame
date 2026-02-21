// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "TrainGameAITypes.generated.h"

// ============================================================================
// AI System Type Definitions
// Snowpiercer: Eternal Engine — NPC AI, Schedules, Spawning
// ============================================================================

/** High-level NPC AI state for behavior tree */
UENUM(BlueprintType)
enum class ENPCAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrolling	UMETA(DisplayName = "Patrolling"),
	Investigating	UMETA(DisplayName = "Investigating"),
	Chasing		UMETA(DisplayName = "Chasing"),
	Combat		UMETA(DisplayName = "Combat"),
	Fleeing		UMETA(DisplayName = "Fleeing"),
	Scheduled	UMETA(DisplayName = "Following Schedule"),
	Dead		UMETA(DisplayName = "Dead")
};

/** NPC class/caste on the train */
UENUM(BlueprintType)
enum class ENPCClass : uint8
{
	Tailie		UMETA(DisplayName = "Tailie"),
	ThirdClass	UMETA(DisplayName = "Third Class"),
	SecondClass	UMETA(DisplayName = "Second Class"),
	FirstClass	UMETA(DisplayName = "First Class"),
	Jackboot	UMETA(DisplayName = "Jackboot"),
	Engineer	UMETA(DisplayName = "Engineer"),
	Medical		UMETA(DisplayName = "Medical"),
	Conductor	UMETA(DisplayName = "Conductor"),
	Crowd		UMETA(DisplayName = "Crowd (Ambient)")
};

/** Schedule activity type for time-of-day routines */
UENUM(BlueprintType)
enum class EScheduleActivity : uint8
{
	Sleep		UMETA(DisplayName = "Sleep"),
	Eat			UMETA(DisplayName = "Eat"),
	Work		UMETA(DisplayName = "Work"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Socialize	UMETA(DisplayName = "Socialize"),
	Pray		UMETA(DisplayName = "Pray"),
	Recreation	UMETA(DisplayName = "Recreation"),
	Custom		UMETA(DisplayName = "Custom")
};

/** Alert level for Jackboot coordination */
UENUM(BlueprintType)
enum class EAlertLevel : uint8
{
	Green	UMETA(DisplayName = "Green — Normal"),
	Yellow	UMETA(DisplayName = "Yellow — Suspicious Activity"),
	Orange	UMETA(DisplayName = "Orange — Active Search"),
	Red		UMETA(DisplayName = "Red — Combat/Lockdown")
};

/** Crowd NPC behavior (lightweight, no combat) */
UENUM(BlueprintType)
enum class ECrowdBehavior : uint8
{
	Walk		UMETA(DisplayName = "Walk"),
	Idle		UMETA(DisplayName = "Idle/Stand"),
	Talk		UMETA(DisplayName = "Talk to Other"),
	SitDown		UMETA(DisplayName = "Sit Down"),
	Flee		UMETA(DisplayName = "Flee (react to danger)")
};

/** Body state for discovery system */
UENUM(BlueprintType)
enum class EDiscoveredBodyReaction : uint8
{
	RaiseAlarm	UMETA(DisplayName = "Raise Alarm"),
	Investigate	UMETA(DisplayName = "Investigate Quietly"),
	Flee		UMETA(DisplayName = "Flee in Terror"),
	Ignore		UMETA(DisplayName = "Ignore (ally of player)")
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** Single time slot in an NPC's daily schedule */
USTRUCT(BlueprintType)
struct FScheduleEntry
{
	GENERATED_BODY()

	/** Hour this activity starts (0-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	int32 StartHour = 0;

	/** Hour this activity ends (0-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	int32 EndHour = 8;

	/** What the NPC does during this time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	EScheduleActivity Activity = EScheduleActivity::Sleep;

	/** Location tag where the NPC should be */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FName LocationTag = NAME_None;

	/** Optional animation or montage to play at this location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FName AnimationTag = NAME_None;
};

/** DataTable row for NPC spawn definitions per car */
USTRUCT(BlueprintType)
struct FNPCSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Car identifier (matches car actor tag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FName CarTag = NAME_None;

	/** NPC class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<APawn> NPCClass;

	/** NPC caste/role */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	ENPCClass NPCRole = ENPCClass::Crowd;

	/** Number to spawn in this car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	int32 SpawnCount = 1;

	/** Spawn at specific tagged locations (empty = random nav mesh) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FName> SpawnPointTags;

	/** Schedule template for spawned NPCs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FScheduleEntry> Schedule;

	/** Patrol waypoint tags (for Jackboots) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FName> PatrolWaypointTags;

	/** Which zone this car belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	EStealthZone Zone = EStealthZone::ThirdClass;
};

/** Blackboard key names — centralized to avoid typos */
namespace BBKeys
{
	const FName AIState		= TEXT("AIState");
	const FName TargetActor = TEXT("TargetActor");
	const FName TargetLocation = TEXT("TargetLocation");
	const FName PatrolIndex = TEXT("PatrolIndex");
	const FName AlertLevel	= TEXT("AlertLevel");
	const FName HomeZone	= TEXT("HomeZone");
	const FName ScheduleActivity = TEXT("ScheduleActivity");
	const FName ScheduleLocation = TEXT("ScheduleLocation");
	const FName CanSeeTarget = TEXT("CanSeeTarget");
	const FName DistToTarget = TEXT("DistToTarget");
	const FName HasBodyInSight = TEXT("HasBodyInSight");
	const FName IsCompanion = TEXT("IsCompanion");
	const FName BehaviorMode = TEXT("BehaviorMode");
}
