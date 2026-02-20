// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "StealthAIController.generated.h"

class UDetectionComponent;

// ============================================================================
// AStealthAIController
//
// AI controller for NPCs during stealth gameplay. Drives patrol routes,
// investigation behavior, search patterns, and challenge initiation.
// Hands off to CombatAIController when detection reaches COMBAT state.
// ============================================================================

/** Patrol behavior when not detecting anything */
UENUM(BlueprintType)
enum class EPatrolBehavior : uint8
{
	/** Walk a fixed route, looping */
	RoutePatrol		UMETA(DisplayName = "Route Patrol"),

	/** Stand in one spot, periodically look around */
	Stationary		UMETA(DisplayName = "Stationary"),

	/** Wander randomly within a defined area */
	Wander			UMETA(DisplayName = "Wander"),

	/** Follow a schedule (move between locations at set times) */
	Scheduled		UMETA(DisplayName = "Scheduled")
};

UCLASS()
class TRAINGAME_API AStealthAIController : public AAIController
{
	GENERATED_BODY()

public:
	AStealthAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// --- State Responses ---

	/** Called when detection state changes. Adjusts AI behavior accordingly. */
	UFUNCTION()
	void OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState);

protected:
	// --- Patrol ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Patrol")
	EPatrolBehavior PatrolBehavior = EPatrolBehavior::RoutePatrol;

	/** Patrol route waypoints (used with RoutePatrol) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Patrol")
	TArray<FVector> PatrolWaypoints;

	/** Time to pause at each waypoint before moving to the next */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Patrol")
	float WaypointPauseDuration = 3.f;

	/** Patrol walk speed (slower than alert/combat speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Patrol")
	float PatrolSpeed = 200.f;

	/** Alert walk speed (searching) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Patrol")
	float AlertSpeed = 350.f;

	// --- Investigation ---

	/** Radius to search around a noise source or last known position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Investigation")
	float InvestigationRadius = 500.f;

	/** Number of points to check during an investigation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Investigation")
	int32 InvestigationCheckpoints = 3;

	// --- Challenge ---

	/** Whether this NPC can initiate disguise challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Challenge")
	bool bCanChallenge = true;

	/** The zone this NPC considers home (better at detecting wrong disguises) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StealthAI|Challenge")
	ETrainZone HomeZone = ETrainZone::ThirdClass;

private:
	void TickPatrol(float DeltaTime);
	void TickSuspicious(float DeltaTime);
	void TickAlerted(float DeltaTime);
	void TickCombat(float DeltaTime);

	void MoveToInvestigationPoint();
	void SearchNearbyHidingSpots();
	void ReturnToPatrol();
	void InitiateChallenge(AActor* SuspectedPlayer);
	void HandOffToCombatAI();

	/** Find hiding spots within investigation radius */
	TArray<AActor*> FindNearbyHidingSpots() const;

	UPROPERTY()
	UDetectionComponent* DetectionComp = nullptr;

	int32 CurrentWaypointIndex = 0;
	float WaypointPauseTimer = 0.f;
	bool bIsInvestigating = false;
	FVector InvestigationTarget = FVector::ZeroVector;
	int32 HidingSpotsChecked = 0;
};
