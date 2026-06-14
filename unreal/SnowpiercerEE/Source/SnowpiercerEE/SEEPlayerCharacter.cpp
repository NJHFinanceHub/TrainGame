#include "SEEPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SEEHealthComponent.h"
#include "SEECombatComponent.h"
#include "SEECharacterAnimInstance.h"
#include "Animation/SEEAnimDriverComponent.h"
#include "SnowpiercerEEGameMode.h"

ASEEPlayerCharacter::ASEEPlayerCharacter()
{
	// Player uses a visible skeletal mesh (assign in Blueprint)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	// Code-driven animation (no AnimBlueprint): the driver flips the mesh to
	// single-node mode at init and swaps clips per frame from movement state.
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	AnimDriver = CreateDefaultSubobject<USEEAnimDriverComponent>(TEXT("AnimDriver"));
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

	// Drive the attack one-shot from combat state changes.
	if (CombatComponent)
	{
		CombatComponent->OnCombatStateChanged.AddDynamic(this, &ASEEPlayerCharacter::OnCombatStateChanged);
	}
}

void ASEEPlayerCharacter::OnCombatStateChanged(ESEECombatState NewState)
{
	if (NewState == ESEECombatState::Attacking && AnimDriver)
	{
		AnimDriver->PlayAction(ESEEAnimAction::Attack);
	}
}

void ASEEPlayerCharacter::FellOutOfWorld(const UDamageType& DmgType)
{
	// Do NOT call Super — AActor::FellOutOfWorld destroys the pawn.
	if (ASnowpiercerEEGameMode* GM = GetWorld()->GetAuthGameMode<ASnowpiercerEEGameMode>())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GM->RespawnPlayer();
		return;
	}
	Super::FellOutOfWorld(DmgType);
}

void ASEEPlayerCharacter::OnDamageTaken(float Damage, ESEEDamageType DamageType, AActor* DamageInstigator)
{
	if (!HealthComponent || HealthComponent->IsDead()) return;

	// Brief FOV dip for combat hits only (environmental drains tick with no instigator)
	if (DamageInstigator)
	{
		AddCameraFOVImpulse(-DamageFOVDip);
	}

	if (bInHitReaction) return;

	// Environmental drains (hunger/cold, no instigator) tick continuously —
	// they should not lock the player into a hit-reaction loop
	if (!DamageInstigator) return;

	// Determine reaction severity based on damage
	float DamagePercent = (HealthComponent->GetMaxHealth() > 0.0f)
		? (Damage / HealthComponent->GetMaxHealth()) * 100.0f
		: 0.0f;

	FVector HitDir = (GetActorLocation() - DamageInstigator->GetActorLocation()).GetSafeNormal();

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

	// Code-driven hit reaction one-shot.
	if (AnimDriver)
	{
		AnimDriver->PlayAction(ESEEAnimAction::HitReact);
	}

	// Legacy AnimInstance path: harmless no-op under single-node mode (cast is
	// null), kept so an AnimBlueprint-driven mesh would still react if assigned.
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

	// Play the death one-shot first so the driver stops driving locomotion;
	// it holds the last frame and yields the moment physics starts simulating.
	if (AnimDriver)
	{
		AnimDriver->PlayAction(ESEEAnimAction::Death);
	}

	// Legacy AnimInstance path: harmless no-op under single-node mode.
	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayHitReaction(ESEEHitReactionType::DeathRagdoll);
	}

	// Stop the pawn before ragdolling so the corpse doesn't inherit a launch
	// velocity (a mesh that imported lying down + leftover movement velocity is
	// what made bodies fly across the car). Disable capsule collision and movement
	// first, then enable physics on an already-still mesh so it settles in place.
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Enable ragdoll physics, then zero any residual body velocities so the limp
	// body drops where it stands instead of being flung.
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	GetMesh()->SetAllPhysicsAngularVelocityInRadians(FVector::ZeroVector);
}

void ASEEPlayerCharacter::AttachToWeaponSocket(AActor* ActorToAttach)
{
	if (!ActorToAttach) return;

	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	ActorToAttach->AttachToComponent(GetMesh(), Rules, WeaponSocketName);
}

void ASEEPlayerCharacter::AttachWeaponActorToHand(AActor* WeaponActor)
{
	if (!WeaponActor) return;

	// Use the configured socket when the skeletal mesh actually has it (socket or bone)
	if (GetMesh() && GetMesh()->DoesSocketExist(WeaponSocketName))
	{
		AttachToWeaponSocket(WeaponActor);
		return;
	}

	Super::AttachWeaponActorToHand(WeaponActor);
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
