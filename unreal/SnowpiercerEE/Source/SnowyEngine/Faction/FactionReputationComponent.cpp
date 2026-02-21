// FactionReputationComponent.cpp - Implementation of the faction reputation component
#include "FactionReputationComponent.h"

UFactionReputationComponent::UFactionReputationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Only need to tick for cooldown expiry
}

void UFactionReputationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Expire event cooldowns
	TArray<FName> ExpiredCooldowns;
	for (auto& Pair : EventCooldowns)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			ExpiredCooldowns.Add(Pair.Key);
		}
	}
	for (const FName& Key : ExpiredCooldowns)
	{
		EventCooldowns.Remove(Key);
	}

	// Disable tick when no active cooldowns (re-enabled on next ModifyReputation)
	if (EventCooldowns.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}
}

void UFactionReputationComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeDefaultConfigs();

	// Start with tick disabled - only need it when cooldowns are active
	SetComponentTickEnabled(false);
}

// --- Initialization ---

void UFactionReputationComponent::InitializeDefaultConfigs()
{
	// Helper to ensure a faction has a config
	auto EnsureFaction = [this](EFaction Faction, const FString& Name, float StartRep, FName Currency)
	{
		if (!FactionConfigs.Contains(Faction))
		{
			FFactionConfig Config;
			Config.Faction = Faction;
			Config.FactionName = FText::FromString(Name);
			Config.ReputationCurrency = Currency;
			Config.StartingReputation = StartRep;
			Config.MinReputation = -100.0f;
			Config.MaxReputation = 100.0f;

			// Default thresholds (used by all factions unless overridden)
			Config.Thresholds = {
				{ EReputationLevel::Distrusted, -100.0f },
				{ EReputationLevel::Unknown,    -20.0f },
				{ EReputationLevel::Known,      1.0f },
				{ EReputationLevel::Respected,  26.0f },
				{ EReputationLevel::Champion,   51.0f },
				{ EReputationLevel::Legend,      76.0f }
			};

			FactionConfigs.Add(Faction, Config);
		}
	};

	// Initialize all 8 factions with defaults
	EnsureFaction(EFaction::Tailies,          TEXT("Tailies"),              -5.0f,  FName("Solidarity"));
	EnsureFaction(EFaction::ThirdClassUnion,  TEXT("Third Class Union"),    -50.0f, FName("WorkerCredit"));
	EnsureFaction(EFaction::Jackboots,        TEXT("Jackboots"),            -80.0f, FName("Compliance"));
	EnsureFaction(EFaction::Bureaucracy,      TEXT("The Bureaucracy"),      -60.0f, FName("Paperwork"));
	EnsureFaction(EFaction::FirstClassElite,  TEXT("First Class Elite"),    -90.0f, FName("Prestige"));
	EnsureFaction(EFaction::OrderOfTheEngine, TEXT("Order of the Engine"),  -70.0f, FName("Devotion"));
	EnsureFaction(EFaction::KronoleNetwork,   TEXT("Kronole Network"),      -40.0f, FName("Favor"));
	EnsureFaction(EFaction::TheThaw,          TEXT("The Thaw"),             -30.0f, FName("Hope"));

	// Override Tailies with full Zone 1 configuration
	FFactionConfig* TailiesConfig = FactionConfigs.Find(EFaction::Tailies);
	if (TailiesConfig)
	{
		InitializeTailiesConfig(*TailiesConfig);
	}

	// Initialize reputation values and cached levels from configs
	for (const auto& Pair : FactionConfigs)
	{
		float StartRep = Pair.Value.StartingReputation;
		ReputationValues.Add(Pair.Key, StartRep);
		CachedLevels.Add(Pair.Key, CalculateLevel(Pair.Key, StartRep));
	}

	// Activate any perks the player starts with
	for (const auto& Pair : FactionConfigs)
	{
		RefreshPerks(Pair.Key);
	}
}

void UFactionReputationComponent::InitializeTailiesConfig(FFactionConfig& Config)
{
	// Tailies: player is Tail-born, starting Neutral-Positive per lore
	Config.StartingReputation = -5.0f;

	// Tailies-specific thresholds matching the faction lore bible
	Config.Thresholds = {
		{ EReputationLevel::Distrusted, -100.0f },  // < -20: refuse trade, hostility
		{ EReputationLevel::Unknown,    -20.0f },    // -20 to 0: default, neutral
		{ EReputationLevel::Known,      1.0f },      // 1-25: basic trading, minor info
		{ EReputationLevel::Respected,  26.0f },     // 26-50: companion recruitment, Workshop
		{ EReputationLevel::Champion,   51.0f },     // 51-75: Council access, secrets
		{ EReputationLevel::Legend,     76.0f }       // 76-100: full trust, fight and die
	};

	// Tailies faction perks
	Config.Perks = {
		{
			FName("TailRemembers"),
			FText::FromString(TEXT("The Tail Remembers")),
			FText::FromString(TEXT("NPCs in Zone 2+ who hear you're from the Tail give cautious respect.")),
			EReputationLevel::Known,
			0.0f
		},
		{
			FName("Solidarity"),
			FText::FromString(TEXT("Solidarity")),
			FText::FromString(TEXT("Tailie NPCs in forward zones offer secret help.")),
			EReputationLevel::Respected,
			0.0f
		},
		{
			FName("Revolutionary"),
			FText::FromString(TEXT("Revolutionary")),
			FText::FromString(TEXT("The Tail sends supply drops as you progress. +1 care package per zone.")),
			EReputationLevel::Champion,
			1.0f  // 1 care package per zone
		},
		{
			FName("TheLongWalk"),
			FText::FromString(TEXT("The Long Walk")),
			FText::FromString(TEXT("Tailie uprising distracts Jackboots. -20% enemies in current zone.")),
			EReputationLevel::Legend,
			0.2f  // 20% enemy reduction
		}
	};
}

// --- Reputation Access ---

float UFactionReputationComponent::GetReputation(EFaction Faction) const
{
	const float* Value = ReputationValues.Find(Faction);
	return Value ? *Value : 0.0f;
}

float UFactionReputationComponent::GetReputationNormalized(EFaction Faction) const
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	if (!Config) return 0.5f;

	float Rep = GetReputation(Faction);
	float Range = Config->MaxReputation - Config->MinReputation;
	if (Range <= 0.0f) return 0.5f;

	return (Rep - Config->MinReputation) / Range;
}

EReputationLevel UFactionReputationComponent::GetReputationLevel(EFaction Faction) const
{
	const EReputationLevel* Cached = CachedLevels.Find(Faction);
	return Cached ? *Cached : EReputationLevel::Unknown;
}

EFactionDisposition UFactionReputationComponent::GetDisposition(EFaction Faction) const
{
	return DispositionFromLevel(GetReputationLevel(Faction));
}

// --- Reputation Modification ---

void UFactionReputationComponent::ModifyReputation(EFaction Faction, float Delta, FName EventTag)
{
	// Check cooldown if an event tag is provided
	if (EventTag != NAME_None && EventCooldowns.Contains(EventTag))
	{
		return;
	}

	float* Value = ReputationValues.Find(Faction);
	if (!Value) return;

	float OldValue = *Value;
	*Value = ClampReputation(Faction, *Value + Delta);

	// Register cooldown and enable tick to expire it
	if (EventTag != NAME_None)
	{
		EventCooldowns.Add(EventTag, EventCooldownSeconds);
		SetComponentTickEnabled(true);
	}

	if (!FMath::IsNearlyEqual(OldValue, *Value))
	{
		OnReputationChanged.Broadcast(Faction, OldValue, *Value);
		CheckLevelCrossings(Faction, OldValue);
		RefreshPerks(Faction);
	}
}

void UFactionReputationComponent::SetReputation(EFaction Faction, float NewValue)
{
	float* Value = ReputationValues.Find(Faction);
	if (!Value) return;

	float OldValue = *Value;
	*Value = ClampReputation(Faction, NewValue);

	if (!FMath::IsNearlyEqual(OldValue, *Value))
	{
		OnReputationChanged.Broadcast(Faction, OldValue, *Value);
		CheckLevelCrossings(Faction, OldValue);
		RefreshPerks(Faction);
	}
}

void UFactionReputationComponent::ModifyReputationMultiple(const TMap<EFaction, float>& Deltas, FName EventTag)
{
	// Check cooldown once for the whole batch
	if (EventTag != NAME_None && EventCooldowns.Contains(EventTag))
	{
		return;
	}

	// Register cooldown before processing and enable tick
	if (EventTag != NAME_None)
	{
		EventCooldowns.Add(EventTag, EventCooldownSeconds);
		SetComponentTickEnabled(true);
	}

	for (const auto& Pair : Deltas)
	{
		float* Value = ReputationValues.Find(Pair.Key);
		if (!Value) continue;

		float OldValue = *Value;
		*Value = ClampReputation(Pair.Key, *Value + Pair.Value);

		if (!FMath::IsNearlyEqual(OldValue, *Value))
		{
			OnReputationChanged.Broadcast(Pair.Key, OldValue, *Value);
			CheckLevelCrossings(Pair.Key, OldValue);
			RefreshPerks(Pair.Key);
		}
	}
}

// --- Perk System ---

bool UFactionReputationComponent::IsPerkActive(FName PerkTag) const
{
	return ActivePerks.Contains(PerkTag);
}

TArray<FFactionPerk> UFactionReputationComponent::GetActivePerks() const
{
	TArray<FFactionPerk> Result;
	for (const auto& ConfigPair : FactionConfigs)
	{
		EReputationLevel CurrentLevel = GetReputationLevel(ConfigPair.Key);
		for (const FFactionPerk& Perk : ConfigPair.Value.Perks)
		{
			if (ActivePerks.Contains(Perk.PerkTag))
			{
				Result.Add(Perk);
			}
		}
	}
	return Result;
}

TArray<FFactionPerk> UFactionReputationComponent::GetFactionPerks(EFaction Faction) const
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	if (!Config) return {};
	return Config->Perks;
}

// --- Query ---

FFactionConfig UFactionReputationComponent::GetFactionConfig(EFaction Faction) const
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	return Config ? *Config : FFactionConfig();
}

bool UFactionReputationComponent::MeetsReputationRequirement(EFaction Faction, EReputationLevel RequiredLevel) const
{
	EReputationLevel Current = GetReputationLevel(Faction);
	return static_cast<uint8>(Current) >= static_cast<uint8>(RequiredLevel);
}

TArray<EFaction> UFactionReputationComponent::GetFactionsAtLevel(EReputationLevel MinLevel) const
{
	TArray<EFaction> Result;
	for (const auto& Pair : CachedLevels)
	{
		if (static_cast<uint8>(Pair.Value) >= static_cast<uint8>(MinLevel))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

// --- Serialization ---

FFactionReputationSnapshot UFactionReputationComponent::CreateSnapshot() const
{
	FFactionReputationSnapshot Snap;
	Snap.ReputationValues = ReputationValues;
	for (const FName& PerkTag : ActivePerks)
	{
		Snap.ActivePerks.Add(PerkTag);
	}
	return Snap;
}

void UFactionReputationComponent::RestoreFromSnapshot(const FFactionReputationSnapshot& Snapshot)
{
	for (const auto& Pair : Snapshot.ReputationValues)
	{
		SetReputation(Pair.Key, Pair.Value);
	}
}

// --- Private ---

EReputationLevel UFactionReputationComponent::CalculateLevel(EFaction Faction, float RepValue) const
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	if (!Config || Config->Thresholds.Num() == 0)
	{
		return EReputationLevel::Unknown;
	}

	// Walk thresholds in reverse (highest first) to find the matching level
	EReputationLevel Result = EReputationLevel::Distrusted;
	for (const FReputationThreshold& Threshold : Config->Thresholds)
	{
		if (RepValue >= Threshold.MinReputation)
		{
			Result = Threshold.Level;
		}
	}
	return Result;
}

void UFactionReputationComponent::CheckLevelCrossings(EFaction Faction, float OldValue)
{
	EReputationLevel NewLevel = CalculateLevel(Faction, GetReputation(Faction));
	EReputationLevel* OldLevel = CachedLevels.Find(Faction);

	if (OldLevel && *OldLevel != NewLevel)
	{
		EReputationLevel PrevLevel = *OldLevel;
		*OldLevel = NewLevel;
		OnReputationLevelChanged.Broadcast(Faction, PrevLevel, NewLevel);
	}
}

void UFactionReputationComponent::RefreshPerks(EFaction Faction)
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	if (!Config) return;

	EReputationLevel CurrentLevel = GetReputationLevel(Faction);

	for (const FFactionPerk& Perk : Config->Perks)
	{
		bool bShouldBeActive = static_cast<uint8>(CurrentLevel) >= static_cast<uint8>(Perk.RequiredLevel);
		bool bIsPerkActive = ActivePerks.Contains(Perk.PerkTag);

		if (bShouldBeActive && !bIsPerkActive)
		{
			ActivePerks.Add(Perk.PerkTag);
			OnPerkUnlocked.Broadcast(Faction, Perk.PerkTag);
		}
		else if (!bShouldBeActive && bIsPerkActive)
		{
			ActivePerks.Remove(Perk.PerkTag);
			OnPerkLost.Broadcast(Faction, Perk.PerkTag);
		}
	}
}

float UFactionReputationComponent::ClampReputation(EFaction Faction, float Value) const
{
	const FFactionConfig* Config = FactionConfigs.Find(Faction);
	if (Config)
	{
		return FMath::Clamp(Value, Config->MinReputation, Config->MaxReputation);
	}
	return FMath::Clamp(Value, -100.0f, 100.0f);
}

EFactionDisposition UFactionReputationComponent::DispositionFromLevel(EReputationLevel Level) const
{
	switch (Level)
	{
	case EReputationLevel::Distrusted:	return EFactionDisposition::Hostile;
	case EReputationLevel::Unknown:		return EFactionDisposition::Unfriendly;
	case EReputationLevel::Known:		return EFactionDisposition::Neutral;
	case EReputationLevel::Respected:	return EFactionDisposition::Friendly;
	case EReputationLevel::Champion:	return EFactionDisposition::Allied;
	case EReputationLevel::Legend:		return EFactionDisposition::Allied;
	default:							return EFactionDisposition::Neutral;
	}
}
