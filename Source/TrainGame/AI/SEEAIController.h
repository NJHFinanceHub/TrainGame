// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "SEEAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class UBlackboardData;
class UDetectionComponent;

// ============================================================================
// ASEEAIController
//
// Master AI controller for all NPCs in Snowpiercer: Eternal Engine.
// Uses UBehaviorTree + UBlackboardData for state-driven AI:
//   Patrol → Investigate → Chase → Combat
// Bridges existing component systems (Detection, Combat, Stealth) with
// a behavior tree backbone. Individual NPC types (Jackboot, Crowd, Companion)
// extend this with specialized trees and blackboard data.
// ============================================================================

UCLASS()
class TRAINGAME_API ASEEAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASEEAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// --- State Management ---

	/** Get the current high-level AI state from blackboard */
	UFUNCTION(BlueprintPure, Category = "AI|State")
	ENPCAIState GetAIState() const;

	/** Set the high-level AI state on the blackboard */
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetAIState(ENPCAIState NewState);

	/** Set the target actor on the blackboard */
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetTargetActor(AActor* Target);

	/** Set the investigation/movement target location */
	UFUNCTION(BlueprintCallable, Category = "AI|State")
	void SetTargetLocation(FVector Location);

	// --- Behavior Tree ---

	/** Start running the assigned behavior tree */
	UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
	void StartBehaviorTree();

	/** Stop the behavior tree */
	UFUNCTION(BlueprintCallable, Category = "AI|BehaviorTree")
	void StopBehaviorTree();

	/** Get the behavior tree component */
	UFUNCTION(BlueprintPure, Category = "AI|BehaviorTree")
	UBehaviorTreeComponent* GetBehaviorTreeComp() const { return BehaviorTreeComp; }

	/** Get the blackboard component */
	UFUNCTION(BlueprintPure, Category = "AI|BehaviorTree")
	UBlackboardComponent* GetBlackboardComp() const { return BlackboardComp; }

	// --- Detection Bridge ---

	/** Called when the NPC's detection state changes */
	UFUNCTION()
	void OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState);

	/** Called when a body is discovered */
	UFUNCTION()
	void OnBodyDiscovered(AActor* Body);

protected:
	/** Behavior tree asset to run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BehaviorTree")
	UBehaviorTree* BehaviorTreeAsset = nullptr;

	/** Blackboard data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BehaviorTree")
	UBlackboardData* BlackboardAsset = nullptr;

	/** NPC role (determines default behaviors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	ENPCRole NPCRole = ENPCRole::ThirdClass;

	/** Whether this NPC can engage in combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Config")
	bool bCanFight = true;

	/** Patrol waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	TArray<FVector> PatrolWaypoints;

	/** Patrol speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	float PatrolSpeed = 200.f;

	/** Alert/chase speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Patrol")
	float AlertSpeed = 400.f;

	/** Combat engagement range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	float CombatRange = 200.f;

	/** Initialize blackboard with default values */
	virtual void InitializeBlackboard();

	/** Map detection state changes to AI state transitions */
	ENPCAIState MapDetectionToAIState(EDetectionState DetectionState) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "AI|Components")
	UBehaviorTreeComponent* BehaviorTreeComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "AI|Components")
	UBlackboardComponent* BlackboardComp = nullptr;

	/** Cached detection component from possessed pawn */
	UPROPERTY()
	UDetectionComponent* DetectionComp = nullptr;
};
