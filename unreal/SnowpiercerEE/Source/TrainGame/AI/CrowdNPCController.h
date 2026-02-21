// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITypes.h"
#include "CrowdNPCController.generated.h"

// ============================================================================
// ACrowdNPCController
//
// Lightweight AI controller for ambient crowd NPCs. No combat, no detection
// system — just walk, idle, talk, and flee. Uses simple state machine
// instead of full behavior tree for performance (many crowd NPCs per car).
// ============================================================================

UCLASS()
class TRAINGAME_API ACrowdNPCController : public AAIController
{
	GENERATED_BODY()

public:
	ACrowdNPCController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	/** Force all crowd NPCs to flee from a threat */
	UFUNCTION(BlueprintCallable, Category = "Crowd NPC")
	void TriggerFlee(FVector ThreatLocation);

	/** Return to normal behavior after fleeing */
	UFUNCTION(BlueprintCallable, Category = "Crowd NPC")
	void StopFleeing();

	/** Get current crowd behavior */
	UFUNCTION(BlueprintPure, Category = "Crowd NPC")
	ECrowdBehavior GetCurrentBehavior() const { return CurrentBehavior; }

protected:
	/** How far this NPC wanders from their anchor point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float WanderRadius = 400.f;

	/** Walk speed for ambient movement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float WalkSpeed = 150.f;

	/** Flee speed when reacting to danger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float FleeSpeed = 400.f;

	/** How long to idle before choosing next action (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float MinIdleTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float MaxIdleTime = 10.f;

	/** How long to talk when socializing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float TalkDuration = 8.f;

	/** Chance to talk vs walk when choosing next action (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float TalkChance = 0.3f;

	/** How far to flee from a threat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd NPC")
	float FleeDistance = 800.f;

private:
	void TickWalk(float DeltaTime);
	void TickIdle(float DeltaTime);
	void TickTalk(float DeltaTime);
	void TickFlee(float DeltaTime);

	void ChooseNextBehavior();
	void PickWanderDestination();
	AActor* FindNearbyTalkPartner() const;

	ECrowdBehavior CurrentBehavior = ECrowdBehavior::Idle;
	FVector AnchorPoint = FVector::ZeroVector;
	FVector FleeFromLocation = FVector::ZeroVector;
	float BehaviorTimer = 0.f;
	bool bHasReachedDestination = false;

	UPROPERTY()
	AActor* TalkPartner = nullptr;
};
