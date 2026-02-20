// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CrowdNPCController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ACrowdNPCController::ACrowdNPCController()
{
	// Crowd NPCs tick at reduced frequency for performance
	PrimaryActorTick.TickInterval = 0.5f;

	// Crowd NPCs don't fight
	bCanFight = false;
	NPCRole = ENPCRole::Crowd;
}

void ACrowdNPCController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		SpawnLocation = InPawn->GetActorLocation();

		// Set walk speed
		if (ACharacter* Character = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = CrowdWalkSpeed;
			}
		}
	}

	PickNewBehavior();
}

void ACrowdNPCController::Tick(float DeltaTime)
{
	// Skip the parent Tick to avoid behavior tree overhead for crowd NPCs
	AAIController::Tick(DeltaTime);

	BehaviorTimer += DeltaTime;

	switch (CrowdBehavior)
	{
	case ECrowdBehavior::Idle:
		TickIdle(DeltaTime);
		break;
	case ECrowdBehavior::Walking:
		TickWalking(DeltaTime);
		break;
	case ECrowdBehavior::Talking:
		TickTalking(DeltaTime);
		break;
	case ECrowdBehavior::Fleeing:
		TickFleeing(DeltaTime);
		break;
	case ECrowdBehavior::Sitting:
		TickIdle(DeltaTime); // Sitting uses same timer logic as idle
		break;
	}
}

void ACrowdNPCController::ReactToDisturbance(FVector DisturbanceLocation)
{
	CrowdBehavior = ECrowdBehavior::Fleeing;
	BehaviorTimer = 0.f;
	CurrentBehaviorDuration = FleeDuration;

	// Flee away from disturbance
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return;
	}

	FVector FleeDirection = (MyPawn->GetActorLocation() - DisturbanceLocation).GetSafeNormal();
	FVector FleeTarget = MyPawn->GetActorLocation() + FleeDirection * WanderRadius;

	// Set flee speed
	if (ACharacter* Character = Cast<ACharacter>(MyPawn))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = FleeSpeed;
		}
	}

	MoveToLocation(FleeTarget);
}

// --- Private ---

void ACrowdNPCController::TickIdle(float DeltaTime)
{
	if (BehaviorTimer >= CurrentBehaviorDuration)
	{
		PickNewBehavior();
	}
}

void ACrowdNPCController::TickWalking(float DeltaTime)
{
	// Check if reached destination or timed out
	EPathFollowingStatus::Type Status = GetMoveStatus();
	if (Status != EPathFollowingStatus::Moving || BehaviorTimer >= CurrentBehaviorDuration)
	{
		CrowdBehavior = ECrowdBehavior::Idle;
		BehaviorTimer = 0.f;
		CurrentBehaviorDuration = FMath::RandRange(IdleDurationRange.X, IdleDurationRange.Y);
	}
}

void ACrowdNPCController::TickTalking(float DeltaTime)
{
	if (BehaviorTimer >= CurrentBehaviorDuration)
	{
		PickNewBehavior();
	}
}

void ACrowdNPCController::TickFleeing(float DeltaTime)
{
	if (BehaviorTimer >= CurrentBehaviorDuration)
	{
		// Calm down, return to normal behavior
		if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
		{
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = CrowdWalkSpeed;
			}
		}

		CrowdBehavior = ECrowdBehavior::Idle;
		BehaviorTimer = 0.f;
		CurrentBehaviorDuration = FMath::RandRange(IdleDurationRange.X, IdleDurationRange.Y);
	}
}

void ACrowdNPCController::PickNewBehavior()
{
	BehaviorTimer = 0.f;
	float Roll = FMath::FRand();

	if (Roll < ConversationChance)
	{
		StartConversation();
	}
	else if (Roll < 0.7f)
	{
		StartWalking();
	}
	else
	{
		// Idle or sit
		CrowdBehavior = (FMath::FRand() < 0.5f) ? ECrowdBehavior::Idle : ECrowdBehavior::Sitting;
		CurrentBehaviorDuration = FMath::RandRange(IdleDurationRange.X, IdleDurationRange.Y);
	}
}

void ACrowdNPCController::StartWalking()
{
	CrowdBehavior = ECrowdBehavior::Walking;
	CurrentBehaviorDuration = 15.f; // Timeout
	WalkTarget = PickWanderPoint();
	MoveToLocation(WalkTarget);
}

void ACrowdNPCController::StartConversation()
{
	CrowdBehavior = ECrowdBehavior::Talking;
	CurrentBehaviorDuration = FMath::RandRange(5.f, 15.f);
	StopMovement();
}

FVector ACrowdNPCController::PickWanderPoint() const
{
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys)
	{
		// Fallback: random offset from spawn
		return SpawnLocation + FVector(
			FMath::RandRange(-WanderRadius, WanderRadius),
			FMath::RandRange(-WanderRadius, WanderRadius),
			0.f
		);
	}

	FNavLocation NavLoc;
	if (NavSys->GetRandomReachablePointInRadius(SpawnLocation, WanderRadius, NavLoc))
	{
		return NavLoc.Location;
	}

	return SpawnLocation;
}
