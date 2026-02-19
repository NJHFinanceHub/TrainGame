// SurvivalComponent.cpp - Implementation of the core survival stat component
#include "SurvivalComponent.h"

USurvivalComponent::USurvivalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f; // Tick 4x per second for performance
}

void USurvivalComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDefaults();
}

void USurvivalComponent::InitializeDefaults()
{
	// Set up default configs if not configured in editor
	auto EnsureConfig = [this](ESurvivalStatType Type, float MaxVal, float DefaultVal, float Decay)
	{
		if (!StatConfigs.Contains(Type))
		{
			FSurvivalStatConfig Config;
			Config.MaxValue = MaxVal;
			Config.DefaultValue = DefaultVal;
			Config.DecayRatePerSecond = Decay;
			StatConfigs.Add(Type, Config);
		}
	};

	// Hunger decays over time - roughly 100 -> 0 in ~20 minutes of game time
	EnsureConfig(ESurvivalStatType::Hunger, 100.0f, 100.0f, 0.083f);
	// Cold doesn't decay on its own - driven by environment
	EnsureConfig(ESurvivalStatType::Cold, 100.0f, 100.0f, 0.0f);
	// Morale decays very slowly - driven mainly by events
	EnsureConfig(ESurvivalStatType::Morale, 100.0f, 50.0f, 0.01f);
	// Health doesn't decay passively
	EnsureConfig(ESurvivalStatType::Health, 100.0f, 100.0f, 0.0f);
	// Stamina regens (handled separately), doesn't decay
	EnsureConfig(ESurvivalStatType::Stamina, 100.0f, 100.0f, 0.0f);

	// Initialize current values from configs
	for (auto& Pair : StatConfigs)
	{
		CurrentStats.Add(Pair.Key, Pair.Value.DefaultValue);
		CachedThresholds.Add(Pair.Key, CalculateThreshold(Pair.Value.DefaultValue / Pair.Value.MaxValue));
	}
}

void USurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickDecay(DeltaTime);
	TickColdExposure(DeltaTime);
	TickStaminaRegen(DeltaTime);

	// Tick morale event cooldowns
	TArray<FName> ExpiredCooldowns;
	for (auto& Pair : MoraleEventCooldowns)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			ExpiredCooldowns.Add(Pair.Key);
		}
	}
	for (const FName& Key : ExpiredCooldowns)
	{
		MoraleEventCooldowns.Remove(Key);
	}
}

// --- Stat Access ---

float USurvivalComponent::GetStatValue(ESurvivalStatType StatType) const
{
	const float* Value = CurrentStats.Find(StatType);
	return Value ? *Value : 0.0f;
}

float USurvivalComponent::GetStatPercent(ESurvivalStatType StatType) const
{
	const float* Value = CurrentStats.Find(StatType);
	const FSurvivalStatConfig* Config = StatConfigs.Find(StatType);
	if (Value && Config && Config->MaxValue > 0.0f)
	{
		return *Value / Config->MaxValue;
	}
	return 0.0f;
}

ESurvivalThreshold USurvivalComponent::GetStatThreshold(ESurvivalStatType StatType) const
{
	const ESurvivalThreshold* Cached = CachedThresholds.Find(StatType);
	return Cached ? *Cached : ESurvivalThreshold::Normal;
}

// --- Stat Modification ---

void USurvivalComponent::ModifyStat(ESurvivalStatType StatType, float Delta)
{
	float* Value = CurrentStats.Find(StatType);
	if (!Value) return;

	float OldValue = *Value;
	*Value = ClampStat(StatType, *Value + Delta);

	if (!FMath::IsNearlyEqual(OldValue, *Value))
	{
		OnSurvivalStatChanged.Broadcast(StatType, OldValue, *Value);
		CheckThresholdCrossings(StatType, OldValue);
	}
}

void USurvivalComponent::SetStat(ESurvivalStatType StatType, float NewValue)
{
	float* Value = CurrentStats.Find(StatType);
	if (!Value) return;

	float OldValue = *Value;
	*Value = ClampStat(StatType, NewValue);

	if (!FMath::IsNearlyEqual(OldValue, *Value))
	{
		OnSurvivalStatChanged.Broadcast(StatType, OldValue, *Value);
		CheckThresholdCrossings(StatType, OldValue);
	}
}

// --- Derived Modifiers ---

float USurvivalComponent::GetStaminaRegenMultiplier() const
{
	float Combined = 1.0f;
	for (const auto& Pair : StatConfigs)
	{
		ESurvivalThreshold Threshold = GetStatThreshold(Pair.Key);
		Combined *= GetMultiplierForThreshold(Pair.Value.StaminaRegenMultipliers, Threshold);
	}
	return Combined;
}

float USurvivalComponent::GetCombatEffectivenessMultiplier() const
{
	float Combined = 1.0f;
	for (const auto& Pair : StatConfigs)
	{
		// Stamina doesn't affect combat effectiveness directly via this path
		if (Pair.Key == ESurvivalStatType::Stamina) continue;

		ESurvivalThreshold Threshold = GetStatThreshold(Pair.Key);
		Combined *= GetMultiplierForThreshold(Pair.Value.CombatEffectivenessMultipliers, Threshold);
	}
	return Combined;
}

// --- Cold System ---

void USurvivalComponent::SetEnvironmentColdLevel(float ColdLevel)
{
	EnvironmentColdLevel = FMath::Clamp(ColdLevel, 0.0f, 1.0f);
}

bool USurvivalComponent::HasColdProtection() const
{
	return bHasColdProtection;
}

void USurvivalComponent::SetColdProtection(bool bProtected)
{
	bHasColdProtection = bProtected;
}

// --- Morale System ---

void USurvivalComponent::ApplyMoraleEvent(float MoraleDelta, FName EventTag)
{
	// Check cooldown to prevent morale event spam
	if (MoraleEventCooldowns.Contains(EventTag))
	{
		return;
	}

	ModifyStat(ESurvivalStatType::Morale, MoraleDelta);
	MoraleEventCooldowns.Add(EventTag, MoraleEventCooldownSeconds);
}

// --- Serialization ---

FSurvivalSnapshot USurvivalComponent::CreateSnapshot() const
{
	FSurvivalSnapshot Snap;
	Snap.Hunger = GetStatValue(ESurvivalStatType::Hunger);
	Snap.Cold = GetStatValue(ESurvivalStatType::Cold);
	Snap.Morale = GetStatValue(ESurvivalStatType::Morale);
	Snap.Health = GetStatValue(ESurvivalStatType::Health);
	Snap.Stamina = GetStatValue(ESurvivalStatType::Stamina);
	return Snap;
}

void USurvivalComponent::RestoreFromSnapshot(const FSurvivalSnapshot& Snapshot)
{
	SetStat(ESurvivalStatType::Hunger, Snapshot.Hunger);
	SetStat(ESurvivalStatType::Cold, Snapshot.Cold);
	SetStat(ESurvivalStatType::Morale, Snapshot.Morale);
	SetStat(ESurvivalStatType::Health, Snapshot.Health);
	SetStat(ESurvivalStatType::Stamina, Snapshot.Stamina);
}

// --- Private ---

void USurvivalComponent::TickDecay(float DeltaTime)
{
	for (const auto& Pair : StatConfigs)
	{
		if (Pair.Value.DecayRatePerSecond > 0.0f)
		{
			ModifyStat(Pair.Key, -Pair.Value.DecayRatePerSecond * DeltaTime);
		}
	}
}

void USurvivalComponent::TickColdExposure(float DeltaTime)
{
	if (EnvironmentColdLevel <= 0.0f) return;

	float EffectiveColdRate = ColdDrainRate * EnvironmentColdLevel;

	// Cold protection reduces cold drain by 80%
	if (bHasColdProtection)
	{
		EffectiveColdRate *= 0.2f;
	}

	ModifyStat(ESurvivalStatType::Cold, -EffectiveColdRate * DeltaTime);

	// When Cold is critically low, start draining Health
	if (GetStatThreshold(ESurvivalStatType::Cold) == ESurvivalThreshold::Critical)
	{
		ModifyStat(ESurvivalStatType::Health, -1.0f * DeltaTime);
	}
}

void USurvivalComponent::TickStaminaRegen(float DeltaTime)
{
	float StaminaPercent = GetStatPercent(ESurvivalStatType::Stamina);
	if (StaminaPercent >= 1.0f) return;

	// Base regen: 10 per second, modified by survival state
	float BaseRegen = 10.0f;
	float Regen = BaseRegen * GetStaminaRegenMultiplier() * DeltaTime;
	ModifyStat(ESurvivalStatType::Stamina, Regen);
}

void USurvivalComponent::CheckThresholdCrossings(ESurvivalStatType StatType, float OldValue)
{
	const FSurvivalStatConfig* Config = StatConfigs.Find(StatType);
	if (!Config || Config->MaxValue <= 0.0f) return;

	float NewPercent = GetStatPercent(StatType);
	ESurvivalThreshold NewThreshold = CalculateThreshold(NewPercent);
	ESurvivalThreshold* OldThreshold = CachedThresholds.Find(StatType);

	if (OldThreshold && *OldThreshold != NewThreshold)
	{
		ESurvivalThreshold PrevThreshold = *OldThreshold;
		*OldThreshold = NewThreshold;
		OnThresholdCrossed.Broadcast(StatType, PrevThreshold, NewThreshold);
	}
}

ESurvivalThreshold USurvivalComponent::CalculateThreshold(float Percent) const
{
	if (Percent <= 0.15f) return ESurvivalThreshold::Critical;
	if (Percent <= 0.35f) return ESurvivalThreshold::Low;
	if (Percent <= 0.65f) return ESurvivalThreshold::Normal;
	if (Percent <= 0.85f) return ESurvivalThreshold::Good;
	return ESurvivalThreshold::Excellent;
}

float USurvivalComponent::GetMultiplierForThreshold(const TArray<float>& Multipliers, ESurvivalThreshold Threshold) const
{
	int32 Index = static_cast<int32>(Threshold);
	if (Multipliers.IsValidIndex(Index))
	{
		return Multipliers[Index];
	}
	return 1.0f;
}

float USurvivalComponent::ClampStat(ESurvivalStatType StatType, float Value) const
{
	const FSurvivalStatConfig* Config = StatConfigs.Find(StatType);
	if (Config)
	{
		return FMath::Clamp(Value, 0.0f, Config->MaxValue);
	}
	return FMath::Max(Value, 0.0f);
}
