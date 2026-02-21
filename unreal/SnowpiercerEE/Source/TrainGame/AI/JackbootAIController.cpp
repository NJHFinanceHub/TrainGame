// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "JackbootAIController.h"
#include "CrowdNPCController.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AJackbootAIController::AJackbootAIController()
{
	// Jackboots are disciplined military units
	NPCRole = ENPCClass::Jackboot;
	PatrolSpeed = 250.f;
	AlertSpeed = 400.f;
	ChaseSpeed = 500.f;
}

void AJackbootAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AJackbootAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update cooldowns
	if (AlertCooldownTimer > 0.f)
	{
		AlertCooldownTimer -= DeltaTime;
	}

	// Formation combat logic
	if (GetAIState() == ENPCAIState::Combat && bWaitingForFormation)
	{
		TickFormationCombat(DeltaTime);
	}

	// Backup response
	if (bRespondingToBackup)
	{
		TickBackupResponse(DeltaTime);
	}
}

// --- Alert Propagation ---

void AJackbootAIController::PropagateAlert(EAlertLevel Level, FVector ThreatLocation, AActor* ThreatActor)
{
	if (AlertCooldownTimer > 0.f) return;

	CurrentAlertLevel = Level;
	LastThreatLocation = ThreatLocation;
	AlertCooldownTimer = AlertCooldown;

	float Range = bHasRadio ? RadioAlertRange : VocalAlertRange;
	TArray<AJackbootAIController*> NearbyJackboots = FindNearbyJackboots(Range);

	for (AJackbootAIController* Ally : NearbyJackboots)
	{
		Ally->ReceiveAlert(Level, ThreatLocation, ThreatActor);
	}

	// Crowd NPCs flee from combat alerts
	if (Level >= EAlertLevel::Red)
	{
		TArray<AActor*> AllPawns;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

		for (AActor* Actor : AllPawns)
		{
			APawn* NearbyPawn = Cast<APawn>(Actor);
			if (!NearbyPawn) continue;

			ACrowdNPCController* CrowdAI = Cast<ACrowdNPCController>(NearbyPawn->GetController());
			if (CrowdAI)
			{
				float Dist = FVector::Dist(NearbyPawn->GetActorLocation(), ThreatLocation);
				if (Dist < VocalAlertRange)
				{
					CrowdAI->TriggerFlee(ThreatLocation);
				}
			}
		}
	}
}

void AJackbootAIController::ReceiveAlert(EAlertLevel Level, FVector ThreatLocation, AActor* ThreatInstigator)
{
	// Only escalate, never de-escalate from an incoming alert
	if (Level <= CurrentAlertLevel) return;

	CurrentAlertLevel = Level;
	LastThreatLocation = ThreatLocation;

	if (ThreatInstigator)
	{
		SetTarget(ThreatInstigator);
	}

	SetInvestigationLocation(ThreatLocation);

	// Transition AI state based on alert level
	switch (Level)
	{
	case EAlertLevel::Yellow:
		if (GetAIState() < ENPCAIState::Investigating)
		{
			SetAIState(ENPCAIState::Investigating);
		}
		break;
	case EAlertLevel::Orange:
		if (GetAIState() < ENPCAIState::Chasing)
		{
			SetAIState(ENPCAIState::Chasing);
		}
		break;
	case EAlertLevel::Red:
		SetAIState(ENPCAIState::Combat);
		break;
	default:
		break;
	}
}

// --- Backup ---

void AJackbootAIController::CallForBackup(FVector ThreatLocation)
{
	// Propagate an orange alert to bring reinforcements
	PropagateAlert(EAlertLevel::Orange, ThreatLocation);

	// Set rally point slightly behind the threat (flanking position)
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn)
	{
		FVector DirToThreat = (ThreatLocation - ControlledPawn->GetActorLocation()).GetSafeNormal();
		BackupRallyPoint = ThreatLocation - DirToThreat * FormationSpacing * 2.f;
	}

	// Tell nearby Jackboots to respond
	float Range = bHasRadio ? RadioAlertRange : VocalAlertRange;
	TArray<AJackbootAIController*> NearbyJackboots = FindNearbyJackboots(Range);

	for (AJackbootAIController* Ally : NearbyJackboots)
	{
		if (Ally->GetAIState() < ENPCAIState::Combat) // Don't pull from active fights
		{
			Ally->RespondToBackup(BackupRallyPoint);
		}
	}
}

void AJackbootAIController::RespondToBackup(FVector RallyPoint)
{
	bRespondingToBackup = true;
	BackupRallyPoint = RallyPoint;

	SetAIState(ENPCAIState::Chasing);
	MoveToLocation(RallyPoint);
}

// --- Formation Combat ---

FVector AJackbootAIController::GetFormationCombatPosition() const
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return FVector::ZeroVector;

	// Find our index among nearby Jackboots for formation offset
	TArray<AJackbootAIController*> Allies = FindNearbyJackboots(FormationSpacing * 4.f);
	int32 MyIndex = 0;

	for (int32 i = 0; i < Allies.Num(); ++i)
	{
		if (Allies[i] == this)
		{
			MyIndex = i;
			break;
		}
	}

	// Corridor formation: stagger positions laterally
	FVector DirToTarget = (LastThreatLocation - ControlledPawn->GetActorLocation()).GetSafeNormal();
	FVector Lateral = FVector::CrossProduct(DirToTarget, FVector::UpVector).GetSafeNormal();

	// Alternating left-right offset
	float LateralOffset = (MyIndex % 2 == 0 ? 1.f : -1.f) * FormationSpacing * 0.5f;
	float ForwardOffset = (MyIndex / 2) * FormationSpacing;

	return LastThreatLocation - DirToTarget * (FormationSpacing + ForwardOffset) + Lateral * LateralOffset;
}

bool AJackbootAIController::ShouldWaitForFormation() const
{
	if (MinFormationSize <= 1) return false;

	int32 NearbyCount = GetNearbyAllyCount();
	return NearbyCount < MinFormationSize && FormationWaitTimer < MaxFormationWaitTime;
}

int32 AJackbootAIController::GetNearbyAllyCount() const
{
	return FindNearbyJackboots(FormationSpacing * 3.f).Num();
}

// --- Body Discovery ---

void AJackbootAIController::OnBodyDiscovered(AActor* Body, EBodyState BodyState)
{
	if (!Body) return;

	// Set investigation point to body location
	SetInvestigationLocation(Body->GetActorLocation());

	switch (BodyState)
	{
	case EBodyState::Unconscious:
	case EBodyState::Stunned:
		// Suspicious — investigate and call it in
		PropagateAlert(EAlertLevel::Yellow, Body->GetActorLocation());
		SetAIState(ENPCAIState::Investigating);
		break;

	case EBodyState::Dead:
		// Immediate escalation — call backup and search
		PropagateAlert(EAlertLevel::Orange, Body->GetActorLocation());
		CallForBackup(Body->GetActorLocation());
		SetAIState(ENPCAIState::Chasing);
		break;

	case EBodyState::Hidden:
		// If found hidden, player was definitely here — full alert
		PropagateAlert(EAlertLevel::Red, Body->GetActorLocation());
		CallForBackup(Body->GetActorLocation());
		SetAIState(ENPCAIState::Combat);
		break;

	default:
		break;
	}
}

// --- Private ---

void AJackbootAIController::TickFormationCombat(float DeltaTime)
{
	FormationWaitTimer += DeltaTime;

	if (!ShouldWaitForFormation())
	{
		bWaitingForFormation = false;
		FormationWaitTimer = 0.f;
		return;
	}

	// Move to formation position while waiting
	FVector FormationPos = GetFormationCombatPosition();
	MoveToLocation(FormationPos);
}

void AJackbootAIController::TickBackupResponse(float DeltaTime)
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	float DistToRally = FVector::Dist(ControlledPawn->GetActorLocation(), BackupRallyPoint);
	if (DistToRally < FormationSpacing)
	{
		bRespondingToBackup = false;

		// Arrived at rally — join formation combat
		bWaitingForFormation = true;
		FormationWaitTimer = 0.f;
		SetAIState(ENPCAIState::Combat);
	}
}

void AJackbootAIController::UpdateAlertLevel()
{
	// De-escalate over time if no threats
	// Managed by the AlertPropagationSubsystem
}

TArray<AJackbootAIController*> AJackbootAIController::FindNearbyJackboots(float Range) const
{
	TArray<AJackbootAIController*> Result;
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return Result;

	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	for (AActor* Actor : AllPawns)
	{
		APawn* OtherPawn = Cast<APawn>(Actor);
		if (!OtherPawn || OtherPawn == ControlledPawn) continue;

		AJackbootAIController* OtherJackboot = Cast<AJackbootAIController>(OtherPawn->GetController());
		if (!OtherJackboot) continue;

		float Dist = FVector::Dist(ControlledPawn->GetActorLocation(), OtherPawn->GetActorLocation());
		if (Dist <= Range)
		{
			Result.Add(OtherJackboot);
		}
	}

	return Result;
}
