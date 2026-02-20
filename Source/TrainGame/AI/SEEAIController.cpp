// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Enum.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Int.h"
#include "TrainGame/Stealth/DetectionComponent.h"

ASEEAIController::ASEEAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	BehaviorTreeComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ASEEAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn)
	{
		return;
	}

	// Cache detection component
	DetectionComp = InPawn->FindComponentByClass<UDetectionComponent>();
	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.AddDynamic(this, &ASEEAIController::OnDetectionStateChanged);
		DetectionComp->OnBodyDiscovered.AddDynamic(this, &ASEEAIController::OnBodyDiscovered);
	}

	// Initialize blackboard and start behavior tree
	InitializeBlackboard();
	StartBehaviorTree();
}

void ASEEAIController::OnUnPossess()
{
	StopBehaviorTree();

	if (DetectionComp)
	{
		DetectionComp->OnDetectionStateChanged.RemoveDynamic(this, &ASEEAIController::OnDetectionStateChanged);
		DetectionComp->OnBodyDiscovered.RemoveDynamic(this, &ASEEAIController::OnBodyDiscovered);
		DetectionComp = nullptr;
	}

	Super::OnUnPossess();
}

void ASEEAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Sync detection state to blackboard each tick
	if (DetectionComp && BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(SEEBlackboardKeys::DetectionState,
			static_cast<uint8>(DetectionComp->GetDetectionState()));
	}
}

// --- State Management ---

ENPCAIState ASEEAIController::GetAIState() const
{
	if (!BlackboardComp)
	{
		return ENPCAIState::Idle;
	}
	return static_cast<ENPCAIState>(BlackboardComp->GetValueAsEnum(SEEBlackboardKeys::AIState));
}

void ASEEAIController::SetAIState(ENPCAIState NewState)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsEnum(SEEBlackboardKeys::AIState, static_cast<uint8>(NewState));
	}
}

void ASEEAIController::SetTargetActor(AActor* Target)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(SEEBlackboardKeys::TargetActor, Target);
	}
}

void ASEEAIController::SetTargetLocation(FVector Location)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsVector(SEEBlackboardKeys::TargetLocation, Location);
	}
}

// --- Behavior Tree ---

void ASEEAIController::StartBehaviorTree()
{
	if (BehaviorTreeAsset && BehaviorTreeComp && BlackboardComp)
	{
		BehaviorTreeComp->StartTree(*BehaviorTreeAsset);
	}
}

void ASEEAIController::StopBehaviorTree()
{
	if (BehaviorTreeComp && BehaviorTreeComp->IsRunning())
	{
		BehaviorTreeComp->StopTree();
	}
}

// --- Detection Bridge ---

void ASEEAIController::OnDetectionStateChanged(EDetectionState OldState, EDetectionState NewState)
{
	ENPCAIState NewAIState = MapDetectionToAIState(NewState);
	SetAIState(NewAIState);

	// Update target location from detection component
	if (DetectionComp && (NewState == EDetectionState::Suspicious || NewState == EDetectionState::Alerted))
	{
		SetTargetLocation(DetectionComp->GetLastKnownTargetLocation());
	}
}

void ASEEAIController::OnBodyDiscovered(AActor* Body)
{
	if (!Body)
	{
		return;
	}

	// Body discovery triggers investigation at the body's location
	SetTargetLocation(Body->GetActorLocation());

	ENPCAIState CurrentState = GetAIState();
	if (CurrentState < ENPCAIState::Investigate)
	{
		SetAIState(ENPCAIState::Investigate);
	}
}

// --- Protected ---

void ASEEAIController::InitializeBlackboard()
{
	if (!BlackboardComp || !BlackboardAsset)
	{
		return;
	}

	if (BlackboardComp->InitializeBlackboard(*BlackboardAsset))
	{
		// Set initial state
		SetAIState(ENPCAIState::Idle);
		BlackboardComp->SetValueAsEnum(SEEBlackboardKeys::DetectionState,
			static_cast<uint8>(EDetectionState::Unaware));
		BlackboardComp->SetValueAsBool(SEEBlackboardKeys::CanFight, bCanFight);
		BlackboardComp->SetValueAsInt(SEEBlackboardKeys::PatrolIndex, 0);

		if (APawn* MyPawn = GetPawn())
		{
			BlackboardComp->SetValueAsVector(SEEBlackboardKeys::HomeLocation,
				MyPawn->GetActorLocation());
		}
	}
}

ENPCAIState ASEEAIController::MapDetectionToAIState(EDetectionState DetectionState) const
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
		return bCanFight ? ENPCAIState::Combat : ENPCAIState::Flee;

	default:
		return ENPCAIState::Idle;
	}
}
