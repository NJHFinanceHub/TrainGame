#include "SEECompanionCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SEECharacterAnimInstance.h"

ASEECompanionCharacter::ASEECompanionCharacter()
{
	// Companion defaults — capable fighter, loyal ally
	NPCClass = ESEENPCClass::Tailie;
	MaxHealth = 120.0f;
	CurrentHealth = 120.0f;
	MeleeDamage = 18.0f;
	AttackRange = 200.0f;
	AttackCooldown = 1.1f;

	// Companions have good awareness
	SightRange = 2000.0f;
	SightAngle = 80.0f;
	HearingRange = 900.0f;
	DetectionRate = 1.2f;

	// Fast to keep up with player
	WalkSpeed = 220.0f;
	RunSpeed = 560.0f;

	// Skeletal mesh setup (assign in Blueprint)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

void ASEECompanionCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASEECompanionCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ESEENPCState::Dead) return;

	switch (CurrentBehavior)
	{
	case ESEECompanionBehavior::Follow:
		if (CurrentState != ESEENPCState::Combat)
		{
			UpdateFollow(DeltaTime);
		}
		break;
	case ESEECompanionBehavior::Aggressive:
		if (CurrentState != ESEENPCState::Combat)
		{
			UpdateFollow(DeltaTime);
		}
		UpdateCompanionCombat(DeltaTime);
		break;
	case ESEECompanionBehavior::Defensive:
		UpdateFollow(DeltaTime);
		// Only fight if player is attacked (handled by combat state from base class)
		break;
	case ESEECompanionBehavior::Hold:
		// Stay put
		break;
	case ESEECompanionBehavior::Passive:
		UpdateFollow(DeltaTime);
		// Never enter combat
		if (CurrentState == ESEENPCState::Combat)
		{
			SetState(ESEENPCState::Idle);
		}
		break;
	}
}

void ASEECompanionCharacter::SetBehavior(ESEECompanionBehavior NewBehavior)
{
	CurrentBehavior = NewBehavior;
}

void ASEECompanionCharacter::CommandFollow()
{
	SetBehavior(ESEECompanionBehavior::Follow);
	SetState(ESEENPCState::Idle);
	CommandedTarget = nullptr;
}

void ASEECompanionCharacter::CommandHold()
{
	SetBehavior(ESEECompanionBehavior::Hold);
	SetState(ESEENPCState::Idle);
	CommandedTarget = nullptr;
}

void ASEECompanionCharacter::CommandAttack(AActor* Target)
{
	if (!Target) return;
	CommandedTarget = Target;
	SetBehavior(ESEECompanionBehavior::Aggressive);
	SetState(ESEENPCState::Combat);
}

void ASEECompanionCharacter::UpdateFollow(float DeltaTime)
{
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return;

	float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	// Teleport if too far
	if (DistToPlayer > TeleportDistance)
	{
		FVector TeleportLocation = PlayerPawn->GetActorLocation() - PlayerPawn->GetActorForwardVector() * FollowDistance;
		SetActorLocation(TeleportLocation);
		return;
	}

	// Move toward player if beyond follow distance
	if (DistToPlayer > FollowDistance)
	{
		FVector Direction = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		float SpeedFactor = (DistToPlayer > CatchUpDistance) ? 1.0f : 0.6f;
		AddMovementInput(Direction, SpeedFactor);

		// Run to catch up
		if (DistToPlayer > CatchUpDistance)
		{
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		}
	}
}

void ASEECompanionCharacter::UpdateCompanionCombat(float DeltaTime)
{
	// If we have a commanded target, prioritize it
	if (CommandedTarget && !CommandedTarget->IsPendingKillPending())
	{
		float DistToTarget = FVector::Dist(GetActorLocation(), CommandedTarget->GetActorLocation());
		if (DistToTarget <= AggressiveEngageRange)
		{
			SetState(ESEENPCState::Combat);
		}
		return;
	}

	// Look for nearby hostile NPCs when in aggressive mode
	if (CurrentBehavior == ESEECompanionBehavior::Aggressive)
	{
		APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (!PlayerPawn) return;

		// Only auto-engage enemies near the player
		TArray<AActor*> NearbyActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASEENPCCharacter::StaticClass(), NearbyActors);

		for (AActor* Actor : NearbyActors)
		{
			ASEENPCCharacter* OtherNPC = Cast<ASEENPCCharacter>(Actor);
			if (OtherNPC && OtherNPC != this && OtherNPC->GetCurrentState() == ESEENPCState::Combat)
			{
				float DistToNPC = FVector::Dist(GetActorLocation(), OtherNPC->GetActorLocation());
				if (DistToNPC <= AggressiveEngageRange)
				{
					CommandedTarget = OtherNPC;
					SetState(ESEENPCState::Combat);
					return;
				}
			}
		}
	}
}

void ASEECompanionCharacter::ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection)
{
	if (ReactionType == ESEEHitReactionType::DeathRagdoll)
	{
		ActivateDeathRagdoll();
		return;
	}

	USEECharacterAnimInstance* AnimInst = Cast<USEECharacterAnimInstance>(GetMesh()->GetAnimInstance());
	if (AnimInst)
	{
		AnimInst->PlayHitReaction(ReactionType);
	}

	if ((ReactionType == ESEEHitReactionType::KnockbackFront || ReactionType == ESEEHitReactionType::KnockbackBack)
		&& !HitDirection.IsNearlyZero())
	{
		LaunchCharacter(HitDirection * 400.0f + FVector(0, 0, 80.0f), false, false);
	}
}

void ASEECompanionCharacter::ActivateDeathRagdoll()
{
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

void ASEECompanionCharacter::AttachToWeaponSocket(AActor* ActorToAttach)
{
	if (!ActorToAttach) return;
	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	ActorToAttach->AttachToComponent(GetMesh(), Rules, WeaponSocketName);
}
