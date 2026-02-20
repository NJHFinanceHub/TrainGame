#include "SEEWeaponBase.h"

ASEEWeaponBase::ASEEWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;
    CurrentDurability = MaxDurability;
}

float ASEEWeaponBase::GetEffectiveDamage() const
{
    if (IsBroken())
    {
        return BaseDamage * BrokenDamageMultiplier;
    }

    float DurabilityRatio = GetDurabilityPercent();
    // Linear interpolation: full damage at 100%, BrokenDamageMultiplier at 0%
    float DegradationFactor = FMath::Lerp(BrokenDamageMultiplier, 1.0f, DurabilityRatio);
    return BaseDamage * DegradationFactor;
}

float ASEEWeaponBase::GetDurabilityPercent() const
{
    return (MaxDurability > 0.0f) ? FMath::Clamp(CurrentDurability / MaxDurability, 0.0f, 1.0f) : 0.0f;
}

void ASEEWeaponBase::ApplyDegradation(float Multiplier)
{
    float OldDurability = CurrentDurability;
    CurrentDurability = FMath::Max(0.0f, CurrentDurability - DegradationPerHit * Multiplier);

    if (CurrentDurability != OldDurability)
    {
        OnDurabilityChanged.Broadcast(CurrentDurability);
    }

    if (CurrentDurability <= 0.0f && OldDurability > 0.0f)
    {
        OnWeaponBroken.Broadcast();
    }
}

void ASEEWeaponBase::Repair(float Amount)
{
    CurrentDurability = FMath::Min(MaxDurability, CurrentDurability + Amount);
    OnDurabilityChanged.Broadcast(CurrentDurability);
}

bool ASEEWeaponBase::IsBroken() const
{
    return CurrentDurability <= 0.0f;
}

bool ASEEWeaponBase::IsMelee() const
{
    return Category == ESEEWeaponCategory::Melee_Light || Category == ESEEWeaponCategory::Melee_Heavy;
}

bool ASEEWeaponBase::IsRanged() const
{
    return Category == ESEEWeaponCategory::Ranged_Pistol
        || Category == ESEEWeaponCategory::Ranged_Rifle
        || Category == ESEEWeaponCategory::Ranged_Thrown;
}
