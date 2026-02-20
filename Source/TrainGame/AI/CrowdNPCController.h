// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEEAIController.h"
#include "CrowdNPCController.generated.h"

// ============================================================================
// ACrowdNPCController
//
// Lightweight AI for ambient crowd NPCs. These NPCs provide atmosphere
// but don't engage in combat. They walk, idle, talk to each other,
// and react to disturbances by fleeing. Designed for low CPU overhead
// with reduced tick frequency and simple state machine.
// ============================================================================

/** Crowd NPC behavior */
UENUM(BlueprintType)
enum class ECrowdBehavior : uint8
{
	/** Standing in place, looking around */
	Idle		UMETA(DisplayName = "Idle"),

	/** Walking slowly between points */
	Walking		UMETA(DisplayName = "Walking"),

	/** Talking to another crowd NPC */
	Talking		UMETA(DisplayName = "Talking"),

	/** Sitting down */
	Sitting		UMETA(DisplayName = "Sitting"),

	/** Fleeing from danger */
	Fleeing		UMETA(DisplayName = "Fleeing")
};

UCLASS()
class TRAINGAME_API ACrowdNPCController : public ASEEAIController
{
	GENERATED_BODY()

public:
	ACrowdNPCController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	/** React to a nearby disturbance (combat, loud noise, etc.) */
	UFUNCTION(BlueprintCallable, Category = "AI|Crowd")
	void ReactToDisturbance(FVector DisturbanceLocation);

	/** Get current crowd behavior */
	UFUNCTION(BlueprintPure, Category = "AI|Crowd")
	ECrowdBehavior GetCrowdBehavior() const { return CrowdBehavior; }

protected:
	/** Wander radius from spawn point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float WanderRadius = 400.f;

	/** Time range to idle before picking a new action (min, max seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	FVector2D IdleDurationRange = FVector2D(3.f, 10.f);

	/** Walk speed for crowd NPCs (slow, ambient) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float CrowdWalkSpeed = 120.f;

	/** Flee speed when disturbed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float FleeSpeed = 400.f;

	/** How long to flee before calming down */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float FleeDuration = 5.f;

	/** Radius to find a conversation partner */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float ConversationRadius = 300.f;

	/** Chance to start a conversation when idling (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crowd")
	float ConversationChance = 0.3f;

private:
	void TickIdle(float DeltaTime);
	void TickWalking(float DeltaTime);
	void TickTalking(float DeltaTime);
	void TickFleeing(float DeltaTime);

	void PickNewBehavior();
	void StartWalking();
	void StartConversation();
	FVector PickWanderPoint() const;

	ECrowdBehavior CrowdBehavior = ECrowdBehavior::Idle;
	FVector SpawnLocation = FVector::ZeroVector;
	FVector WalkTarget = FVector::ZeroVector;
	float BehaviorTimer = 0.f;
	float CurrentBehaviorDuration = 5.f;
};
