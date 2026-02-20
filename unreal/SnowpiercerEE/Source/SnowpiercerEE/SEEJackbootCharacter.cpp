#include "SEEJackbootCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SEECharacterAnimInstance.h"

ASEEJackbootCharacter::ASEEJackbootCharacter()
{
	// Jackboot combat defaults — tougher than standard NPCs
	NPCClass = ESEENPCClass::Jackboot;
	MaxHealth = 150.0f;
	CurrentHealth = 150.0f;
	MeleeDamage = 25.0f;
	AttackRange = 220.0f;
	AttackCooldown = 1.0f;

	// Enhanced detection
	SightRange = 2500.0f;
	SightAngle = 70.0f;
	HearingRange = 1000.0f;
	DetectionRate = 1.5f;

	// Faster movement
	WalkSpeed = 250.0f;
	RunSpeed = 550.0f;

	// Skeletal mesh setup (assign in Blueprint)
	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
}

void ASEEJackbootCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASEEJackbootCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Alert reinforcements when entering combat for the first time
	if (CurrentState == ESEENPCState::Combat && bCanCallReinforcements && !bHasAlertedReinforcements)
	{
		AlertNearbyJackboots();
		bHasAlertedReinforcements = true;
	}

	// Reset alert flag when leaving combat
	if (CurrentState != ESEENPCState::Combat)
	{
		bHasAlertedReinforcements = false;
	}
}

void ASEEJackbootCharacter::ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection)
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

void ASEEJackbootCharacter::ActivateDeathRagdoll()
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

void ASEEJackbootCharacter::AttachToWeaponSocket(AActor* ActorToAttach)
{
	if (!ActorToAttach) return;
	FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	ActorToAttach->AttachToComponent(GetMesh(), Rules, WeaponSocketName);
}

void ASEEJackbootCharacter::AlertNearbyJackboots()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASEEJackbootCharacter::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		ASEEJackbootCharacter* OtherJackboot = Cast<ASEEJackbootCharacter>(Actor);
		if (OtherJackboot && OtherJackboot != this)
		{
			float Distance = FVector::Dist(GetActorLocation(), OtherJackboot->GetActorLocation());
			if (Distance <= ReinforcementAlertRadius && OtherJackboot->GetCurrentState() != ESEENPCState::Dead)
			{
				OtherJackboot->SetState(ESEENPCState::Alerted);
			}
		}
	}
}
