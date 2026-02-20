// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "WeaponComponent.h"

UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::EquipWeapon(const FWeaponStats& NewWeapon)
{
	CurrentWeapon = NewWeapon;
	bHasWeapon = true;
	OnWeaponEquipped.Broadcast(CurrentWeapon);
	OnDurabilityChanged.Broadcast(CurrentWeapon.Durability, CurrentWeapon.MaxDurability);

	if (CurrentWeapon.MaxAmmo > 0)
	{
		OnAmmoChanged.Broadcast(CurrentWeapon.CurrentAmmo, CurrentWeapon.MaxAmmo);
	}
}

void UWeaponComponent::UnequipWeapon()
{
	CurrentWeapon = FWeaponStats();
	bHasWeapon = false;
}

void UWeaponComponent::ApplyHitDurabilityLoss()
{
	if (!bHasWeapon) return;

	CurrentWeapon.Durability = FMath::Max(0.f, CurrentWeapon.Durability - CurrentWeapon.DurabilityLossPerHit);
	OnDurabilityChanged.Broadcast(CurrentWeapon.Durability, CurrentWeapon.MaxDurability);

	if (CurrentWeapon.IsBroken())
	{
		HandleWeaponBreak();
	}
}

void UWeaponComponent::ApplyBlockDurabilityLoss()
{
	if (!bHasWeapon) return;

	CurrentWeapon.Durability = FMath::Max(0.f, CurrentWeapon.Durability - CurrentWeapon.DurabilityLossOnBlock);
	OnDurabilityChanged.Broadcast(CurrentWeapon.Durability, CurrentWeapon.MaxDurability);

	if (CurrentWeapon.IsBroken())
	{
		HandleWeaponBreak();
	}
}

void UWeaponComponent::RepairWeapon(float RepairAmount)
{
	if (!bHasWeapon) return;

	CurrentWeapon.Durability = FMath::Min(CurrentWeapon.MaxDurability,
		CurrentWeapon.Durability + RepairAmount);
	OnDurabilityChanged.Broadcast(CurrentWeapon.Durability, CurrentWeapon.MaxDurability);
}

bool UWeaponComponent::ConsumeAmmo()
{
	if (!bHasWeapon) return false;
	if (CurrentWeapon.CurrentAmmo == -1) return true; // Melee, no ammo needed
	if (CurrentWeapon.CurrentAmmo <= 0)
	{
		OnAmmoEmpty.Broadcast();
		return false;
	}

	CurrentWeapon.CurrentAmmo--;
	OnAmmoChanged.Broadcast(CurrentWeapon.CurrentAmmo, CurrentWeapon.MaxAmmo);

	if (CurrentWeapon.CurrentAmmo <= 0)
	{
		OnAmmoEmpty.Broadcast();
	}

	return true;
}

void UWeaponComponent::AddAmmo(int32 Amount)
{
	if (!bHasWeapon || CurrentWeapon.MaxAmmo <= 0) return;

	CurrentWeapon.CurrentAmmo = FMath::Min(CurrentWeapon.CurrentAmmo + Amount, CurrentWeapon.MaxAmmo);
	OnAmmoChanged.Broadcast(CurrentWeapon.CurrentAmmo, CurrentWeapon.MaxAmmo);
}

float UWeaponComponent::GetDurabilityPercent() const
{
	if (!bHasWeapon || CurrentWeapon.MaxDurability <= 0.f) return 0.f;
	return CurrentWeapon.Durability / CurrentWeapon.MaxDurability;
}

void UWeaponComponent::HandleWeaponBreak()
{
	OnWeaponBroken.Broadcast();
	// Weapon remains equipped but broken - player can still use it at reduced effectiveness
	// or switch to unarmed. Broken weapons deal minimal damage.
}

// ============================================================================
// Preset Weapon Factory Methods — Tail (Improvised)
// ============================================================================

FWeaponStats UWeaponComponent::MakePipeClub()
{
	FWeaponStats W;
	W.WeaponName = FName("Pipe Club");
	W.Category = EWeaponCategory::Blunt;
	W.Tier = EWeaponTier::Improvised;
	W.BaseDamage = 12.f;
	W.AttackSpeed = 0.9f;
	W.Range = 160.f;
	W.StaminaCostPerSwing = 14.f;
	W.Durability = 40.f;
	W.MaxDurability = 40.f;
	W.DurabilityLossPerHit = 3.f;
	W.DurabilityLossOnBlock = 6.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeShiv()
{
	FWeaponStats W;
	W.WeaponName = FName("Shiv");
	W.Category = EWeaponCategory::Bladed;
	W.Tier = EWeaponTier::Improvised;
	W.BaseDamage = 15.f;
	W.AttackSpeed = 1.4f;
	W.Range = 100.f;
	W.StaminaCostPerSwing = 8.f;
	W.Durability = 25.f;
	W.MaxDurability = 25.f;
	W.DurabilityLossPerHit = 4.f;
	W.DurabilityLossOnBlock = 8.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeNailBat()
{
	FWeaponStats W;
	W.WeaponName = FName("Nail Bat");
	W.Category = EWeaponCategory::Blunt;
	W.Tier = EWeaponTier::Improvised;
	W.BaseDamage = 16.f;
	W.AttackSpeed = 0.8f;
	W.Range = 180.f;
	W.StaminaCostPerSwing = 16.f;
	W.Durability = 35.f;
	W.MaxDurability = 35.f;
	W.DurabilityLossPerHit = 3.f;
	W.DurabilityLossOnBlock = 7.f;
	return W;
}

// ============================================================================
// Third Class (Functional)
// ============================================================================

FWeaponStats UWeaponComponent::MakeReinforcedAxe()
{
	FWeaponStats W;
	W.WeaponName = FName("Reinforced Axe");
	W.Category = EWeaponCategory::Bladed;
	W.Tier = EWeaponTier::Functional;
	W.BaseDamage = 22.f;
	W.AttackSpeed = 0.7f;
	W.Range = 170.f;
	W.StaminaCostPerSwing = 20.f;
	W.Durability = 80.f;
	W.MaxDurability = 80.f;
	W.DurabilityLossPerHit = 2.f;
	W.DurabilityLossOnBlock = 4.f;
	return W;
}

// ============================================================================
// Military (Jackboot)
// ============================================================================

FWeaponStats UWeaponComponent::MakeJackbootBaton()
{
	FWeaponStats W;
	W.WeaponName = FName("Jackboot Shock Baton");
	W.Category = EWeaponCategory::Blunt;
	W.Tier = EWeaponTier::Military;
	W.BaseDamage = 18.f;
	W.AttackSpeed = 1.1f;
	W.Range = 150.f;
	W.StaminaCostPerSwing = 12.f;
	W.Durability = 120.f;
	W.MaxDurability = 120.f;
	W.DurabilityLossPerHit = 1.f;
	W.DurabilityLossOnBlock = 2.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeOfficerSword()
{
	FWeaponStats W;
	W.WeaponName = FName("Officer's Sword");
	W.Category = EWeaponCategory::Bladed;
	W.Tier = EWeaponTier::Military;
	W.BaseDamage = 24.f;
	W.AttackSpeed = 1.0f;
	W.Range = 180.f;
	W.StaminaCostPerSwing = 16.f;
	W.Durability = 100.f;
	W.MaxDurability = 100.f;
	W.DurabilityLossPerHit = 1.5f;
	W.DurabilityLossOnBlock = 3.f;
	return W;
}

// ============================================================================
// Ranged Weapons
// ============================================================================

FWeaponStats UWeaponComponent::MakeCrossbow()
{
	FWeaponStats W;
	W.WeaponName = FName("Crossbow");
	W.Category = EWeaponCategory::Ranged;
	W.Tier = EWeaponTier::Functional;
	W.BaseDamage = 35.f;
	W.AttackSpeed = 0.3f; // Slow reload
	W.Range = 2000.f;
	W.StaminaCostPerSwing = 5.f;
	W.Durability = 60.f;
	W.MaxDurability = 60.f;
	W.DurabilityLossPerHit = 1.f;
	W.DurabilityLossOnBlock = 0.f;
	W.CurrentAmmo = 8;
	W.MaxAmmo = 8;
	W.ProjectileSpeed = 6000.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeImprovisedFirearm()
{
	FWeaponStats W;
	W.WeaponName = FName("Pipe Gun");
	W.Category = EWeaponCategory::Ranged;
	W.Tier = EWeaponTier::Improvised;
	W.BaseDamage = 45.f;
	W.AttackSpeed = 0.2f; // Very slow reload
	W.Range = 1500.f;
	W.StaminaCostPerSwing = 3.f;
	W.Durability = 15.f;
	W.MaxDurability = 15.f;
	W.DurabilityLossPerHit = 5.f;
	W.DurabilityLossOnBlock = 0.f;
	W.CurrentAmmo = 3;
	W.MaxAmmo = 3;
	W.ProjectileSpeed = 8000.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeFirstClassPistol()
{
	FWeaponStats W;
	W.WeaponName = FName("First Class Pistol");
	W.Category = EWeaponCategory::Ranged;
	W.Tier = EWeaponTier::Specialized;
	W.BaseDamage = 30.f;
	W.AttackSpeed = 0.6f; // Faster than crossbow
	W.Range = 2500.f;
	W.StaminaCostPerSwing = 4.f;
	W.Durability = 150.f;
	W.MaxDurability = 150.f;
	W.DurabilityLossPerHit = 0.5f;
	W.DurabilityLossOnBlock = 0.f;
	W.CurrentAmmo = 12;
	W.MaxAmmo = 12;
	W.ProjectileSpeed = 10000.f;
	return W;
}

// ============================================================================
// Thrown Weapons
// ============================================================================

FWeaponStats UWeaponComponent::MakeThrowingKnife()
{
	FWeaponStats W;
	W.WeaponName = FName("Throwing Knife");
	W.Category = EWeaponCategory::Thrown;
	W.Tier = EWeaponTier::Functional;
	W.BaseDamage = 25.f;
	W.AttackSpeed = 1.2f; // Quick throw
	W.Range = 1200.f;
	W.StaminaCostPerSwing = 8.f;
	W.Durability = 30.f;
	W.MaxDurability = 30.f;
	W.DurabilityLossPerHit = 10.f; // Degrades fast on impact
	W.DurabilityLossOnBlock = 0.f;
	W.CurrentAmmo = 3;
	W.MaxAmmo = 3;
	W.ProjectileSpeed = 4000.f;
	return W;
}

// ============================================================================
// Specialized (Enemy-Unique)
// ============================================================================

FWeaponStats UWeaponComponent::MakeZealotBlade()
{
	FWeaponStats W;
	W.WeaponName = FName("Zealot Consecrated Blade");
	W.Category = EWeaponCategory::Bladed;
	W.Tier = EWeaponTier::Specialized;
	W.BaseDamage = 28.f;
	W.AttackSpeed = 1.3f; // Fast, frenzied slashes
	W.Range = 140.f;
	W.StaminaCostPerSwing = 10.f;
	W.Durability = 70.f;
	W.MaxDurability = 70.f;
	W.DurabilityLossPerHit = 2.f;
	W.DurabilityLossOnBlock = 5.f;
	return W;
}

FWeaponStats UWeaponComponent::MakeElectricProd()
{
	FWeaponStats W;
	W.WeaponName = FName("Electric Prod");
	W.Category = EWeaponCategory::Piercing;
	W.Tier = EWeaponTier::Specialized;
	W.BaseDamage = 20.f;
	W.AttackSpeed = 1.0f;
	W.Range = 160.f;
	W.StaminaCostPerSwing = 14.f;
	W.Durability = 200.f; // Very durable, engineered
	W.MaxDurability = 200.f;
	W.DurabilityLossPerHit = 0.5f;
	W.DurabilityLossOnBlock = 1.f;
	return W;
}
