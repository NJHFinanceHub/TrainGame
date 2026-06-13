// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCHealthComponent.h"

#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

USEENPCHealthComponent::USEENPCHealthComponent()
{
	// The base component goes "downed" at zero health and only dies after
	// DownedDuration (a player revive window). NPCs just die: zero duration
	// promotes downed -> dead (OnDeath) on the next component tick, and the
	// AI controller also reacts to OnDowned immediately.
	DownedDuration = 0.0f;

	// Inherits SetIsReplicatedByDefault(true) + DOREPLIFETIME(CurrentHealth/
	// bIsDead) from USEEHealthComponent — NPC health is already server-
	// authoritative and replicated. Nothing extra needed here for the values.
}

void USEENPCHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// Bind the client-side ragdoll to the replicated death edge. OnDeath fires on
	// authority (TakeDamage/Tick) AND on clients (base OnRep_IsDead); the handler
	// itself no-ops on authority so only clients ragdoll from replication.
	OnDeath.AddDynamic(this, &USEENPCHealthComponent::HandleClientDeathRagdoll);
}

void USEENPCHealthComponent::HandleClientDeathRagdoll()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Authority already ran the full death sequence (death anim + delayed ragdoll)
	// in ASEENPCAIController::HandlePawnDeath. Only clients need to react here.
	if (Owner->HasAuthority()) return;

	ACharacter* PawnChar = Cast<ACharacter>(Owner);
	if (!PawnChar) return;

	// Mirror the server's collapse so the corpse looks the same on clients:
	// drop capsule collision, stop movement, and ragdoll the mesh. The mesh's
	// bones then settle via local physics (the server's replicated movement on a
	// dead, movement-disabled pawn no longer fights this).
	if (UCapsuleComponent* Capsule = PawnChar->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (UCharacterMovementComponent* Move = PawnChar->GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	USkeletalMeshComponent* Mesh = PawnChar->GetMesh();
	if (Mesh && Mesh->GetSkeletalMeshAsset())
	{
		Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
		Mesh->SetSimulatePhysics(true);
	}
	else
	{
		// No visible mesh — hide it out of sight, matching the server fallback.
		PawnChar->SetActorHiddenInGame(true);
	}
}

void USEENPCHealthComponent::InitHealth(float InMaxHealth)
{
	MaxHealth = FMath::Max(1.0f, InMaxHealth);
	CurrentHealth = MaxHealth;
}
