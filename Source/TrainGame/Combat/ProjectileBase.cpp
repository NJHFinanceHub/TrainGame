// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ProjectileBase.h"
#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(10.f);
	CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->SetGenerateOverlapEvents(true);
	SetRootComponent(CollisionComp);

	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlapBegin);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = GravityScale;
	ProjectileMovement->bShouldBounce = false;
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(LifeSpan);

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->ProjectileGravityScale = GravityScale;

	if (bCanRicochet)
	{
		ProjectileMovement->bShouldBounce = true;
		ProjectileMovement->Bounciness = 0.6f;
	}
}

void AProjectileBase::InitProjectile(float InDamage, EDamageType InDamageType, EProjectileType InProjectileType, AActor* InInstigator)
{
	Damage = InDamage;
	DamageType = InDamageType;
	ProjectileType = InProjectileType;
	ProjectileInstigator = InInstigator;
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit) return;
	if (OtherActor == ProjectileInstigator) return;

	// Check if we hit a combatant
	if (OtherActor)
	{
		UCombatComponent* CombatComp = OtherActor->FindComponentByClass<UCombatComponent>();
		if (CombatComp)
		{
			ApplyDamageToTarget(OtherActor);
			bHasHit = true;
			Destroy();
			return;
		}
	}

	// Hit a wall - check ricochet
	if (bCanRicochet && RicochetCount < MaxRicochets)
	{
		RicochetCount++;
		// ProjectileMovement handles bounce automatically
		return;
	}

	// No more ricochets, destroy
	bHasHit = true;
	Destroy();
}

void AProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bHasHit) return;
	if (OtherActor == ProjectileInstigator) return;
	if (!OtherActor) return;

	UCombatComponent* CombatComp = OtherActor->FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		ApplyDamageToTarget(OtherActor);
		bHasHit = true;
		Destroy();
	}
}

void AProjectileBase::ApplyDamageToTarget(AActor* Target)
{
	if (!Target) return;

	UCombatComponent* CombatComp = Target->FindComponentByClass<UCombatComponent>();
	if (!CombatComp || !CombatComp->IsAlive()) return;

	CombatComp->ReceiveAttack(Damage, EAttackDirection::Mid, DamageType, ProjectileInstigator);

	// Apply knockback
	if (KnockbackForce > 0.f)
	{
		ACharacter* TargetChar = Cast<ACharacter>(Target);
		if (TargetChar)
		{
			FVector KnockDir = GetVelocity().GetSafeNormal();
			TargetChar->LaunchCharacter(KnockDir * KnockbackForce, true, false);
		}
	}
}
