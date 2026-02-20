// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCCharacter.h"
#include "SEEAIController.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "NPCScheduleComponent.h"
#include "BodyDiscoveryComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameFramework/CharacterMovementComponent.h"

ASEENPCCharacter::ASEENPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Default AI controller
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ASEEAIController::StaticClass();

	// Create default components
	DetectionComp = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComp"));
	ScheduleComp = CreateDefaultSubobject<UNPCScheduleComponent>(TEXT("ScheduleComp"));
	BodyDiscoveryComp = CreateDefaultSubobject<UBodyDiscoveryComponent>(TEXT("BodyDiscoveryComp"));
}

void ASEENPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	// Set movement speeds
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = WalkSpeed;
	}

	// Configure AI controller with our assets
	ASEEAIController* SEEAI = Cast<ASEEAIController>(GetController());
	if (SEEAI)
	{
		if (BehaviorTreeAsset)
		{
			SEEAI->BehaviorTreeAsset = BehaviorTreeAsset;
		}
		if (BlackboardAsset)
		{
			SEEAI->BlackboardAsset = BlackboardAsset;
		}
	}
}

void ASEENPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsIncapacitated)
	{
		return;
	}

	// Track AI state changes for delegate
	ENPCAIState CurrentAIState = GetCurrentAIState();
	if (CurrentAIState != LastKnownAIState)
	{
		OnNPCStateChanged.Broadcast(LastKnownAIState, CurrentAIState);
		LastKnownAIState = CurrentAIState;

		// Adjust movement speed based on state
		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			switch (CurrentAIState)
			{
			case ENPCAIState::Patrol:
			case ENPCAIState::Scheduled:
			case ENPCAIState::Idle:
				MoveComp->MaxWalkSpeed = WalkSpeed;
				break;

			case ENPCAIState::Investigate:
			case ENPCAIState::Chase:
			case ENPCAIState::Combat:
			case ENPCAIState::Flee:
				MoveComp->MaxWalkSpeed = RunSpeed;
				break;

			default:
				break;
			}
		}
	}
}

// --- State ---

ENPCAIState ASEENPCCharacter::GetCurrentAIState() const
{
	if (bIsIncapacitated)
	{
		return ENPCAIState::Dead;
	}

	ASEEAIController* SEEAI = Cast<ASEEAIController>(GetController());
	if (SEEAI)
	{
		return SEEAI->GetAIState();
	}
	return ENPCAIState::Idle;
}

// --- Health ---

void ASEENPCCharacter::ApplyDamage(float Damage, AActor* DamageCauser)
{
	if (bIsIncapacitated)
	{
		return;
	}

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Damage);

	if (CurrentHealth <= 0.f)
	{
		Die(DamageCauser);
	}
}

void ASEENPCCharacter::Incapacitate(EBodyState NewBodyState)
{
	if (bIsIncapacitated)
	{
		return;
	}

	MarkAsBody(NewBodyState);
}

// --- Ambient Dialogue ---

FText ASEENPCCharacter::GetRandomAmbientLine() const
{
	if (AmbientLines.Num() == 0)
	{
		return FText::GetEmpty();
	}
	int32 Index = FMath::RandRange(0, AmbientLines.Num() - 1);
	return AmbientLines[Index];
}

// --- Private ---

void ASEENPCCharacter::Die(AActor* Killer)
{
	MarkAsBody(EBodyState::Dead);
}

void ASEENPCCharacter::MarkAsBody(EBodyState State)
{
	bIsIncapacitated = true;
	BodyState = State;

	// Tag the actor so body discovery can find it
	Tags.AddUnique(TEXT("Body"));
	if (State == EBodyState::Dead)
	{
		Tags.AddUnique(TEXT("Dead"));
	}
	else if (State == EBodyState::Stunned)
	{
		Tags.AddUnique(TEXT("Stunned"));
	}

	// Stop AI
	ASEEAIController* SEEAI = Cast<ASEEAIController>(GetController());
	if (SEEAI)
	{
		SEEAI->SetAIState(ENPCAIState::Dead);
		SEEAI->StopBehaviorTree();
	}

	// Disable movement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
}
