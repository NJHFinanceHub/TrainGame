// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainGame/Core/CombatTypes.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

// ============================================================================
// AProjectileBase
//
// Base projectile actor for ranged combat. Used by crossbow bolts, thrown
// objects (knives, bottles), and improvised firearms. Spawned by
// CombatComponent::PerformRangedAttack(), flies forward, deals damage on hit.
// In tight train corridors, projectiles are deadly but ricochet is possible.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHit, AActor*, HitActor, const FHitResult_Combat&, HitResult);

UCLASS()
class TRAINGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	/** Initialize projectile with weapon stats and shooter */
	void InitProjectile(const FWeaponStats& WeaponStats, AActor* Shooter, float DamageMultiplier = 1.f);

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FOnProjectileHit OnProjectileHit;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	/** Damage this projectile deals on impact */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	float Damage = 0.f;

	/** Damage type of this projectile */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	EDamageType DamageType = EDamageType::Piercing;

	/** Lifetime before auto-destroy (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Lifetime = 3.f;

	/** Can this projectile pierce through the first target? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanPierce = false;

	/** Headshot multiplier when hitting from specific angles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float HeadshotMultiplier = 2.f;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY()
	AActor* OwnerShooter = nullptr;

	float DamageMultiplier = 1.f;
	bool bHasHit = false;
};
