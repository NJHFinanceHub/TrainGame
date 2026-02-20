// ArmorComponent.cpp - Equipped armor management implementation
#include "ArmorComponent.h"

UArmorComponent::UArmorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// --- Equip / Unequip ---

bool UArmorComponent::EquipArmor(const FEquippedArmor& Armor)
{
	if (!Armor.IsValid()) return false;

	// Unequip existing piece in that slot
	if (EquippedArmor.Contains(Armor.Slot))
	{
		UnequipArmor(Armor.Slot);
	}

	EquippedArmor.Add(Armor.Slot, Armor);
	OnArmorEquipped.Broadcast(Armor.Slot, Armor);
	return true;
}

FEquippedArmor UArmorComponent::UnequipArmor(EArmorSlot Slot)
{
	FEquippedArmor Removed;
	if (EquippedArmor.RemoveAndCopyValue(Slot, Removed))
	{
		OnArmorUnequipped.Broadcast(Slot);
	}
	return Removed;
}

bool UArmorComponent::GetArmorInSlot(EArmorSlot Slot, FEquippedArmor& OutArmor) const
{
	const FEquippedArmor* Found = EquippedArmor.Find(Slot);
	if (Found)
	{
		OutArmor = *Found;
		return true;
	}
	return false;
}

bool UArmorComponent::HasArmorInSlot(EArmorSlot Slot) const
{
	return EquippedArmor.Contains(Slot);
}

// --- Damage & Durability ---

float UArmorComponent::ApplyHitToArmor(EArmorSlot Slot, float DurabilityLoss)
{
	FEquippedArmor* Armor = EquippedArmor.Find(Slot);
	if (!Armor || Armor->IsBroken()) return 0.0f;

	float DR = (Slot == EArmorSlot::Shield) ? Armor->BlockBonus : Armor->DamageReduction;

	Armor->CurrentDurability = FMath::Max(0.0f, Armor->CurrentDurability - DurabilityLoss);
	OnArmorDamaged.Broadcast(Slot, Armor->CurrentDurability);

	if (Armor->IsBroken())
	{
		OnArmorBroken.Broadcast(Slot);
	}

	return DR;
}

void UArmorComponent::RepairArmor(EArmorSlot Slot, float RepairAmount)
{
	FEquippedArmor* Armor = EquippedArmor.Find(Slot);
	if (!Armor) return;

	Armor->CurrentDurability = FMath::Min(Armor->CurrentDurability + RepairAmount, Armor->MaxDurability);
}

// --- Aggregate Stats ---

float UArmorComponent::GetTotalDamageReduction() const
{
	float Total = 0.0f;

	for (const auto& Pair : EquippedArmor)
	{
		// Shield DR is active-only (block bonus), not passive
		if (Pair.Key != EArmorSlot::Shield && !Pair.Value.IsBroken())
		{
			Total += Pair.Value.DamageReduction;
		}
	}

	// Cap at 75% to prevent invulnerability
	return FMath::Min(Total, 75.0f);
}

float UArmorComponent::GetShieldBlockBonus() const
{
	const FEquippedArmor* Shield = EquippedArmor.Find(EArmorSlot::Shield);
	if (Shield && !Shield->IsBroken())
	{
		return Shield->BlockBonus;
	}
	return 0.0f;
}

float UArmorComponent::GetTotalColdResistance() const
{
	float Total = 0.0f;

	for (const auto& Pair : EquippedArmor)
	{
		if (!Pair.Value.IsBroken())
		{
			Total += Pair.Value.ColdResistance;
		}
	}

	return Total;
}

EArmorNoiseLevel UArmorComponent::GetWorstNoiseLevel() const
{
	EArmorNoiseLevel Worst = EArmorNoiseLevel::Silent;

	for (const auto& Pair : EquippedArmor)
	{
		if (!Pair.Value.IsBroken() && Pair.Value.NoiseLevel > Worst)
		{
			Worst = Pair.Value.NoiseLevel;
		}
	}

	return Worst;
}

float UArmorComponent::GetNoiseMultiplier() const
{
	switch (GetWorstNoiseLevel())
	{
	case EArmorNoiseLevel::Silent:	return 0.5f;
	case EArmorNoiseLevel::Quiet:	return 0.75f;
	case EArmorNoiseLevel::Normal:	return 1.0f;
	case EArmorNoiseLevel::Loud:	return 1.5f;
	default:						return 1.0f;
	}
}

float UArmorComponent::GetTotalArmorWeight() const
{
	float Total = 0.0f;

	for (const auto& Pair : EquippedArmor)
	{
		Total += Pair.Value.Weight;
	}

	return Total;
}

bool UArmorComponent::HasHeavyArmorPenalty() const
{
	const FEquippedArmor* Torso = EquippedArmor.Find(EArmorSlot::Torso);
	return Torso && !Torso->IsBroken() && Torso->Weight > HeavyArmorThreshold;
}
