// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ProjectileBase.h"
#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetGenerateOverlapEvents(false);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan);
	CurrentDamage = Damage;

	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);

	// Configure movement
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = MaxSpeed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->bShouldBounce = bCanRicochet;
}

void AProjectileBase::FireInDirection(FVector Direction, AActor* ProjectileOwner)
{
	OwnerActor = ProjectileOwner;
	Direction.Normalize();
	ProjectileMovement->Velocity = Direction * InitialSpeed;
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == OwnerActor)
	{
		return;
	}

	// Check if we hit a combatant
	UCombatComponent* TargetCombat = OtherActor->FindComponentByClass<UCombatComponent>();
	if (TargetCombat)
	{
		ApplyDamageToTarget(OtherActor, CurrentDamage);
		OnProjectileHit.Broadcast(OtherActor, Hit);

		if (!bCanPenetrate)
		{
			Destroy();
		}
		return;
	}

	// Hit a wall/environment
	if (bCanRicochet && RicochetCount < MaxRicochets)
	{
		RicochetCount++;
		CurrentDamage *= RicochetDamageMultiplier;

		// Reflect velocity off the surface normal
		FVector Reflected = FMath::GetReflectionVector(ProjectileMovement->Velocity, Hit.Normal);
		ProjectileMovement->Velocity = Reflected;
	}
	else
	{
		Destroy();
	}
}

void AProjectileBase::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
	UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
	if (!TargetCombat)
	{
		return;
	}

	// Ranged attacks come from the front — use Mid direction
	EDamageType AppliedType = bNonLethal ? EDamageType::NonLethal : DamageType;
	TargetCombat->ReceiveAttack(DamageAmount, EAttackDirection::Mid, AppliedType, OwnerActor);
}
