#include "SEECivilianCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SEECharacterAnimInstance.h"

ASEECivilianCharacter::ASEECivilianCharacter()
{
	// Civilians are non-combatants with lower stats
	NPCClass = ESEENPCClass::Tailie;
	MaxHealth = 60.0f;
	CurrentHealth = 60.0f;
	MeleeDamage = 5.0f;
	AttackRange = 100.0f;

	// Civilians are less observant
	SightRange = 1200.0f;
	SightAngle = 50.0f;
	HearingRange = 500.0f;
	DetectionRate = 0.5f;

	// Standard movement
	WalkSpeed = 180.0f;
	RunSpeed = 380.0f;

	// Skeletal mesh setup (assign variant mesh in Blueprint)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

void ASEECivilianCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Civilians flee when alerted instead of fighting
	if (CurrentState == ESEENPCState::Combat || CurrentState == ESEENPCState::Alerted)
	{
		SetState(ESEENPCState::Fleeing);
	}

	if (CurrentState == ESEENPCState::Fleeing)
	{
		FleeFromThreat(DeltaTime);
	}
}

void ASEECivilianCharacter::FleeFromThreat(float DeltaTime)
{
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return;

	FVector AwayFromPlayer = (GetActorLocation() - PlayerPawn->GetActorLocation()).GetSafeNormal2D();
	AddMovementInput(AwayFromPlayer, 1.0f);

	// Stop fleeing if far enough away
	float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	if (Distance > SightRange * 1.5f)
	{
		SetState(ESEENPCState::Idle);
	}
}

void ASEECivilianCharacter::ActivateDeathRagdoll()
{
	if (bIsEssential) return;

	SetState(ESEENPCState::Dead);

	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayHitReaction(ESEEHitReactionType::DeathRagdoll);
	}

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
