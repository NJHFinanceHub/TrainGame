// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "JackbootAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"

AJackbootAIController::AJackbootAIController()
{
	NPCRole = ENPCRole::Jackboot;
	bCanFight = true;
	PatrolSpeed = 250.f;
	AlertSpeed = 450.f;
	CombatRange = 200.f;
}

void AJackbootAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AJackbootAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateAlertCooldown(DeltaTime);
}

// --- Alert System ---

void AJackbootAIController::RaiseAlert(EJackbootAlertLevel NewLevel, FVector ThreatLocation)
{
	if (NewLevel <= AlertLevel)
	{
		return; // Don't de-escalate via raise
	}

	EJackbootAlertLevel OldLevel = AlertLevel;
	AlertLevel = NewLevel;
	LastThreatLocation = ThreatLocation;
	AlertCooldownTimer = 0.f;

	// Update blackboard
	if (UBlackboardComponent* BB = GetBlackboardComp())
	{
		BB->SetValueAsEnum(SEEBlackboardKeys::AlertLevel, static_cast<uint8>(AlertLevel));
	}

	OnAlertLevelChanged.Broadcast(OldLevel, AlertLevel);

	// Propagate to nearby Jackboots
	PropagateAlert(NewLevel, ThreatLocation);

	// Orange or Red: call backup via radio
	if (NewLevel >= EJackbootAlertLevel::Orange && bHasRadio)
	{
		CallBackup(ThreatLocation);
	}
}

void AJackbootAIController::ReceiveJackbootAlert(EJackbootAlertLevel Level, FVector ThreatLocation, AActor* Caller)
{
	if (Level <= AlertLevel)
	{
		return;
	}

	EJackbootAlertLevel OldLevel = AlertLevel;
	AlertLevel = Level;
	LastThreatLocation = ThreatLocation;
	AlertCooldownTimer = 0.f;

	// Update blackboard
	if (UBlackboardComponent* BB = GetBlackboardComp())
	{
		BB->SetValueAsEnum(SEEBlackboardKeys::AlertLevel, static_cast<uint8>(AlertLevel));
	}

	// Set investigation target to the threat location
	SetTargetLocation(ThreatLocation);

	// Transition AI state based on alert level
	if (Level >= EJackbootAlertLevel::Orange)
	{
		SetAIState(ENPCAIState::Chase);
	}
	else if (Level >= EJackbootAlertLevel::Yellow)
	{
		SetAIState(ENPCAIState::Investigate);
	}

	OnAlertLevelChanged.Broadcast(OldLevel, AlertLevel);
}

void AJackbootAIController::CallBackup(FVector ThreatLocation)
{
	if (!bHasRadio)
	{
		return;
	}

	// Radio call reaches all Jackboots within radio range
	TArray<AJackbootAIController*> NearbyJackboots = FindNearbyJackboots(RadioRange);
	for (AJackbootAIController* Jackboot : NearbyJackboots)
	{
		Jackboot->ReceiveJackbootAlert(AlertLevel, ThreatLocation, GetPawn());
	}
}

// --- Formation ---

void AJackbootAIController::SetFormation(EJackbootFormation NewFormation)
{
	CurrentFormation = NewFormation;
}

FVector AJackbootAIController::GetFormationPosition() const
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return FVector::ZeroVector;
	}

	// Formation positions relative to squad leader / threat direction
	FVector ForwardDir = MyPawn->GetActorForwardVector();
	FVector RightDir = MyPawn->GetActorRightVector();
	FVector BaseLocation = LastThreatLocation != FVector::ZeroVector
		? MyPawn->GetActorLocation()
		: MyPawn->GetActorLocation();

	switch (CurrentFormation)
	{
	case EJackbootFormation::SingleFile:
		// Line up behind the leader
		return BaseLocation - ForwardDir * (FormationSpacing * FormationIndex);

	case EJackbootFormation::Paired:
		// Two abreast
		{
			int32 Row = FormationIndex / 2;
			int32 Side = (FormationIndex % 2 == 0) ? -1 : 1;
			return BaseLocation
				- ForwardDir * (FormationSpacing * Row)
				+ RightDir * (FormationSpacing * 0.5f * Side);
		}

	case EJackbootFormation::ShieldWall:
		// Shoulder to shoulder
		{
			int32 Position = FormationIndex - 1; // Center is 0
			return BaseLocation + RightDir * (FormationSpacing * 0.6f * Position);
		}

	case EJackbootFormation::StaggeredPush:
		// Zigzag advance
		{
			int32 Row = FormationIndex / 2;
			int32 Side = (FormationIndex % 2 == 0) ? -1 : 1;
			float Offset = (Row % 2 == 0) ? 0.f : FormationSpacing * 0.3f;
			return BaseLocation
				- ForwardDir * (FormationSpacing * Row)
				+ RightDir * (FormationSpacing * 0.4f * Side + Offset);
		}
	}

	return BaseLocation;
}

// --- Protected ---

void AJackbootAIController::InitializeBlackboard()
{
	Super::InitializeBlackboard();

	if (UBlackboardComponent* BB = GetBlackboardComp())
	{
		BB->SetValueAsEnum(SEEBlackboardKeys::AlertLevel, static_cast<uint8>(AlertLevel));
		BB->SetValueAsBool(SEEBlackboardKeys::HasRadio, bHasRadio);
	}
}

// --- Private ---

void AJackbootAIController::UpdateAlertCooldown(float DeltaTime)
{
	if (AlertLevel == EJackbootAlertLevel::Green)
	{
		return;
	}

	AlertCooldownTimer += DeltaTime;
	if (AlertCooldownTimer >= AlertCooldownDuration)
	{
		// De-escalate by one level
		EJackbootAlertLevel OldLevel = AlertLevel;
		AlertLevel = static_cast<EJackbootAlertLevel>(
			FMath::Max(0, static_cast<int32>(AlertLevel) - 1));
		AlertCooldownTimer = 0.f;

		if (UBlackboardComponent* BB = GetBlackboardComp())
		{
			BB->SetValueAsEnum(SEEBlackboardKeys::AlertLevel, static_cast<uint8>(AlertLevel));
		}

		OnAlertLevelChanged.Broadcast(OldLevel, AlertLevel);
	}
}

void AJackbootAIController::PropagateAlert(EJackbootAlertLevel Level, FVector ThreatLocation)
{
	// Vocal alert to nearby Jackboots
	TArray<AJackbootAIController*> NearbyJackboots = FindNearbyJackboots(VocalRange);
	for (AJackbootAIController* Jackboot : NearbyJackboots)
	{
		Jackboot->ReceiveJackbootAlert(Level, ThreatLocation, GetPawn());
	}
}

TArray<AJackbootAIController*> AJackbootAIController::FindNearbyJackboots(float Range) const
{
	TArray<AJackbootAIController*> Result;

	APawn* MyPawn = GetPawn();
	if (!MyPawn)
	{
		return Result;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return Result;
	}

	FVector MyLocation = MyPawn->GetActorLocation();

	for (TActorIterator<AJackbootAIController> It(World); It; ++It)
	{
		AJackbootAIController* Other = *It;
		if (Other == this || !Other->GetPawn())
		{
			continue;
		}

		float Distance = FVector::Dist(MyLocation, Other->GetPawn()->GetActorLocation());
		if (Distance <= Range)
		{
			Result.Add(Other);
		}
	}

	return Result;
}
