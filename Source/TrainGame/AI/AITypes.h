// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "AITypes.generated.h"

// ============================================================================
// AI System Type Definitions
// Snowpiercer: Eternal Engine - NPC AI, Schedules, Spawning
// ============================================================================

/** High-level NPC AI state for behavior tree blackboard */
UENUM(BlueprintType)
enum class ENPCAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Investigate	UMETA(DisplayName = "Investigate"),
	Chase		UMETA(DisplayName = "Chase"),
	Combat		UMETA(DisplayName = "Combat"),
	Flee		UMETA(DisplayName = "Flee"),
	Schedule	UMETA(DisplayName = "Following Schedule")
};

/** NPC archetype determines AI behavior profile and capabilities */
UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
	Civilian	UMETA(DisplayName = "Civilian"),
	Jackboot	UMETA(DisplayName = "Jackboot"),
	Engineer	UMETA(DisplayName = "Engineer"),
	Medic		UMETA(DisplayName = "Medic"),
	Worker		UMETA(DisplayName = "Worker"),
	Crowd		UMETA(DisplayName = "Crowd (Ambient)")
};

/** Jackboot rank affects AI aggressiveness and authority */
UENUM(BlueprintType)
enum class EJackbootRank : uint8
{
	Grunt		UMETA(DisplayName = "Grunt"),
	Corporal	UMETA(DisplayName = "Corporal"),
	Sergeant	UMETA(DisplayName = "Sergeant"),
	Lieutenant	UMETA(DisplayName = "Lieutenant"),
	Commander	UMETA(DisplayName = "Commander")
};

/** Schedule activity type */
UENUM(BlueprintType)
enum class EScheduleActivity : uint8
{
	Sleep		UMETA(DisplayName = "Sleep"),
	Work		UMETA(DisplayName = "Work"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Eat			UMETA(DisplayName = "Eat"),
	Socialize	UMETA(DisplayName = "Socialize"),
	Pray		UMETA(DisplayName = "Pray"),
	Custom		UMETA(DisplayName = "Custom")
};

/** Crowd NPC ambient behavior */
UENUM(BlueprintType)
enum class ECrowdBehavior : uint8
{
	Idle		UMETA(DisplayName = "Idle/Standing"),
	Walk		UMETA(DisplayName = "Walking"),
	Talk		UMETA(DisplayName = "Talking (Group)"),
	SitDown		UMETA(DisplayName = "Sitting"),
	Working		UMETA(DisplayName = "Working (Ambient)")
};

/** Alert level for Jackboot coordination */
UENUM(BlueprintType)
enum class EAlertLevel : uint8
{
	Green		UMETA(DisplayName = "Green (Normal)"),
	Yellow		UMETA(DisplayName = "Yellow (Suspicious)"),
	Orange		UMETA(DisplayName = "Orange (Active Search)"),
	Red			UMETA(DisplayName = "Red (Combat/Lockdown)")
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** A single time-block in an NPC's daily schedule */
USTRUCT(BlueprintType)
struct FScheduleEntry
{
	GENERATED_BODY()

	/** Hour this activity starts (0-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartHour = 0;

	/** Hour this activity ends (0-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "23"))
	int32 EndHour = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EScheduleActivity Activity = EScheduleActivity::Sleep;

	/** Location tag to navigate to (references a tagged actor in the level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationTag;

	/** Optional patrol route tag (if Activity == Patrol) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PatrolRouteTag;
};

/** NPC spawn entry for data-driven per-car spawning */
USTRUCT(BlueprintType)
struct FNPCSpawnEntry : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ENPCArchetype Archetype = ENPCArchetype::Civilian;

	/** Blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<APawn> NPCClass;

	/** Spawn location tag within the car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SpawnLocationTag;

	/** Number to spawn (for crowd NPCs, can be > 1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1"))
	int32 Count = 1;

	/** Schedule entries for this NPC (empty = no schedule) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FScheduleEntry> Schedule;

	/** Patrol route tag (if patrolling) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PatrolRouteTag;

	/** Jackboot rank (only relevant for Jackboot archetype) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EJackbootRank JackbootRank = EJackbootRank::Grunt;

	/** Train zone this NPC belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrainZone HomeZone = ETrainZone::ThirdClass;

	/** Whether this NPC has a radio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasRadio = false;
};

/** Body discovery event data */
USTRUCT(BlueprintType)
struct FBodyDiscoveryEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* DiscoveredBody = nullptr;

	UPROPERTY(BlueprintReadOnly)
	AActor* Discoverer = nullptr;

	UPROPERTY(BlueprintReadOnly)
	EBodyState BodyState = EBodyState::Unconscious;

	UPROPERTY(BlueprintReadOnly)
	FVector DiscoveryLocation = FVector::ZeroVector;
};
