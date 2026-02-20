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
// Base projectile actor for ranged combat. Used by crossbows, thrown objects,
// and improvised firearms. Projectiles travel in tight train corridors, so
// they ricochet off walls and have limited effective range.
// ============================================================================

UCLASS()
class TRAINGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	/** Initialize projectile with damage, type, and the actor who fired it */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitProjectile(float InDamage, EDamageType InDamageType, EProjectileType InProjectileType, AActor* InInstigator);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EDamageType DamageType = EDamageType::Piercing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EProjectileType ProjectileType = EProjectileType::Bolt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Speed = 3000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float GravityScale = 0.3f;

	/** Max lifetime before auto-destroy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float LifeSpan = 5.f;

	/** Can this projectile ricochet off walls? (corridor bouncing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanRicochet = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bCanRicochet"))
	int32 MaxRicochets = 1;

	/** Does this projectile apply knockback? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float KnockbackForce = 500.f;

private:
	UPROPERTY()
	AActor* ProjectileInstigator = nullptr;

	int32 RicochetCount = 0;
	bool bHasHit = false;

	void ApplyDamageToTarget(AActor* Target);
};
