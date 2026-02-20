// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "RangedCombatComponent.h"
#include "ProjectileBase.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

URangedCombatComponent::URangedCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URangedCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void URangedCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FireCooldownTimer > 0.f)
	{
		FireCooldownTimer -= DeltaTime;
	}

	if (bIsReloading)
	{
		ReloadTimer -= DeltaTime;
		if (ReloadTimer <= 0.f)
		{
			CompleteReload();
		}
	}
}

bool URangedCombatComponent::FireRangedWeapon(FVector AimDirection)
{
	if (!CanFire()) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UWeaponComponent* WeaponComp = Owner->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComp || !WeaponComp->HasWeaponEquipped()) return false;

	const FWeaponStats& Weapon = WeaponComp->GetCurrentWeapon();
	if (!Weapon.IsRanged()) return false;
	if (!Weapon.HasAmmo())
	{
		OnAmmoEmpty.Broadcast();
		return false;
	}

	// Check stamina
	UCombatComponent* CombatComp = Owner->FindComponentByClass<UCombatComponent>();
	if (CombatComp && CombatComp->GetCurrentStamina() < StaminaCostPerShot)
	{
		return false;
	}

	// Apply accuracy spread
	AimDirection.Normalize();
	if (AccuracySpreadDegrees > 0.f)
	{
		float SpreadRad = FMath::DegreesToRadians(AccuracySpreadDegrees);
		FVector SpreadOffset = FMath::VRand() * FMath::Tan(SpreadRad);
		AimDirection = (AimDirection + SpreadOffset * 0.1f).GetSafeNormal();
	}

	// Spawn projectile
	UWorld* World = GetWorld();
	if (!World) return false;

	FVector SpawnLoc = Owner->GetActorLocation() + Owner->GetActorTransform().TransformVector(MuzzleOffset);
	FRotator SpawnRot = AimDirection.Rotation();

	AProjectileBase* Projectile = nullptr;
	if (ProjectileClass)
	{
		Projectile = World->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLoc, SpawnRot);
	}
	else
	{
		Projectile = World->SpawnActor<AProjectileBase>(AProjectileBase::StaticClass(), SpawnLoc, SpawnRot);
	}

	if (Projectile)
	{
		EDamageType DmgType = EDamageType::Piercing;
		if (Weapon.Category == EWeaponCategory::Thrown) DmgType = EDamageType::Physical;

		Projectile->InitProjectile(Weapon.GetEffectiveDamage(), DmgType, Weapon.ProjectileType, Owner);

		// Consume ammo
		WeaponComp->ApplyHitDurabilityLoss();

		// Consume stamina
		if (CombatComp)
		{
			// Stamina is consumed via the existing combat system
		}

		// Set fire cooldown based on weapon attack speed
		FireCooldownTimer = 1.f / FMath::Max(0.1f, Weapon.AttackSpeed);

		OnProjectileFired.Broadcast(Weapon.ProjectileType);
		return true;
	}

	return false;
}

bool URangedCombatComponent::ThrowObject(FVector ThrowDirection, float ThrowForce)
{
	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UWorld* World = GetWorld();
	if (!World) return false;

	FVector SpawnLoc = Owner->GetActorLocation() + Owner->GetActorTransform().TransformVector(MuzzleOffset);
	FRotator SpawnRot = ThrowDirection.Rotation();

	AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(AProjectileBase::StaticClass(), SpawnLoc, SpawnRot);
	if (Projectile)
	{
		// Thrown objects: lower damage, higher gravity, can ricochet in corridors
		Projectile->InitProjectile(15.f, EDamageType::Physical, EProjectileType::ThrownObject, Owner);
		return true;
	}

	return false;
}

void URangedCombatComponent::StartReload()
{
	if (bIsReloading) return;

	UWeaponComponent* WeaponComp = GetOwner() ? GetOwner()->FindComponentByClass<UWeaponComponent>() : nullptr;
	if (!WeaponComp || !WeaponComp->HasWeaponEquipped()) return;

	bIsReloading = true;
	ReloadTimer = ReloadTime;
}

void URangedCombatComponent::CancelReload()
{
	bIsReloading = false;
	ReloadTimer = 0.f;
}

void URangedCombatComponent::CompleteReload()
{
	bIsReloading = false;
	ReloadTimer = 0.f;
	OnReloadComplete.Broadcast();
}

bool URangedCombatComponent::CanFire() const
{
	if (bIsReloading) return false;
	if (FireCooldownTimer > 0.f) return false;

	AActor* Owner = GetOwner();
	if (!Owner) return false;

	UCombatComponent* CombatComp = Owner->FindComponentByClass<UCombatComponent>();
	if (CombatComp)
	{
		if (CombatComp->IsStaggered() || !CombatComp->IsAlive()) return false;
	}

	return true;
}

int32 URangedCombatComponent::GetCurrentAmmo() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return 0;

	UWeaponComponent* WeaponComp = Owner->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComp || !WeaponComp->HasWeaponEquipped()) return 0;

	return WeaponComp->GetCurrentWeapon().AmmoCount;
}
