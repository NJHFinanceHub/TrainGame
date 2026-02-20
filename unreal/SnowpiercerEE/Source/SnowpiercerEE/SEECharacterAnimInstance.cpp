#include "SEECharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SEECharacter.h"
#include "SEECombatComponent.h"
#include "SEEHealthComponent.h"
#include "SEEWeaponBase.h"

void USEECharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void USEECharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
		if (!OwnerCharacter) return;
	}

	UpdateLocomotion(DeltaSeconds);
	UpdateCombat();
	UpdateHealth();
}

void USEECharacterAnimInstance::UpdateLocomotion(float DeltaSeconds)
{
	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	if (!Movement) return;

	FVector Velocity = OwnerCharacter->GetVelocity();
	Speed = Velocity.Size2D();
	bIsInAir = Movement->IsFalling();
	bIsCrouching = Movement->IsCrouching();

	// Calculate direction relative to actor facing
	if (Speed > 10.0f)
	{
		FRotator ActorRotation = OwnerCharacter->GetActorRotation();
		Direction = CalculateDirection(Velocity, ActorRotation);
	}
	else
	{
		Direction = 0.0f;
	}

	// Determine locomotion state
	if (bIsInAir)
	{
		LocomotionState = Velocity.Z > 0.0f ? ESEELocomotionState::Jumping : ESEELocomotionState::Falling;
	}
	else if (bIsCrouching)
	{
		LocomotionState = ESEELocomotionState::Crouching;
	}
	else if (Speed < 10.0f)
	{
		LocomotionState = ESEELocomotionState::Idle;
	}
	else if (Speed >= 650.0f)
	{
		LocomotionState = ESEELocomotionState::Sprinting;
	}
	else if (Speed >= 450.0f)
	{
		LocomotionState = ESEELocomotionState::Running;
	}
	else
	{
		LocomotionState = ESEELocomotionState::Walking;
	}

	// Movement input axes
	FVector LastInput = OwnerCharacter->GetLastMovementInputVector();
	if (!LastInput.IsNearlyZero())
	{
		FRotator ActorRot = OwnerCharacter->GetActorRotation();
		FVector Forward = FRotationMatrix(ActorRot).GetUnitAxis(EAxis::X);
		FVector Right = FRotationMatrix(ActorRot).GetUnitAxis(EAxis::Y);
		MovementInputForward = FVector::DotProduct(LastInput, Forward);
		MovementInputRight = FVector::DotProduct(LastInput, Right);
	}
	else
	{
		MovementInputForward = 0.0f;
		MovementInputRight = 0.0f;
	}
}

void USEECharacterAnimInstance::UpdateCombat()
{
	USEECombatComponent* Combat = OwnerCharacter->FindComponentByClass<USEECombatComponent>();
	if (Combat)
	{
		CombatState = Combat->GetCombatState();
		bIsBlocking = Combat->IsBlocking();
		bHasWeaponEquipped = Combat->GetEquippedWeapon() != nullptr;
		bIsAttacking = (CombatState == ESEECombatState::Attacking);
		bIsDodging = (CombatState == ESEECombatState::Dodging);
		bIsStaggered = (CombatState == ESEECombatState::Staggered);

		// Attack play rate based on weapon speed
		if (bHasWeaponEquipped && Combat->GetEquippedWeapon())
		{
			AttackPlayRate = Combat->GetEquippedWeapon()->GetAttackSpeed();
		}
		else
		{
			AttackPlayRate = 1.0f;
		}
	}
}

void USEECharacterAnimInstance::UpdateHealth()
{
	USEEHealthComponent* Health = OwnerCharacter->FindComponentByClass<USEEHealthComponent>();
	if (Health)
	{
		HealthPercent = Health->GetHealthPercent();
		bIsDead = Health->IsDead();
	}
}

void USEECharacterAnimInstance::PlayHitReaction(ESEEHitReactionType ReactionType)
{
	ActiveHitReaction = ReactionType;
}

void USEECharacterAnimInstance::ClearHitReaction()
{
	ActiveHitReaction = ESEEHitReactionType::None;
}
