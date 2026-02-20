// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "JackbootAIController.h"
#include "SEENPCCharacter.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AJackbootAIController::AJackbootAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AJackbootAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Set rank-based behavior modifiers on the blackboard
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsEnum(BB_AlertLevel, static_cast<uint8>(EAlertLevel::Green));
	}
}

void AJackbootAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TickAlertPropagation(DeltaTime);
}

void AJackbootAIController::RaiseVocalAlert(EAlertLevel Level, FVector TargetLocation)
{
	if (Level <= CurrentAlertLevel)
	{
		return;
	}

	CurrentAlertLevel = Level;

	// Find nearby Jackboots and alert them
	TArray<AJackbootAIController*> NearbyJackboots;
	FindNearbyJackboots(NearbyJackboots, VocalAlertRadius);

	for (AJackbootAIController* Other : NearbyJackboots)
	{
		Other->ReceiveJackbootAlert(Level, TargetLocation, this);
	}

	OnAlertPropagated.Broadcast(Level, TargetLocation, this);

	// If we have a radio, also queue radio alert
	if (bHasRadio && Level >= EAlertLevel::Orange)
	{
		bRadioAlertPending = true;
		RadioAlertTimer = RadioAlertDelay;
		PendingAlertLocation = TargetLocation;
		PendingAlertLevel = Level;
	}
}

void AJackbootAIController::RaiseRadioAlert(EAlertLevel Level, FVector TargetLocation)
{
	if (!bHasRadio)
	{
		return;
	}

	// Radio reaches all Jackboots in the world (train-wide)
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* OtherPawn = *It;
		if (OtherPawn == GetPawn())
		{
			continue;
		}

		AJackbootAIController* OtherJackboot = Cast<AJackbootAIController>(OtherPawn->GetController());
		if (OtherJackboot)
		{
			OtherJackboot->ReceiveJackbootAlert(Level, TargetLocation, this);
		}
	}

	OnAlertPropagated.Broadcast(Level, TargetLocation, this);
}

void AJackbootAIController::ReceiveJackbootAlert(EAlertLevel Level, FVector TargetLocation, AJackbootAIController* Source)
{
	if (Level <= CurrentAlertLevel)
	{
		return;
	}

	CurrentAlertLevel = Level;

	UBlackboardComponent* BB = GetBlackboardComponent();
	if (BB)
	{
		BB->SetValueAsEnum(BB_AlertLevel, static_cast<uint8>(Level));
	}

	// Map alert level to AI state
	switch (Level)
	{
	case EAlertLevel::Yellow:
		SetAIState(ENPCAIState::Investigate);
		SetInvestigationLocation(TargetLocation);
		break;
	case EAlertLevel::Orange:
		SetAIState(ENPCAIState::Chase);
		SetInvestigationLocation(TargetLocation);
		break;
	case EAlertLevel::Red:
		SetAIState(ENPCAIState::Combat);
		SetInvestigationLocation(TargetLocation);
		break;
	default:
		break;
	}
}

void AJackbootAIController::CallForBackup()
{
	if (bBackupCalled)
	{
		return;
	}

	bBackupCalled = true;

	// Radio alert at Red level
	RaiseRadioAlert(EAlertLevel::Red, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector);
}

bool AJackbootAIController::ShouldHoldFormation() const
{
	int32 EngagedCount = CountEngagedSquadMembers();
	int32 MySlot = GetEngagementSlot();

	return MySlot >= MaxSquadEngagers || EngagedCount >= MaxSquadEngagers;
}

int32 AJackbootAIController::GetEngagementSlot() const
{
	// Rank determines priority — higher rank gets lower slot numbers
	int32 BaseSlot = static_cast<int32>(Rank);

	// Reverse: Commander = 0 (first to engage), Grunt = 4
	return 4 - BaseSlot;
}

FVector AJackbootAIController::GetFormationPosition() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return FVector::ZeroVector;
	}

	int32 Slot = GetEngagementSlot();

	// Staggered formation: alternate left/right behind the lead
	float LateralOffset = (Slot % 2 == 0 ? 1.f : -1.f) * FormationSpacing * 0.5f;
	float ForwardOffset = -FormationSpacing * ((Slot + 1) / 2);

	FVector Forward = ControlledPawn->GetActorForwardVector();
	FVector Right = ControlledPawn->GetActorRightVector();

	return ControlledPawn->GetActorLocation() + Forward * ForwardOffset + Right * LateralOffset;
}

bool AJackbootAIController::CanIssueOrders() const
{
	return Rank >= EJackbootRank::Sergeant;
}

void AJackbootAIController::TickAlertPropagation(float DeltaTime)
{
	if (!bRadioAlertPending)
	{
		return;
	}

	RadioAlertTimer -= DeltaTime;
	if (RadioAlertTimer <= 0.f)
	{
		bRadioAlertPending = false;
		RaiseRadioAlert(PendingAlertLevel, PendingAlertLocation);
	}
}

void AJackbootAIController::FindNearbyJackboots(TArray<AJackbootAIController*>& OutJackboots, float Radius) const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FVector MyLocation = ControlledPawn->GetActorLocation();

	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* OtherPawn = *It;
		if (OtherPawn == ControlledPawn)
		{
			continue;
		}

		AJackbootAIController* OtherJackboot = Cast<AJackbootAIController>(OtherPawn->GetController());
		if (!OtherJackboot)
		{
			continue;
		}

		float Dist = FVector::Dist(MyLocation, OtherPawn->GetActorLocation());
		if (Dist <= Radius)
		{
			OutJackboots.Add(OtherJackboot);
		}
	}
}

int32 AJackbootAIController::CountEngagedSquadMembers() const
{
	UBlackboardComponent* BB = GetBlackboardComponent();
	if (!BB)
	{
		return 0;
	}

	UObject* TargetObj = BB->GetValueAsObject(BB_TargetActor);
	if (!TargetObj)
	{
		return 0;
	}

	AActor* Target = Cast<AActor>(TargetObj);
	if (!Target)
	{
		return 0;
	}

	int32 Count = 0;
	TArray<AJackbootAIController*> NearbyJackboots;
	FindNearbyJackboots(NearbyJackboots, 1000.f);

	for (AJackbootAIController* Other : NearbyJackboots)
	{
		if (Other->GetAIState() == ENPCAIState::Combat)
		{
			Count++;
		}
	}

	return Count;
}
