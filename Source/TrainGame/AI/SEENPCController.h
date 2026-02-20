// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITypes.h"
#include "SEENPCController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UBlackboardComponent;
class ASEENPCCharacter;
class UDetectionComponent;

// ============================================================================
// ASEENPCController
//
// Primary AI controller for NPCs. Uses UBehaviorTree + UBlackboardData
// to drive patrol/investigate/chase/combat states. Wraps detection and
// schedule events into blackboard key changes that the BT reads.
// ============================================================================

UCLASS()
class TRAINGAME_API ASEENPCController : public AAIController
{
	GENERATED_BODY()

public:
	ASEENPCController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// --- Blackboard Helpers ---

	UFUNCTION(BlueprintCallable, Category = "NPCAI")
	void SetAIState(ENPCAIState NewState);

	UFUNCTION(BlueprintPure, Category = "NPCAI")
	ENPCAIState GetAIState() const { return CurrentAIState; }

	UFUNCTION(BlueprintCallable, Category = "NPCAI")
	void SetTargetActor(AActor* NewTarget);

	UFUNCTION(BlueprintCallable, Category = "NPCAI")
	void SetInvestigationLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "NPCAI")
	void SetPatrolRouteTag(FName RouteTag);

	// --- Blackboard Key Names ---

	static const FName BB_AIState;
	static const FName BB_TargetActor;
	static const FName BB_InvestigationLocation;
	static const FName BB_PatrolRouteTag;
	static const FName BB_HomeLocation;
	static const FName BB_ScheduleActivity;
	static const FName BB_AlertLevel;
	static const FName BB_IsIncapacitated;

protected:
	/** Behavior tree asset to run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCAI")
	UBehaviorTree* BehaviorTreeAsset;

	/** Blackboard data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCAI")
	UBlackboardData* BlackboardAsset;

private:
	UFUNCTION()
	void OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState);

	UFUNCTION()
	void OnBodyDiscovered(AActor* Body);

	void InitializeBlackboard(ASEENPCCharacter* NPC);
	ENPCAIState MapDetectionToAIState(EDetectionState DetectionState) const;

	ENPCAIState CurrentAIState = ENPCAIState::Idle;

	UPROPERTY()
	ASEENPCCharacter* CachedNPC = nullptr;
};
