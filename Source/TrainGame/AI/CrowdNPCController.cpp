// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CrowdNPCController.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

ACrowdNPCController::ACrowdNPCController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.5f; // Lower tick rate for performance
}

void ACrowdNPCController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CurrentBehavior = DefaultBehavior;

	if (InPawn)
	{
		WanderCenter = InPawn->GetActorLocation();
	}

	CurrentIdleDuration = FMath::RandRange(MinIdleTime, MaxIdleTime);
}

void ACrowdNPCController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!GetPawn())
	{
		return;
	}

	if (bIsFleeing)
	{
		TickFlee(DeltaTime);
		return;
	}

	switch (CurrentBehavior)
	{
	case ECrowdBehavior::Idle:
	case ECrowdBehavior::SitDown:
	case ECrowdBehavior::Working:
		TickIdle(DeltaTime);
		break;
	case ECrowdBehavior::Walk:
		TickWalk(DeltaTime);
		break;
	case ECrowdBehavior::Talk:
		TickTalk(DeltaTime);
		break;
	}
}

void ACrowdNPCController::SetBehavior(ECrowdBehavior NewBehavior)
{
	CurrentBehavior = NewBehavior;

	if (NewBehavior == ECrowdBehavior::Walk)
	{
		PickNewWanderTarget();
	}
}

void ACrowdNPCController::SetWanderArea(FVector Center, float Radius)
{
	WanderCenter = Center;
	WanderRadius = Radius;
}

void ACrowdNPCController::JoinTalkGroup(const TArray<AActor*>& GroupMembers)
{
	TalkGroup = GroupMembers;
	CurrentBehavior = ECrowdBehavior::Talk;
}

void ACrowdNPCController::ReactToDisturbance(FVector DisturbanceLocation, float Severity)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	if (Severity < 0.3f)
	{
		// Minor disturbance: just turn and look
		return;
	}

	// Flee away from the disturbance
	bIsFleeing = true;
	FleeDirection = (ControlledPawn->GetActorLocation() - DisturbanceLocation).GetSafeNormal();
	FleeTimer = FMath::Lerp(2.f, 5.f, Severity);

	FVector FleeTarget = ControlledPawn->GetActorLocation() + FleeDirection * 800.f;
	MoveToLocation(FleeTarget, 50.f);
}

void ACrowdNPCController::TickIdle(float DeltaTime)
{
	IdleTimer += DeltaTime;

	if (IdleTimer >= CurrentIdleDuration)
	{
		IdleTimer = 0.f;
		CurrentIdleDuration = FMath::RandRange(MinIdleTime, MaxIdleTime);

		// Randomly decide to start walking
		if (FMath::FRand() < 0.4f)
		{
			CurrentBehavior = ECrowdBehavior::Walk;
			PickNewWanderTarget();
		}
	}
}

void ACrowdNPCController::TickWalk(float DeltaTime)
{
	// Check if we've reached our wander target
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	float DistToTarget = FVector::Dist(ControlledPawn->GetActorLocation(), WanderTarget);
	if (DistToTarget < 100.f || GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Moving)
	{
		// Reached destination or path failed — switch to idle
		CurrentBehavior = ECrowdBehavior::Idle;
		IdleTimer = 0.f;
		CurrentIdleDuration = FMath::RandRange(MinIdleTime, MaxIdleTime);
	}
}

void ACrowdNPCController::TickTalk(float DeltaTime)
{
	// Face a random member of the talk group periodically
	if (TalkGroup.Num() == 0)
	{
		CurrentBehavior = ECrowdBehavior::Idle;
		return;
	}

	// Just idle in place — animation system handles talk gestures
}

void ACrowdNPCController::TickFlee(float DeltaTime)
{
	FleeTimer -= DeltaTime;
	if (FleeTimer <= 0.f)
	{
		bIsFleeing = false;
		CurrentBehavior = ECrowdBehavior::Idle;
		StopMovement();
	}
}

void ACrowdNPCController::PickNewWanderTarget()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return;
	}

	FNavLocation NavResult;
	bool bFound = NavSys->GetRandomReachablePointInRadius(WanderCenter, WanderRadius, NavResult);

	if (bFound)
	{
		WanderTarget = NavResult.Location;
		MoveToLocation(WanderTarget, 50.f);
	}
}
