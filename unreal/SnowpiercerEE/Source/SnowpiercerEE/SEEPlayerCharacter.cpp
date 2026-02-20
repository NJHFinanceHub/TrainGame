#include "SEEPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SEEHealthComponent.h"
#include "SEECharacterAnimInstance.h"

ASEEPlayerCharacter::ASEEPlayerCharacter()
{
	// Player uses a visible skeletal mesh (assign in Blueprint)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

void ASEEPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind to health component for hit reactions
	if (HealthComponent)
	{
		HealthComponent->OnDamageTaken.AddDynamic(this, &ASEEPlayerCharacter::OnDamageTaken);
		HealthComponent->OnDeath.AddDynamic(this, &ASEEPlayerCharacter::ActivateDeathRagdoll);
	}
}

void ASEEPlayerCharacter::OnDamageTaken(float Damage, ESEEDamageType DamageType, AActor* Instigator)
{
	if (bInHitReaction) return;
	if (!HealthComponent || HealthComponent->IsDead()) return;

	// Determine reaction severity based on damage
	float DamagePercent = (HealthComponent->GetMaxHealth() > 0.0f)
		? (Damage / HealthComponent->GetMaxHealth()) * 100.0f
		: 0.0f;

	FVector HitDir = FVector::ZeroVector;
	if (Instigator)
	{
		HitDir = (GetActorLocation() - Instigator->GetActorLocation()).GetSafeNormal();
	}

	if (DamagePercent >= HeavyStaggerThreshold)
	{
		// Large hit: knockback
		ApplyHitReaction(ESEEHitReactionType::KnockbackFront, HitDir);
	}
	else
	{
		// Small hit: light stagger
		ApplyHitReaction(ESEEHitReactionType::StaggerLight, HitDir);
	}
}

void ASEEPlayerCharacter::ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection)
{
	if (ReactionType == ESEEHitReactionType::DeathRagdoll)
	{
		ActivateDeathRagdoll();
		return;
	}

	bInHitReaction = true;

	// Notify anim instance
	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayHitReaction(ReactionType);
	}

	// Apply knockback impulse for knockback reactions
	if (ReactionType == ESEEHitReactionType::KnockbackFront || ReactionType == ESEEHitReactionType::KnockbackBack)
	{
		if (!HitDirection.IsNearlyZero())
		{
			LaunchCharacter(HitDirection * KnockbackImpulse + FVector(0, 0, 100.0f), false, false);
		}
	}

	// Set recovery timer
	GetWorldTimerManager().SetTimer(StaggerRecoveryTimer, this, &ASEEPlayerCharacter::EndStagger, StaggerRecoveryTime, false);
}

void ASEEPlayerCharacter::EndStagger()
{
	bInHitReaction = false;

	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->ClearHitReaction();
	}
}

void ASEEPlayerCharacter::ActivateDeathRagdoll()
{
	bInHitReaction = true;

	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayHitReaction(ESEEHitReactionType::DeathRagdoll);
	}

	// Enable ragdoll physics
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASEEPlayerCharacter::AttachToWeaponSocket(AActor* ActorToAttach)
{
	if (!ActorToAttach) return;

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	ActorToAttach->AttachToComponent(GetMesh(), Rules, WeaponSocketName);
}

void ASEEPlayerCharacter::DetachFromWeaponSocket()
{
	// Detach all actors attached to the weapon socket
	TArray<AActor*> AttachedActors;
	GetAttachedActors(AttachedActors);
	for (AActor* Attached : AttachedActors)
	{
		if (Attached)
		{
			FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, false);
			Attached->DetachFromActor(DetachRules);
		}
	}
}
