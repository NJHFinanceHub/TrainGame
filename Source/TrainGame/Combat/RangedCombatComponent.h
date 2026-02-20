// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "RangedCombatComponent.generated.h"

class AProjectileBase;
class UWeaponComponent;

// ============================================================================
// URangedCombatComponent
//
// Handles ranged weapon firing, aiming, and ammo management.
// Works with crossbows, thrown objects, and improvised firearms.
// In tight train corridors, ranged combat is high-risk/high-reward:
// slow reload times leave you vulnerable but the damage is devastating.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectileFired, EProjectileType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAmmoEmpty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadComplete);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API URangedCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URangedCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Fire the currently equipped ranged weapon */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	bool FireRangedWeapon(FVector AimDirection);

	/** Throw an object at a target */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	bool ThrowObject(FVector ThrowDirection, float ThrowForce = 2000.f);

	/** Begin reload (takes time based on weapon) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	void StartReload();

	/** Cancel reload (interrupted by damage, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	void CancelReload();

	UFUNCTION(BlueprintPure, Category = "Combat|Ranged")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Combat|Ranged")
	bool CanFire() const;

	UFUNCTION(BlueprintPure, Category = "Combat|Ranged")
	int32 GetCurrentAmmo() const;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Ranged")
	FOnProjectileFired OnProjectileFired;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Ranged")
	FOnAmmoEmpty OnAmmoEmpty;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Ranged")
	FOnReloadComplete OnReloadComplete;

protected:
	virtual void BeginPlay() override;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	TSubclassOf<AProjectileBase> ProjectileClass;

	/** Spawn offset from owner (barrel/hand position) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	FVector MuzzleOffset = FVector(100.f, 0.f, 50.f);

	/** Time to reload the weapon */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	float ReloadTime = 2.f;

	/** Accuracy spread in degrees (tight corridors are forgiving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	float AccuracySpreadDegrees = 2.f;

	/** Stamina cost per shot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	float StaminaCostPerShot = 5.f;

private:
	void CompleteReload();

	bool bIsReloading = false;
	float ReloadTimer = 0.f;
	float FireCooldownTimer = 0.f;
};
