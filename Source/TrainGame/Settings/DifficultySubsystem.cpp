// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DifficultySubsystem.h"

USEEDifficultySubsystem::USEEDifficultySubsystem()
{
}

void USEEDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDefaultTiers();
}

// ============================================================================
// Default Tier Configurations
// ============================================================================

void USEEDifficultySubsystem::InitializeDefaultTiers()
{
	// --- Passenger ---
	{
		FDifficultyTierConfig Config;
		Config.Tier = EDifficultyTier::Passenger;
		Config.DisplayName = NSLOCTEXT("Difficulty", "Passenger", "Passenger");
		Config.Description = NSLOCTEXT("Difficulty", "PassengerDesc",
			"You're here for the story. The train is dangerous, but forgiving.");

		FDifficultyModifiers& M = Config.Modifiers;
		M.EnemyHealthMultiplier = 0.7f;
		M.EnemyDamageMultiplier = 0.6f;
		M.EnemyDetectionRangeMultiplier = 0.75f;
		M.EnemyDetectionSpeedMultiplier = 0.7f;
		M.ResourceDropRateMultiplier = 1.5f;
		M.ResourceDegradationMultiplier = 0.5f;
		M.ColdExposureTimeMultiplier = 1.5f;
		M.StaminaDrainMultiplier = 0.75f;
		M.XPGainMultiplier = 1.25f;
		M.CompanionDownTimerSeconds = 60.f;
		M.FallDamageMultiplier = 0.5f;
		M.InjurySeverityShift = -1;
		M.StatCheckModifier = 3;
		M.DeathResourceLossFraction = 0.f;
		M.bCompanionPermadeath = false;
		M.bPermadeath = false;
		M.bFriendlyFire = false;
		M.bShowQuestMarkers = true;
		M.bRestrictedSaving = false;

		TierConfigs.Add(EDifficultyTier::Passenger, Config);
	}

	// --- Survivor ---
	{
		FDifficultyTierConfig Config;
		Config.Tier = EDifficultyTier::Survivor;
		Config.DisplayName = NSLOCTEXT("Difficulty", "Survivor", "Survivor");
		Config.Description = NSLOCTEXT("Difficulty", "SurvivorDesc",
			"The intended experience. The train tests you, but fair play is rewarded.");

		// All modifiers at baseline (1.0x)
		FDifficultyModifiers& M = Config.Modifiers;
		M.CompanionDownTimerSeconds = 30.f;
		M.DeathResourceLossFraction = 0.1f;
		M.bCompanionPermadeath = true;

		TierConfigs.Add(EDifficultyTier::Survivor, Config);
	}

	// --- Eternal Engine ---
	{
		FDifficultyTierConfig Config;
		Config.Tier = EDifficultyTier::EternalEngine;
		Config.DisplayName = NSLOCTEXT("Difficulty", "EternalEngine", "Eternal Engine");
		Config.Description = NSLOCTEXT("Difficulty", "EternalEngineDesc",
			"The train is a meat grinder. Every resource matters. Every fight could be your last.");

		FDifficultyModifiers& M = Config.Modifiers;
		M.EnemyHealthMultiplier = 1.25f;
		M.EnemyDamageMultiplier = 1.3f;
		M.EnemyDetectionRangeMultiplier = 1.2f;
		M.EnemyDetectionSpeedMultiplier = 1.25f;
		M.ResourceDropRateMultiplier = 0.7f;
		M.ResourceDegradationMultiplier = 1.25f;
		M.ColdExposureTimeMultiplier = 0.75f;
		M.StaminaDrainMultiplier = 1.2f;
		M.XPGainMultiplier = 0.85f;
		M.CompanionDownTimerSeconds = 15.f;
		M.FallDamageMultiplier = 1.3f;
		M.InjurySeverityShift = 1;
		M.StatCheckModifier = -2;
		M.DeathResourceLossFraction = 0.25f;
		M.bCompanionPermadeath = true;
		M.bFriendlyFire = true;
		M.FriendlyFireDamageMultiplier = 0.25f;
		M.bShowQuestMarkers = false;
		M.bRestrictedSaving = true;

		TierConfigs.Add(EDifficultyTier::EternalEngine, Config);
	}

	// --- Mr. Wilford ---
	{
		FDifficultyTierConfig Config;
		Config.Tier = EDifficultyTier::MrWilford;
		Config.DisplayName = NSLOCTEXT("Difficulty", "MrWilford", "Mr. Wilford");
		Config.Description = NSLOCTEXT("Difficulty", "MrWilfordDesc",
			"Wilford designed this train to break people. Prove him wrong.");

		FDifficultyModifiers& M = Config.Modifiers;
		M.EnemyHealthMultiplier = 1.5f;
		M.EnemyDamageMultiplier = 1.6f;
		M.EnemyDetectionRangeMultiplier = 1.35f;
		M.EnemyDetectionSpeedMultiplier = 1.4f;
		M.ResourceDropRateMultiplier = 0.5f;
		M.ResourceDegradationMultiplier = 1.5f;
		M.ColdExposureTimeMultiplier = 0.6f;
		M.StaminaDrainMultiplier = 1.35f;
		M.XPGainMultiplier = 0.75f;
		M.CompanionDownTimerSeconds = 8.f;
		M.FallDamageMultiplier = 1.6f;
		M.InjurySeverityShift = 2;
		M.StatCheckModifier = -4;
		M.DeathResourceLossFraction = 1.0f; // Permadeath — irrelevant but set for completeness
		M.bCompanionPermadeath = true;
		M.bPermadeath = true;
		M.bFriendlyFire = true;
		M.FriendlyFireDamageMultiplier = 1.0f;
		M.bShowQuestMarkers = false;
		M.bRestrictedSaving = true;

		TierConfigs.Add(EDifficultyTier::MrWilford, Config);
	}
}

// ============================================================================
// Tier Management
// ============================================================================

bool USEEDifficultySubsystem::SetDifficultyTier(EDifficultyTier NewTier)
{
	if (NewTier == CurrentTier)
	{
		return true;
	}

	// Cannot switch TO Mr. Wilford mid-game
	if (NewTier == EDifficultyTier::MrWilford && CurrentTier != EDifficultyTier::MrWilford)
	{
		return false;
	}

	const EDifficultyTier OldTier = CurrentTier;

	// Leaving Mr. Wilford is permanent
	if (OldTier == EDifficultyTier::MrWilford && NewTier != EDifficultyTier::MrWilford)
	{
		bLeftMrWilford = true;
	}

	CurrentTier = NewTier;

	// Track lowest difficulty played
	if (static_cast<uint8>(NewTier) < static_cast<uint8>(LowestDifficultyPlayed))
	{
		LowestDifficultyPlayed = NewTier;
	}

	// Track Mr. Wilford history
	if (NewTier == EDifficultyTier::MrWilford)
	{
		bEverMrWilford = true;
	}

	OnDifficultyChanged.Broadcast(OldTier, NewTier);
	return true;
}

bool USEEDifficultySubsystem::CanRaiseDifficulty(EDifficultyTier TargetTier) const
{
	// Cannot raise to Mr. Wilford mid-game
	if (TargetTier == EDifficultyTier::MrWilford)
	{
		return false;
	}

	return static_cast<uint8>(TargetTier) > static_cast<uint8>(CurrentTier);
}

// ============================================================================
// Modifier Queries
// ============================================================================

FDifficultyModifiers USEEDifficultySubsystem::GetActiveModifiers() const
{
	FDifficultyModifiers Base = GetTierModifiers(CurrentTier);

	if (bAdaptiveEnabled && CurrentTier != EDifficultyTier::MrWilford)
	{
		const float Offset = GetAdaptiveMultiplierOffset();

		// Apply ±10% offset to numerical multipliers
		Base.EnemyHealthMultiplier *= (1.f + Offset);
		Base.EnemyDamageMultiplier *= (1.f + Offset);
		Base.EnemyDetectionRangeMultiplier *= (1.f + Offset);
		Base.EnemyDetectionSpeedMultiplier *= (1.f + Offset);
		Base.ResourceDropRateMultiplier *= (1.f - Offset); // Inverse: harder = fewer resources
		Base.ResourceDegradationMultiplier *= (1.f + Offset);
		Base.StaminaDrainMultiplier *= (1.f + Offset);
	}

	return Base;
}

FDifficultyModifiers USEEDifficultySubsystem::GetTierModifiers(EDifficultyTier Tier) const
{
	if (const FDifficultyTierConfig* Config = TierConfigs.Find(Tier))
	{
		return Config->Modifiers;
	}
	return FDifficultyModifiers(); // Baseline defaults
}

float USEEDifficultySubsystem::GetEnemyHealthMultiplier() const
{
	return GetActiveModifiers().EnemyHealthMultiplier;
}

float USEEDifficultySubsystem::GetEnemyDamageMultiplier() const
{
	return GetActiveModifiers().EnemyDamageMultiplier;
}

float USEEDifficultySubsystem::GetResourceDropMultiplier() const
{
	return GetActiveModifiers().ResourceDropRateMultiplier;
}

float USEEDifficultySubsystem::GetDetectionRangeMultiplier() const
{
	return GetActiveModifiers().EnemyDetectionRangeMultiplier;
}

int32 USEEDifficultySubsystem::GetStatCheckModifier() const
{
	return GetActiveModifiers().StatCheckModifier;
}

float USEEDifficultySubsystem::GetCompanionDownTimer() const
{
	return GetActiveModifiers().CompanionDownTimerSeconds;
}

bool USEEDifficultySubsystem::IsPermadeathActive() const
{
	return GetActiveModifiers().bPermadeath;
}

bool USEEDifficultySubsystem::IsSavingRestricted() const
{
	return GetActiveModifiers().bRestrictedSaving;
}

// ============================================================================
// Adaptive Difficulty
// ============================================================================

void USEEDifficultySubsystem::SetAdaptiveDifficultyEnabled(bool bEnabled)
{
	// Not available on Mr. Wilford
	if (bEnabled && CurrentTier == EDifficultyTier::MrWilford)
	{
		return;
	}

	bAdaptiveEnabled = bEnabled;

	if (!bEnabled)
	{
		// Reset to Normal sub-tier
		const EAdaptiveSubTier OldSubTier = CurrentSubTier;
		CurrentSubTier = EAdaptiveSubTier::Normal;
		if (OldSubTier != CurrentSubTier)
		{
			OnAdaptiveSubTierChanged.Broadcast(OldSubTier, CurrentSubTier);
		}
		PerformanceData.Reset();
	}
}

void USEEDifficultySubsystem::ReportPlayerDeath()
{
	if (!bAdaptiveEnabled) return;

	PerformanceData.DeathCount++;
	PerformanceData.PerformanceScore -= 10;
	UpdateAdaptiveSubTier();
}

void USEEDifficultySubsystem::ReportCombatComplete(float DurationSeconds, float HealthPercent)
{
	if (!bAdaptiveEnabled) return;

	CombatEncounterCount++;

	// Running average
	PerformanceData.AvgCombatTime =
		(PerformanceData.AvgCombatTime * (CombatEncounterCount - 1) + DurationSeconds) / CombatEncounterCount;
	PerformanceData.AvgHealthAtCombatEnd =
		(PerformanceData.AvgHealthAtCombatEnd * (CombatEncounterCount - 1) + HealthPercent) / CombatEncounterCount;

	// Score impact
	if (DurationSeconds > 60.f) PerformanceData.PerformanceScore -= 2;
	else if (DurationSeconds < 15.f) PerformanceData.PerformanceScore += 3;

	if (HealthPercent < 0.25f) PerformanceData.PerformanceScore -= 3;
	else if (HealthPercent > 0.75f) PerformanceData.PerformanceScore += 2;

	UpdateAdaptiveSubTier();
}

void USEEDifficultySubsystem::ReportStealthDetection(bool bWasDetected)
{
	if (!bAdaptiveEnabled) return;

	StealthAttemptCount++;
	if (bWasDetected) StealthDetectionCount++;

	PerformanceData.DetectionRate =
		StealthAttemptCount > 0
		? static_cast<float>(StealthDetectionCount) / static_cast<float>(StealthAttemptCount)
		: 0.f;

	if (PerformanceData.DetectionRate > 0.5f) PerformanceData.PerformanceScore -= 2;
	else if (PerformanceData.DetectionRate < 0.1f) PerformanceData.PerformanceScore += 3;

	UpdateAdaptiveSubTier();
}

void USEEDifficultySubsystem::ReportResourceStatus(float CarryCapacityPercent)
{
	if (!bAdaptiveEnabled) return;

	PerformanceData.ResourcePercent = CarryCapacityPercent;

	if (CarryCapacityPercent < 0.2f) PerformanceData.PerformanceScore -= 2;
	else if (CarryCapacityPercent > 0.6f) PerformanceData.PerformanceScore += 1;

	UpdateAdaptiveSubTier();
}

void USEEDifficultySubsystem::ResetAdaptiveData()
{
	PerformanceData.Reset();
	CombatEncounterCount = 0;
	StealthAttemptCount = 0;
	StealthDetectionCount = 0;

	const EAdaptiveSubTier OldSubTier = CurrentSubTier;
	CurrentSubTier = EAdaptiveSubTier::Normal;
	if (OldSubTier != CurrentSubTier)
	{
		OnAdaptiveSubTierChanged.Broadcast(OldSubTier, CurrentSubTier);
	}
}

void USEEDifficultySubsystem::UpdateAdaptiveSubTier()
{
	if (!bAdaptiveEnabled) return;

	const EAdaptiveSubTier OldSubTier = CurrentSubTier;
	EAdaptiveSubTier NewSubTier = EAdaptiveSubTier::Normal;

	if (PerformanceData.PerformanceScore < -15)
	{
		NewSubTier = EAdaptiveSubTier::Easy;
	}
	else if (PerformanceData.PerformanceScore > 15)
	{
		NewSubTier = EAdaptiveSubTier::Hard;
	}

	if (NewSubTier != OldSubTier)
	{
		CurrentSubTier = NewSubTier;
		OnAdaptiveSubTierChanged.Broadcast(OldSubTier, NewSubTier);
	}
}

float USEEDifficultySubsystem::GetAdaptiveMultiplierOffset() const
{
	switch (CurrentSubTier)
	{
	case EAdaptiveSubTier::Easy: return -0.1f;
	case EAdaptiveSubTier::Hard: return 0.1f;
	default: return 0.f;
	}
}
