// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "AITypes.generated.h"

// ============================================================================
// AI System Type Definitions
// Snowpiercer: Eternal Engine — Behavior Trees, Schedules, Spawning
// ============================================================================

// --- NPC AI State (Blackboard key) ---

/** High-level NPC AI state driven by behavior tree */
UENUM(BlueprintType)
enum class ENPCAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Scheduled	UMETA(DisplayName = "Scheduled Activity"),
	Investigate	UMETA(DisplayName = "Investigate"),
	Chase		UMETA(DisplayName = "Chase"),
	Combat		UMETA(DisplayName = "Combat"),
	Flee		UMETA(DisplayName = "Flee"),
	Dead		UMETA(DisplayName = "Dead")
};

// --- NPC Role ---

/** NPC social class / role on the train */
UENUM(BlueprintType)
enum class ENPCRole : uint8
{
	Tailie		UMETA(DisplayName = "Tailie"),
	ThirdClass	UMETA(DisplayName = "Third Class"),
	SecondClass	UMETA(DisplayName = "Second Class"),
	FirstClass	UMETA(DisplayName = "First Class"),
	Jackboot	UMETA(DisplayName = "Jackboot"),
	OrderMember	UMETA(DisplayName = "Order Member"),
	Breachman	UMETA(DisplayName = "Breachman"),
	Conductor	UMETA(DisplayName = "Conductor"),
	Engineer	UMETA(DisplayName = "Engineer"),
	Crowd		UMETA(DisplayName = "Crowd NPC")
};

// --- Schedule System ---

/** Activity types for NPC schedules */
UENUM(BlueprintType)
enum class ENPCActivity : uint8
{
	Sleep		UMETA(DisplayName = "Sleep"),
	Work		UMETA(DisplayName = "Work"),
	Patrol		UMETA(DisplayName = "Patrol"),
	Eat			UMETA(DisplayName = "Eat"),
	Socialize	UMETA(DisplayName = "Socialize"),
	Pray		UMETA(DisplayName = "Pray"),
	Exercise	UMETA(DisplayName = "Exercise"),
	Guard		UMETA(DisplayName = "Guard"),
	Idle		UMETA(DisplayName = "Idle")
};

/** A single schedule entry: what to do at what time */
USTRUCT(BlueprintType)
struct FNPCScheduleEntry
{
	GENERATED_BODY()

	/** Hour of day this activity starts (0-23) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	int32 StartHour = 0;

	/** Activity to perform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	ENPCActivity Activity = ENPCActivity::Idle;

	/** Location tag where this activity takes place */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	FName LocationTag = NAME_None;

	/** Optional: specific world location (if no tag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	FVector Location = FVector::ZeroVector;
};

/** A full daily schedule for an NPC */
USTRUCT(BlueprintType)
struct FNPCSchedule
{
	GENERATED_BODY()

	/** Ordered list of schedule entries (sorted by StartHour) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	TArray<FNPCScheduleEntry> Entries;
};

// --- Spawner System ---

/** Data table row for NPC spawning configuration */
USTRUCT(BlueprintType)
struct FNPCSpawnEntry : public FTableRowBase
{
	GENERATED_BODY()

	/** NPC blueprint class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	TSoftClassPtr<APawn> NPCClass;

	/** NPC role/social class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	ENPCRole Role = ENPCRole::ThirdClass;

	/** Number of this NPC type to spawn in this car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	int32 Count = 1;

	/** Schedule template for spawned NPCs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	FNPCSchedule Schedule;

	/** Whether this NPC is a crowd (ambient) NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	bool bIsCrowd = false;

	/** Spawn weight for random selection (higher = more likely) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	float SpawnWeight = 1.0f;
};

// --- Jackboot Formation ---

/** Formation type for Jackboot squads */
UENUM(BlueprintType)
enum class EJackbootFormation : uint8
{
	/** Standard patrol: single file in corridor */
	SingleFile		UMETA(DisplayName = "Single File"),

	/** Two abreast in wider corridors */
	Paired			UMETA(DisplayName = "Paired"),

	/** Defensive formation: shield wall */
	ShieldWall		UMETA(DisplayName = "Shield Wall"),

	/** Aggressive push: staggered advance */
	StaggeredPush	UMETA(DisplayName = "Staggered Push")
};

/** Jackboot alert level (escalation system) */
UENUM(BlueprintType)
enum class EJackbootAlertLevel : uint8
{
	/** Normal patrol */
	Green		UMETA(DisplayName = "Green — Normal"),

	/** Heightened awareness, extra patrols */
	Yellow		UMETA(DisplayName = "Yellow — Heightened"),

	/** Active threat, backup called */
	Orange		UMETA(DisplayName = "Orange — Active Threat"),

	/** Full lockdown, all units converge */
	Red			UMETA(DisplayName = "Red — Lockdown")
};

// --- Body Discovery ---

/** Body discovery report */
USTRUCT(BlueprintType)
struct FBodyDiscoveryReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	AActor* Body = nullptr;

	UPROPERTY(BlueprintReadOnly)
	EBodyState BodyState = EBodyState::Unconscious;

	UPROPERTY(BlueprintReadOnly)
	FVector DiscoveryLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	AActor* Discoverer = nullptr;

	UPROPERTY(BlueprintReadOnly)
	float GameTimeDiscovered = 0.f;
};

// --- Blackboard Key Names ---

namespace SEEBlackboardKeys
{
	const FName AIState = TEXT("AIState");
	const FName TargetActor = TEXT("TargetActor");
	const FName TargetLocation = TEXT("TargetLocation");
	const FName PatrolIndex = TEXT("PatrolIndex");
	const FName HomeLocation = TEXT("HomeLocation");
	const FName CurrentActivity = TEXT("CurrentActivity");
	const FName AlertLevel = TEXT("AlertLevel");
	const FName HasRadio = TEXT("HasRadio");
	const FName DetectionState = TEXT("DetectionState");
	const FName CanFight = TEXT("CanFight");
}
