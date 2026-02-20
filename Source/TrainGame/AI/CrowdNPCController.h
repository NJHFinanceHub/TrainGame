// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AITypes.h"
#include "CrowdNPCController.generated.h"

// ============================================================================
// ACrowdNPCController
//
// Lightweight AI controller for ambient crowd NPCs. No combat, no detection.
// Drives simple behaviors: walk between points, idle, sit, talk in groups.
// Designed for low CPU cost — large numbers of these populate the train.
// ============================================================================

UCLASS()
class TRAINGAME_API ACrowdNPCController : public AAIController
{
	GENERATED_BODY()

public:
	ACrowdNPCController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "CrowdAI")
	void SetBehavior(ECrowdBehavior NewBehavior);

	UFUNCTION(BlueprintPure, Category = "CrowdAI")
	ECrowdBehavior GetCurrentBehavior() const { return CurrentBehavior; }

	/** Set a wander area (crowd NPCs walk randomly within this radius) */
	UFUNCTION(BlueprintCallable, Category = "CrowdAI")
	void SetWanderArea(FVector Center, float Radius);

	/** Assign a talk group (crowd NPCs face each other and idle) */
	UFUNCTION(BlueprintCallable, Category = "CrowdAI")
	void JoinTalkGroup(const TArray<AActor*>& GroupMembers);

	/** React to a nearby disturbance (flee, cower, scatter) */
	UFUNCTION(BlueprintCallable, Category = "CrowdAI")
	void ReactToDisturbance(FVector DisturbanceLocation, float Severity);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdAI")
	ECrowdBehavior DefaultBehavior = ECrowdBehavior::Idle;

	/** Walk speed for ambient wandering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdAI")
	float WanderSpeed = 120.f;

	/** Minimum time to idle before picking a new wander destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdAI")
	float MinIdleTime = 3.f;

	/** Maximum time to idle before picking a new wander destination */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdAI")
	float MaxIdleTime = 10.f;

	/** Flee speed when reacting to disturbance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CrowdAI")
	float FleeSpeed = 400.f;

private:
	void TickIdle(float DeltaTime);
	void TickWalk(float DeltaTime);
	void TickTalk(float DeltaTime);
	void TickFlee(float DeltaTime);

	void PickNewWanderTarget();

	ECrowdBehavior CurrentBehavior = ECrowdBehavior::Idle;

	FVector WanderCenter = FVector::ZeroVector;
	float WanderRadius = 500.f;
	FVector WanderTarget = FVector::ZeroVector;

	float IdleTimer = 0.f;
	float CurrentIdleDuration = 5.f;

	bool bIsFleeing = false;
	FVector FleeDirection = FVector::ZeroVector;
	float FleeTimer = 0.f;

	UPROPERTY()
	TArray<AActor*> TalkGroup;
};
