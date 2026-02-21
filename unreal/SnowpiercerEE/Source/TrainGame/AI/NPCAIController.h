// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TrainGameAITypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "NPCAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;
class UDetectionComponent;

// ============================================================================
// ANPCAIController
//
// Base AI controller with Behavior Tree + Blackboard integration.
// Manages state transitions: Idle → Patrol → Investigate → Chase → Combat.
// Subclassed by JackbootAIController and CrowdNPCController.
// ============================================================================

UCLASS()
class TRAINGAME_API ANPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ANPCAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// --- State ---

	/** Get current high-level AI state */
	UFUNCTION(BlueprintPure, Category = "NPC AI")
	ENPCAIState GetAIState() const { return CurrentState; }

	/** Transition to a new AI state (updates blackboard) */
	UFUNCTION(BlueprintCallable, Category = "NPC AI")
	void SetAIState(ENPCAIState NewState);

	/** Get this NPC's caste/class */
	UFUNCTION(BlueprintPure, Category = "NPC AI")
	ENPCClass GetNPCClass() const { return NPCRole; }

	// --- Detection Response ---

	/** Called when the NPC's detection component changes state */
	UFUNCTION()
	void OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState);

	// --- Target Management ---

	/** Set the current target (player or other threat) */
	UFUNCTION(BlueprintCallable, Category = "NPC AI")
	void SetTarget(AActor* NewTarget);

	/** Set investigation location (noise source, body, etc.) */
	UFUNCTION(BlueprintCallable, Category = "NPC AI")
	void SetInvestigationLocation(FVector Location);

	// --- Patrol ---

	/** Set patrol waypoints for this NPC */
	UFUNCTION(BlueprintCallable, Category = "NPC AI")
	void SetPatrolWaypoints(const TArray<FVector>& Waypoints);

	// --- Schedule ---

	/** Set the NPC's daily schedule */
	UFUNCTION(BlueprintCallable, Category = "NPC AI")
	void SetSchedule(const TArray<FScheduleEntry>& InSchedule);

	/** Update schedule-driven behavior based on current game time */
	void UpdateSchedule(float GameTimeHours);

protected:
	/** Behavior tree asset to run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|BehaviorTree")
	UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard data asset defining keys */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|BehaviorTree")
	UBlackboardData* BlackboardAsset;

	/** NPC caste/class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI")
	ENPCClass NPCRole = ENPCClass::ThirdClass;

	/** Zone this NPC belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI")
	ETrainZone HomeZone = ETrainZone::ThirdClass;

	/** Whether this NPC follows a time-of-day schedule */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Schedule")
	bool bUsesSchedule = false;

	/** The NPC's daily routine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Schedule")
	TArray<FScheduleEntry> DailySchedule;

	/** Patrol waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Patrol")
	TArray<FVector> PatrolWaypoints;

	/** Move speed when patrolling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Movement")
	float PatrolSpeed = 200.f;

	/** Move speed when investigating/alerted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Movement")
	float AlertSpeed = 350.f;

	/** Move speed when chasing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC AI|Movement")
	float ChaseSpeed = 500.f;

	/** Initialize the behavior tree and blackboard */
	void InitializeBehaviorTree();

	/** Write current state to the blackboard */
	void UpdateBlackboard();

	/** Find the current schedule entry based on game time */
	const FScheduleEntry* GetCurrentScheduleEntry(float GameTimeHours) const;

private:
	ENPCAIState CurrentState = ENPCAIState::Idle;

	UPROPERTY()
	UDetectionComponent* DetectionComp = nullptr;

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	FVector InvestigationLocation = FVector::ZeroVector;
	int32 CurrentPatrolIndex = 0;
	float ScheduleCheckTimer = 0.f;
};
