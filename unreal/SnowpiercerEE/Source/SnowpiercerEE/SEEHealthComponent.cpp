#include "SEEHealthComponent.h"

USEEHealthComponent::USEEHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentHealth = MaxHealth;
}

void USEEHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDead) return;

	// Downed timer — die if not revived in time
	if (bIsDowned)
	{
		DownedTimer += DeltaTime;
		if (DownedTimer >= DownedDuration)
		{
			bIsDead = true;
			OnDeath.Broadcast();
		}
		return;
	}

	// Internal bleeding drains HP
	if (HasInjury(ESEEInjuryType::InternalBleeding))
	{
		CurrentHealth = FMath::Max(0.0f, CurrentHealth - BleedingDrainRate * DeltaTime);
		OnHealthChanged.Broadcast(GetHealthPercent());
		if (CurrentHealth <= 0.0f)
		{
			bIsDowned = true;
			DownedTimer = 0.0f;
			OnDowned.Broadcast();
		}
	}
}

float USEEHealthComponent::TakeDamage(float BaseDamage, ESEEDamageType DamageType, AActor* Instigator)
{
	if (bIsDead || bIsDowned) return 0.0f;

	float ArmorReduction = GetArmorReduction(DamageType);
	float FinalDamage = BaseDamage * (1.0f - ArmorReduction);

	// Bladed ignores 50% of armor
	if (DamageType == ESEEDamageType::Bladed)
	{
		FinalDamage = BaseDamage * (1.0f - ArmorReduction * 0.5f);
	}

	FinalDamage = FMath::Max(1.0f, FinalDamage);
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - FinalDamage);

	OnDamageTaken.Broadcast(FinalDamage, DamageType, Instigator);
	OnHealthChanged.Broadcast(GetHealthPercent());

	// Chance of injury on heavy hits
	if (FinalDamage >= 25.0f)
	{
		float InjuryChance = (FinalDamage - 20.0f) / 100.0f;
		if (FMath::FRand() < InjuryChance)
		{
			switch (DamageType)
			{
			case ESEEDamageType::Blunt:
				ApplyInjury(FMath::FRand() > 0.5f ? ESEEInjuryType::BrokenArm : ESEEInjuryType::Concussion);
				break;
			case ESEEDamageType::Bladed:
			case ESEEDamageType::Piercing:
				ApplyInjury(FMath::FRand() > 0.7f ? ESEEInjuryType::InternalBleeding : ESEEInjuryType::DeepCut);
				break;
			default:
				ApplyInjury(ESEEInjuryType::LegWound);
				break;
			}
		}
	}

	if (CurrentHealth <= 0.0f)
	{
		bIsDowned = true;
		DownedTimer = 0.0f;
		OnDowned.Broadcast();
	}

	return FinalDamage;
}

void USEEHealthComponent::Heal(float Amount)
{
	if (bIsDead || bIsDowned) return;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);
	OnHealthChanged.Broadcast(GetHealthPercent());
}

void USEEHealthComponent::Revive(float HealthPercent)
{
	if (!bIsDowned || bIsDead) return;
	bIsDowned = false;
	DownedTimer = 0.0f;
	CurrentHealth = MaxHealth * FMath::Clamp(HealthPercent, 0.1f, 1.0f);
	OnRevived.Broadcast();
	OnHealthChanged.Broadcast(GetHealthPercent());
}

void USEEHealthComponent::ApplyInjury(ESEEInjuryType InjuryType, float Severity)
{
	if (HasInjury(InjuryType)) return;

	FSEEInjury Injury;
	Injury.Type = InjuryType;
	Injury.Severity = Severity;
	Injury.bRequiresMedStation = (InjuryType != ESEEInjuryType::DeepCut);
	ActiveInjuries.Add(Injury);

	OnInjuryApplied.Broadcast(InjuryType);
}

void USEEHealthComponent::HealInjury(ESEEInjuryType InjuryType)
{
	ActiveInjuries.RemoveAll([InjuryType](const FSEEInjury& Injury) { return Injury.Type == InjuryType; });
}

void USEEHealthComponent::HealAllInjuries()
{
	ActiveInjuries.Empty();
}

bool USEEHealthComponent::HasInjury(ESEEInjuryType InjuryType) const
{
	return ActiveInjuries.ContainsByPredicate([InjuryType](const FSEEInjury& Injury) { return Injury.Type == InjuryType; });
}

float USEEHealthComponent::GetMeleeDamageModifier() const
{
	float Mod = 1.0f;
	if (HasInjury(ESEEInjuryType::BrokenArm)) Mod *= 0.7f;
	return Mod;
}

float USEEHealthComponent::GetMoveSpeedModifier() const
{
	float Mod = 1.0f;
	if (HasInjury(ESEEInjuryType::LegWound)) Mod *= 0.6f;
	return Mod;
}

float USEEHealthComponent::GetArmorReduction(ESEEDamageType DamageType) const
{
	const float* ArmorPtr = ArmorValues.Find(DamageType);
	return ArmorPtr ? FMath::Clamp(*ArmorPtr, 0.0f, 0.9f) : 0.0f;
}

void USEEHealthComponent::SetArmor(float BluntArmor, float BladedArmor, float PiercingArmor)
{
	ArmorValues.FindOrAdd(ESEEDamageType::Blunt) = BluntArmor;
	ArmorValues.FindOrAdd(ESEEDamageType::Bladed) = BladedArmor;
	ArmorValues.FindOrAdd(ESEEDamageType::Piercing) = PiercingArmor;
}
