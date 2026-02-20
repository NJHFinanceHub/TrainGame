#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"

USEECombatComponent::USEECombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
}

void USEECombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    // Stamina regeneration
    TimeSinceLastStaminaUse += DeltaTime;
    if (TimeSinceLastStaminaUse >= StaminaRegenDelay && CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }

    // Combo timeout
    if (CurrentComboCount > 0)
    {
        TimeSinceLastComboHit += DeltaTime;
        if (TimeSinceLastComboHit >= ComboWindowSeconds)
        {
            ResetCombo();
        }
    }

    // Dodge cooldown
    if (DodgeCooldownRemaining > 0.0f)
    {
        DodgeCooldownRemaining = FMath::Max(0.0f, DodgeCooldownRemaining - DeltaTime);
    }
}

void USEECombatComponent::MeleeAttack(ESEEAttackDirection Direction)
{
    if (bIsDead) return;
    if (CurrentStance == ESEECombatStance::Blocking) return;

    float StaminaCost = ActiveWeapon ? ActiveWeapon->StaminaCostPerAttack : 15.0f;
    if (!HasStamina(StaminaCost)) return;

    ConsumeStamina(StaminaCost);
    SetStance(ESEECombatStance::Melee);

    if (ActiveWeapon)
    {
        ActiveWeapon->ApplyDegradation();
    }

    AdvanceCombo();
}

void USEECombatComponent::StartBlock()
{
    if (bIsDead) return;
    SetStance(ESEECombatStance::Blocking);
}

void USEECombatComponent::StopBlock()
{
    if (CurrentStance == ESEECombatStance::Blocking)
    {
        SetStance(ESEECombatStance::None);
    }
}

void USEECombatComponent::Dodge()
{
    if (bIsDead) return;
    if (DodgeCooldownRemaining > 0.0f) return;
    if (!HasStamina(DodgeStaminaCost)) return;

    ConsumeStamina(DodgeStaminaCost);
    DodgeCooldownRemaining = DodgeCooldown;
    SetStance(ESEECombatStance::Dodging);
}

void USEECombatComponent::TakeDamage(FSEEDamageInfo DamageInfo)
{
    if (bIsDead) return;

    float Damage = DamageInfo.BaseDamage;

    // Apply blocking
    if (CurrentStance == ESEECombatStance::Blocking && HasStamina(BlockStaminaCostPerHit))
    {
        Damage *= (1.0f - BlockDamageReduction);
        ConsumeStamina(BlockStaminaCostPerHit);
        DamageInfo.bWasBlocked = true;
    }

    // Apply armor reduction
    Damage = ApplyArmor(Damage, DamageInfo.DamageType);

    DamageInfo.FinalDamage = FMath::Max(0.0f, Damage);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageInfo.FinalDamage);

    OnDamageTaken.Broadcast(DamageInfo);

    if (CurrentHealth <= 0.0f && !bIsDead)
    {
        bIsDead = true;
        OnDeath.Broadcast();
    }
}

void USEECombatComponent::EquipWeapon(ASEEWeaponBase* Weapon, int32 SlotIndex)
{
    if (SlotIndex == 0)
    {
        PrimaryWeapon = Weapon;
        if (!ActiveWeapon)
        {
            ActiveWeapon = Weapon;
        }
    }
    else
    {
        SecondaryWeapon = Weapon;
    }

    OnWeaponEquipped.Broadcast(Weapon, SlotIndex);
}

void USEECombatComponent::SwitchWeapon()
{
    if (ActiveWeapon == PrimaryWeapon && SecondaryWeapon)
    {
        ActiveWeapon = SecondaryWeapon;
    }
    else if (PrimaryWeapon)
    {
        ActiveWeapon = PrimaryWeapon;
    }
}

bool USEECombatComponent::HasStamina(float Cost) const
{
    return CurrentStamina >= Cost;
}

float USEECombatComponent::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
}

float USEECombatComponent::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f) : 0.0f;
}

void USEECombatComponent::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
    TimeSinceLastStaminaUse = 0.0f;
}

void USEECombatComponent::ResetCombo()
{
    if (CurrentComboCount > 0)
    {
        CurrentComboCount = 0;
        OnComboReset.Broadcast();
    }
}

void USEECombatComponent::AdvanceCombo()
{
    TimeSinceLastComboHit = 0.0f;
    if (CurrentComboCount < MaxComboCount)
    {
        CurrentComboCount++;
        OnComboAdvanced.Broadcast(CurrentComboCount);
    }
}

float USEECombatComponent::ApplyArmor(float RawDamage, ESEEDamageType Type) const
{
    float Damage = RawDamage;

    // Apply flat physical reduction
    if (Type == ESEEDamageType::Physical)
    {
        Damage = FMath::Max(0.0f, Damage - Armor.PhysicalReduction);
    }

    // Apply type-specific resistance
    if (const float* Resistance = Armor.TypeResistances.Find(Type))
    {
        Damage *= FMath::Clamp(*Resistance, 0.0f, 1.0f);
    }

    // Apply percentage reduction
    Damage *= (1.0f - FMath::Clamp(Armor.DamageReductionPercent, 0.0f, 1.0f));

    return FMath::Max(0.0f, Damage);
}

void USEECombatComponent::SetStance(ESEECombatStance NewStance)
{
    if (CurrentStance != NewStance)
    {
        CurrentStance = NewStance;
        OnStanceChanged.Broadcast(NewStance);
    }
}
