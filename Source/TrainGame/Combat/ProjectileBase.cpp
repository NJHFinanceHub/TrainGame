// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ProjectileBase.h"
#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.f);
	CollisionComp->SetCollisionProfileName("Projectile");
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	RootComponent = CollisionComp;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 5000.f;
	ProjectileMovement->MaxSpeed = 5000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.1f; // Slight gravity for realism in corridors
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	SetLifeSpan(Lifetime);
}

void AProjectileBase::InitProjectile(const FWeaponStats& WeaponStats, AActor* Shooter, float InDamageMultiplier)
{
	OwnerShooter = Shooter;
	Damage = WeaponStats.GetEffectiveDamage();
	DamageType = WeaponStats.GetDamageType();
	DamageMultiplier = InDamageMultiplier;

	ProjectileMovement->InitialSpeed = WeaponStats.ProjectileSpeed;
	ProjectileMovement->MaxSpeed = WeaponStats.ProjectileSpeed;

	// Thrown weapons have more gravity
	if (WeaponStats.Category == EWeaponCategory::Thrown)
	{
		ProjectileMovement->ProjectileGravityScale = 0.5f;
	}

	// Ignore the shooter
	if (Shooter)
	{
		CollisionComp->MoveIgnoreActors.Add(Shooter);
	}
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasHit && !bCanPierce) return;
	if (OtherActor == OwnerShooter) return;

	FHitResult_Combat CombatResult;
	CombatResult.DamageType = DamageType;
	CombatResult.AttackDirection = EAttackDirection::Mid;

	if (OtherActor)
	{
		UCombatComponent* TargetCombat = OtherActor->FindComponentByClass<UCombatComponent>();
		if (TargetCombat && TargetCombat->IsAlive())
		{
			float FinalDamage = Damage * DamageMultiplier;

			// Check for headshot (hit in upper 30% of target bounds)
			FVector TargetLocation = OtherActor->GetActorLocation();
			float TargetHeight = 180.f; // Approximate character height
			float HitHeight = Hit.ImpactPoint.Z - (TargetLocation.Z - TargetHeight * 0.5f);
			if (HitHeight > TargetHeight * 0.7f)
			{
				FinalDamage *= HeadshotMultiplier;
				CombatResult.bCritical = true;
				CombatResult.AttackDirection = EAttackDirection::High;
			}

			FHitResult_Combat TargetResult = TargetCombat->ReceiveAttack(
				FinalDamage, CombatResult.AttackDirection, DamageType, OwnerShooter);

			CombatResult.bHit = true;
			CombatResult.bBlocked = TargetResult.bBlocked;
			CombatResult.bDodged = TargetResult.bDodged;
			CombatResult.DamageDealt = TargetResult.DamageDealt;
		}
	}

	OnProjectileHit.Broadcast(OtherActor, CombatResult);

	// Notify shooter's combat component
	if (OwnerShooter)
	{
		UCombatComponent* ShooterCombat = OwnerShooter->FindComponentByClass<UCombatComponent>();
		if (ShooterCombat && CombatResult.bHit)
		{
			ShooterCombat->OnHitLanded.Broadcast(CombatResult);
		}
	}

	bHasHit = true;
	if (!bCanPierce)
	{
		Destroy();
	}
}
