// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CrowdNPCController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

ACrowdNPCController::ACrowdNPCController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.2f; // Crowd NPCs don't need 60hz ticking
}

void ACrowdNPCController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn)
	{
		AnchorPoint = InPawn->GetActorLocation();

		// Set walk speed
		if (ACharacter* Character = Cast<ACharacter>(InPawn))
		{
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = WalkSpeed;
			}
		}
	}

	// Start idle for a random duration before first action
	CurrentBehavior = ECrowdBehavior::Idle;
	BehaviorTimer = FMath::RandRange(0.f, MaxIdleTime);
}

void ACrowdNPCController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	switch (CurrentBehavior)
	{
	case ECrowdBehavior::Walk:
		TickWalk(DeltaTime);
		break;
	case ECrowdBehavior::Idle:
	case ECrowdBehavior::SitDown:
		TickIdle(DeltaTime);
		break;
	case ECrowdBehavior::Talk:
		TickTalk(DeltaTime);
		break;
	case ECrowdBehavior::Flee:
		TickFlee(DeltaTime);
		break;
	}
}

void ACrowdNPCController::TriggerFlee(FVector ThreatLocation)
{
	CurrentBehavior = ECrowdBehavior::Flee;
	FleeFromLocation = ThreatLocation;

	// Set flee speed
	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = FleeSpeed;
		}
	}

	// Move away from threat
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	FVector AwayDir = (ControlledPawn->GetActorLocation() - ThreatLocation).GetSafeNormal();
	FVector FleeTarget = ControlledPawn->GetActorLocation() + AwayDir * FleeDistance;

	MoveToLocation(FleeTarget);
}

void ACrowdNPCController::StopFleeing()
{
	if (CurrentBehavior != ECrowdBehavior::Flee) return;

	// Restore walk speed
	if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
		{
			MoveComp->MaxWalkSpeed = WalkSpeed;
		}
	}

	// Update anchor to current position (don't walk back to danger)
	if (GetPawn())
	{
		AnchorPoint = GetPawn()->GetActorLocation();
	}

	ChooseNextBehavior();
}

void ACrowdNPCController::TickWalk(float DeltaTime)
{
	// Check if we've arrived at destination
	if (GetPathFollowingComponent() &&
		GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Moving)
	{
		ChooseNextBehavior();
	}
}

void ACrowdNPCController::TickIdle(float DeltaTime)
{
	BehaviorTimer -= DeltaTime;
	if (BehaviorTimer <= 0.f)
	{
		ChooseNextBehavior();
	}
}

void ACrowdNPCController::TickTalk(float DeltaTime)
{
	BehaviorTimer -= DeltaTime;

	// Face talk partner
	if (TalkPartner && GetPawn())
	{
		FVector Dir = (TalkPartner->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
		if (!Dir.IsNearlyZero())
		{
			GetPawn()->SetActorRotation(Dir.Rotation());
		}
	}

	if (BehaviorTimer <= 0.f)
	{
		TalkPartner = nullptr;
		ChooseNextBehavior();
	}
}

void ACrowdNPCController::TickFlee(float DeltaTime)
{
	// Check if we've fled far enough
	if (GetPawn())
	{
		float DistFromThreat = FVector::Dist(GetPawn()->GetActorLocation(), FleeFromLocation);
		if (DistFromThreat >= FleeDistance)
		{
			StopFleeing();
		}
	}

	// Also stop if we've reached the flee destination
	if (GetPathFollowingComponent() &&
		GetPathFollowingComponent()->GetStatus() != EPathFollowingStatus::Moving)
	{
		StopFleeing();
	}
}

void ACrowdNPCController::ChooseNextBehavior()
{
	float Roll = FMath::FRand();

	if (Roll < TalkChance)
	{
		// Try to talk
		TalkPartner = FindNearbyTalkPartner();
		if (TalkPartner)
		{
			CurrentBehavior = ECrowdBehavior::Talk;
			BehaviorTimer = TalkDuration;
			StopMovement();
			return;
		}
	}

	if (Roll < 0.6f)
	{
		// Walk to a new spot
		CurrentBehavior = ECrowdBehavior::Walk;
		PickWanderDestination();
	}
	else
	{
		// Idle
		CurrentBehavior = ECrowdBehavior::Idle;
		BehaviorTimer = FMath::RandRange(MinIdleTime, MaxIdleTime);
		StopMovement();
	}
}

void ACrowdNPCController::PickWanderDestination()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		if (NavSys->GetRandomReachablePointInRadius(AnchorPoint, WanderRadius, NavLoc))
		{
			MoveToLocation(NavLoc.Location);
			return;
		}
	}

	// Fallback: random offset from anchor
	FVector Offset(FMath::RandRange(-WanderRadius, WanderRadius),
				   FMath::RandRange(-WanderRadius * 0.3f, WanderRadius * 0.3f), 0.f);
	MoveToLocation(AnchorPoint + Offset);
}

AActor* ACrowdNPCController::FindNearbyTalkPartner() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return nullptr;

	float SearchRadius = 300.f;
	TArray<AActor*> NearbyPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyPawns);

	for (AActor* Actor : NearbyPawns)
	{
		if (Actor == ControlledPawn) continue;

		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn) continue;

		// Only talk to other crowd NPCs
		ACrowdNPCController* OtherCrowd = Cast<ACrowdNPCController>(OtherPawn->GetController());
		if (!OtherCrowd) continue;

		// Must be idle and nearby
		if (OtherCrowd->GetCurrentBehavior() != ECrowdBehavior::Idle &&
			OtherCrowd->GetCurrentBehavior() != ECrowdBehavior::Talk)
		{
			continue;
		}

		float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), OtherPawn->GetActorLocation());
		if (Dist < SearchRadius)
		{
			return OtherPawn;
		}
	}

	return nullptr;
}
