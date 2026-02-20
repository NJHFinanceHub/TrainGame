// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCController.h"
#include "SEENPCCharacter.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

const FName ASEENPCController::BB_AIState(TEXT("AIState"));
const FName ASEENPCController::BB_TargetActor(TEXT("TargetActor"));
const FName ASEENPCController::BB_InvestigationLocation(TEXT("InvestigationLocation"));
const FName ASEENPCController::BB_PatrolRouteTag(TEXT("PatrolRouteTag"));
const FName ASEENPCController::BB_HomeLocation(TEXT("HomeLocation"));
const FName ASEENPCController::BB_ScheduleActivity(TEXT("ScheduleActivity"));
const FName ASEENPCController::BB_AlertLevel(TEXT("AlertLevel"));
const FName ASEENPCController::BB_IsIncapacitated(TEXT("IsIncapacitated"));

ASEENPCController::ASEENPCController()
{
}

void ASEENPCController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedNPC = Cast<ASEENPCCharacter>(InPawn);
	if (!CachedNPC)
	{
		return;
	}

	InitializeBlackboard(CachedNPC);

	// Bind detection events to blackboard updates
	UDetectionComponent* DetectionComp = CachedNPC->GetDetectionComponent();
	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.AddDynamic(this, &ASEENPCController::OnDetectionStateChanged);
		DetectionComp->OnBodyDiscovered.AddDynamic(this, &ASEENPCController::OnBodyDiscovered);
	}

	// Run the behavior tree
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void ASEENPCController::OnUnPossess()
{
	if (CachedNPC)
	{
		UDetectionComponent* DetectionComp = CachedNPC->GetDetectionComponent();
		if (DetectionComp)
		{
			DetectionComp->OnDetectionStateChanged.RemoveDynamic(this, &ASEENPCController::OnDetectionStateChanged);
			DetectionComp->OnBodyDiscovered.RemoveDynamic(this, &ASEENPCController::OnBodyDiscovered);
		}
	}

	CachedNPC = nullptr;
	Super::OnUnPossess();
}

void ASEENPCController::SetAIState(ENPCAIState NewState)
{
	if (CurrentAIState == NewState)
	{
		return;
	}

	ENPCAIState OldState = CurrentAIState;
	CurrentAIState = NewState;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsEnum(BB_AIState, static_cast<uint8>(NewState));
	}

	if (CachedNPC)
	{
		CachedNPC->OnNPCStateChanged.Broadcast(OldState, NewState);
	}
}

void ASEENPCController::SetTargetActor(AActor* NewTarget)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsObject(BB_TargetActor, NewTarget);
	}
}

void ASEENPCController::SetInvestigationLocation(FVector Location)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsVector(BB_InvestigationLocation, Location);
	}
}

void ASEENPCController::SetPatrolRouteTag(FName RouteTag)
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsName(BB_PatrolRouteTag, RouteTag);
	}
}

void ASEENPCController::OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState)
{
	ENPCAIState NewAIState = MapDetectionToAIState(NewState);
	SetAIState(NewAIState);

	if (CachedNPC)
	{
		UDetectionComponent* DetectionComp = CachedNPC->GetDetectionComponent();
		if (DetectionComp)
		{
			if (NewState == EDetectionState::Suspicious || NewState == EDetectionState::Alerted)
			{
				SetInvestigationLocation(DetectionComp->GetLastKnownTargetLocation());
			}

			if (NewState == EDetectionState::Combat)
			{
				// Set the player as the target
				APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
				SetTargetActor(PlayerPawn);
			}
		}
	}
}

void ASEENPCController::OnBodyDiscovered(AActor* Body)
{
	if (!Body)
	{
		return;
	}

	// Body discovery escalates to Investigate at minimum
	if (CurrentAIState < ENPCAIState::Investigate)
	{
		SetAIState(ENPCAIState::Investigate);
		SetInvestigationLocation(Body->GetActorLocation());
	}
}

void ASEENPCController::InitializeBlackboard(ASEENPCCharacter* NPC)
{
	if (BlackboardAsset)
	{
		UseBlackboard(BlackboardAsset, Blackboard);
	}

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return;
	}

	BB->SetValueAsEnum(BB_AIState, static_cast<uint8>(ENPCAIState::Idle));
	BB->SetValueAsVector(BB_HomeLocation, NPC->GetActorLocation());
	BB->SetValueAsBool(BB_IsIncapacitated, false);
}

ENPCAIState ASEENPCController::MapDetectionToAIState(EDetectionState DetectionState) const
{
	switch (DetectionState)
	{
	case EDetectionState::Unaware:
		return ENPCAIState::Patrol;
	case EDetectionState::Suspicious:
		return ENPCAIState::Investigate;
	case EDetectionState::Alerted:
		return ENPCAIState::Chase;
	case EDetectionState::Combat:
		return ENPCAIState::Combat;
	default:
		return ENPCAIState::Idle;
	}
}
