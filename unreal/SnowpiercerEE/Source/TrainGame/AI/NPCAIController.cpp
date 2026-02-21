// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ANPCAIController::ANPCAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ANPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	DetectionComp = InPawn->FindComponentByClass<UDetectionComponent>();
	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.AddDynamic(this, &ANPCAIController::OnDetectionStateChanged);
	}

	InitializeBehaviorTree();

	// Start in idle or scheduled state
	if (bUsesSchedule && DailySchedule.Num() > 0)
	{
		SetAIState(ENPCAIState::Scheduled);
	}
	else if (PatrolWaypoints.Num() > 0)
	{
		SetAIState(ENPCAIState::Patrolling);
	}
	else
	{
		SetAIState(ENPCAIState::Idle);
	}
}

void ANPCAIController::OnUnPossess()
{
	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.RemoveDynamic(this, &ANPCAIController::OnDetectionStateChanged);
		DetectionComp = nullptr;
	}

	Super::OnUnPossess();
}

void ANPCAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ENPCAIState::Dead) return;

	// Periodic schedule check
	if (bUsesSchedule && CurrentState == ENPCAIState::Scheduled)
	{
		ScheduleCheckTimer -= DeltaTime;
		if (ScheduleCheckTimer <= 0.f)
		{
			ScheduleCheckTimer = 5.f; // Check every 5 seconds
			// TODO: Get actual game time from world subsystem
			float GameHour = 12.f; // Placeholder
			UpdateSchedule(GameHour);
		}
	}

	UpdateBlackboard();
}

void ANPCAIController::SetAIState(ENPCAIState NewState)
{
	if (CurrentState == NewState) return;

	ENPCAIState OldState = CurrentState;
	CurrentState = NewState;

	// Adjust movement speed based on state
	if (ACharacter* NPCChar = Cast<ACharacter>(GetPawn()))
	{
		if (UCharacterMovementComponent* MoveComp = NPCChar->GetCharacterMovement())
		{
			switch (NewState)
			{
			case ENPCAIState::Idle:
			case ENPCAIState::Scheduled:
				MoveComp->MaxWalkSpeed = PatrolSpeed;
				break;
			case ENPCAIState::Patrolling:
				MoveComp->MaxWalkSpeed = PatrolSpeed;
				break;
			case ENPCAIState::Investigating:
				MoveComp->MaxWalkSpeed = AlertSpeed;
				break;
			case ENPCAIState::Chasing:
			case ENPCAIState::Combat:
				MoveComp->MaxWalkSpeed = ChaseSpeed;
				break;
			case ENPCAIState::Fleeing:
				MoveComp->MaxWalkSpeed = ChaseSpeed;
				break;
			default:
				break;
			}
		}
	}

	UpdateBlackboard();
}

void ANPCAIController::OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState)
{
	switch (NewState)
	{
	case EDetectionState::Unaware:
		// Return to previous routine
		if (bUsesSchedule)
			SetAIState(ENPCAIState::Scheduled);
		else if (PatrolWaypoints.Num() > 0)
			SetAIState(ENPCAIState::Patrolling);
		else
			SetAIState(ENPCAIState::Idle);
		break;

	case EDetectionState::Suspicious:
		SetAIState(ENPCAIState::Investigating);
		if (DetectionComp)
		{
			SetInvestigationLocation(DetectionComp->GetLastKnownTargetLocation());
		}
		break;

	case EDetectionState::Alerted:
		SetAIState(ENPCAIState::Chasing);
		break;

	case EDetectionState::Combat:
		SetAIState(ENPCAIState::Combat);
		break;
	}
}

void ANPCAIController::SetTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsObject(BBKeys::TargetActor, NewTarget);
	}
}

void ANPCAIController::SetInvestigationLocation(FVector Location)
{
	InvestigationLocation = Location;
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsVector(BBKeys::TargetLocation, Location);
	}
}

void ANPCAIController::SetPatrolWaypoints(const TArray<FVector>& Waypoints)
{
	PatrolWaypoints = Waypoints;
	CurrentPatrolIndex = 0;

	if (PatrolWaypoints.Num() > 0 && CurrentState == ENPCAIState::Idle)
	{
		SetAIState(ENPCAIState::Patrolling);
	}
}

void ANPCAIController::SetSchedule(const TArray<FScheduleEntry>& InSchedule)
{
	DailySchedule = InSchedule;
	bUsesSchedule = DailySchedule.Num() > 0;
}

void ANPCAIController::UpdateSchedule(float GameTimeHours)
{
	const FScheduleEntry* Entry = GetCurrentScheduleEntry(GameTimeHours);
	if (!Entry) return;

	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		BB->SetValueAsEnum(BBKeys::ScheduleActivity, static_cast<uint8>(Entry->Activity));
		BB->SetValueAsName(BBKeys::ScheduleLocation, Entry->LocationTag);
	}
}

void ANPCAIController::InitializeBehaviorTree()
{
	if (BehaviorTreeAsset)
	{
		if (BlackboardAsset)
		{
			UBlackboardComponent* BBComp = Blackboard.Get();
			UseBlackboard(BlackboardAsset, BBComp);
			Blackboard = BBComp;
		}

		RunBehaviorTree(BehaviorTreeAsset);
		UpdateBlackboard();
	}
}

void ANPCAIController::UpdateBlackboard()
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB) return;

	BB->SetValueAsEnum(BBKeys::AIState, static_cast<uint8>(CurrentState));
	BB->SetValueAsEnum(BBKeys::HomeZone, static_cast<uint8>(HomeZone));
	BB->SetValueAsInt(BBKeys::PatrolIndex, CurrentPatrolIndex);

	if (CurrentTarget)
	{
		BB->SetValueAsObject(BBKeys::TargetActor, CurrentTarget);
		if (GetPawn())
		{
			float Dist = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
			BB->SetValueAsFloat(BBKeys::DistToTarget, Dist);
		}

		// Line of sight check
		BB->SetValueAsBool(BBKeys::CanSeeTarget, LineOfSightTo(CurrentTarget));
	}
}

const FScheduleEntry* ANPCAIController::GetCurrentScheduleEntry(float GameTimeHours) const
{
	int32 CurrentHour = FMath::FloorToInt(GameTimeHours) % 24;

	for (const FScheduleEntry& Entry : DailySchedule)
	{
		if (Entry.StartHour <= Entry.EndHour)
		{
			// Normal range (e.g., 8-17)
			if (CurrentHour >= Entry.StartHour && CurrentHour < Entry.EndHour)
			{
				return &Entry;
			}
		}
		else
		{
			// Wraps midnight (e.g., 22-6)
			if (CurrentHour >= Entry.StartHour || CurrentHour < Entry.EndHour)
			{
				return &Entry;
			}
		}
	}

	return nullptr;
}
