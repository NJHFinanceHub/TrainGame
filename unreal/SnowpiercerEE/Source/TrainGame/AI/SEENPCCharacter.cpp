// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCCharacter.h"
#include "BodyDiscoveryComponent.h"
#include "NPCScheduleComponent.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Dialogue/NPCMemoryComponent.h"
#include "TrainGame/Companions/CompanionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ASEENPCCharacter::ASEENPCCharacter()
{
	PrimaryActorTick.bCanEverTick = false; // Components handle their own ticking

	// --- Required Components ---

	DetectionComp = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComponent"));
	ScheduleComp = CreateDefaultSubobject<UNPCScheduleComponent>(TEXT("ScheduleComponent"));
	BodyDiscoveryComp = CreateDefaultSubobject<UBodyDiscoveryComponent>(TEXT("BodyDiscoveryComponent"));

	// --- Optional Components (null by default, added by subclasses or data) ---
	CombatComp = nullptr;
	MemoryComp = nullptr;
	CompanionComp = nullptr;
}

void ASEENPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Attempt to find optional components that may have been added in Blueprint
	if (!CombatComp)
	{
		CombatComp = FindComponentByClass<UCombatComponent>();
	}
	if (!MemoryComp)
	{
		MemoryComp = FindComponentByClass<UNPCMemoryComponent>();
	}
	if (!CompanionComp)
	{
		CompanionComp = FindComponentByClass<UCompanionComponent>();
	}
}

void ASEENPCCharacter::Incapacitate(EBodyState State)
{
	if (bIsIncapacitated) return;

	bIsIncapacitated = true;
	CurrentBodyState = State;

	// Disable AI
	if (AController* AIController = GetController())
	{
		AIController->UnPossess();
	}

	// Disable movement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	// Register body for discovery by other NPCs
	UBodyDiscoveryComponent::RegisterBody(this, State);

	// Disable collision for ragdoll if dead
	if (State == EBodyState::Dead)
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetSimulatePhysics(true);
	}

	OnNPCIncapacitated.Broadcast(State);
}

void ASEENPCCharacter::SetState(ENPCAIState NewState)
{
	if (CurrentState == NewState) return;
	ENPCAIState OldState = CurrentState;
	CurrentState = NewState;
	OnNPCStateChanged.Broadcast(OldState, NewState);
}

void ASEENPCCharacter::Revive()
{
	if (!bIsIncapacitated) return;

	bIsIncapacitated = false;
	CurrentBodyState = EBodyState::Stunned; // Reset

	// Unregister from body discovery
	UBodyDiscoveryComponent::UnregisterBody(this);

	// Re-enable movement
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}

	// Re-enable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetSimulatePhysics(false);

	// Re-possess with AI
	SpawnDefaultController();
}
