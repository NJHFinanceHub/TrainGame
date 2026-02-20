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
// Base projectile actor for ranged combat. Crossbow bolts, thrown objects,
// improvised bullets. Handles velocity, gravity, collision, and damage
// application on hit. Train corridors make ranged combat deadly at close
// range but limited by tight spaces and ricochets.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProjectileHit, AActor*, HitActor, const FHitResult&, HitResult);

UCLASS()
class TRAINGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	/** Initialize projectile with direction and owner */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void FireInDirection(FVector Direction, AActor* ProjectileOwner);

	UPROPERTY(BlueprintAssignable, Category = "Projectile")
	FOnProjectileHit OnProjectileHit;

protected:
	virtual void BeginPlay() override;

	/** Collision sphere */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	USphereComponent* CollisionComp;

	/** Projectile movement */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	/** What type of projectile this is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EProjectileType ProjectileType = EProjectileType::CrossbowBolt;

	/** Damage dealt on direct hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float Damage = 35.f;

	/** Damage type applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	EDamageType DamageType = EDamageType::Piercing;

	/** Initial speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float InitialSpeed = 3000.f;

	/** Max speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float MaxSpeed = 3000.f;

	/** Gravity scale (0 = no gravity, 1 = normal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float GravityScale = 0.3f;

	/** How long the projectile lives before auto-destroy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float LifeSpan = 5.f;

	/** Can this projectile penetrate and hit multiple targets? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanPenetrate = false;

	/** Can this projectile ricochet off walls in tight corridors? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bCanRicochet = false;

	/** Max ricochets before destroying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bCanRicochet"))
	int32 MaxRicochets = 1;

	/** Damage multiplier on ricochet hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bCanRicochet"))
	float RicochetDamageMultiplier = 0.5f;

	/** Is this a non-lethal projectile (tranq dart)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bNonLethal = false;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	void ApplyDamageToTarget(AActor* Target, float DamageAmount);

	UPROPERTY()
	AActor* OwnerActor = nullptr;

	int32 RicochetCount = 0;
	float CurrentDamage = 0.f;
};
