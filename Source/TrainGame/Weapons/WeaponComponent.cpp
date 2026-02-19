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
// Preset Weapon Factory Methods
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
	W.DurabilityLossOnBlock = 0.f; // Ranged, no block contact
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
	W.Durability = 15.f;      // Breaks very fast
	W.MaxDurability = 15.f;
	W.DurabilityLossPerHit = 5.f;
	W.DurabilityLossOnBlock = 0.f;
	return W;
}
