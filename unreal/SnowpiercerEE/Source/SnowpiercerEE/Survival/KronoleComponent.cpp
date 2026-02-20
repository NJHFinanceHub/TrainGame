// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// KronoleComponent.cpp - Kronole drug system implementation

#include "KronoleComponent.h"

UKronoleComponent::UKronoleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;

	// Default withdrawal onset times by addiction stage:
	// Clean=never, Casual=600s, Dependent=300s, Addicted=120s, Terminal=60s
	WithdrawalOnsetTimes = { 9999.0f, 600.0f, 300.0f, 120.0f, 60.0f };
}

void UKronoleComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKronoleComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickBuff(DeltaTime);
	TickAddiction(DeltaTime);
	TickWithdrawal(DeltaTime);
}

// --- Kronole Usage ---

bool UKronoleComponent::TakeDose(bool bRefined)
{
	if (bIsActive) return false;

	bIsActive = true;
	bLastDoseWasRefined = bRefined;
	ActiveBuffTimer = bRefined ? RefinedBuffDuration : RawBuffDuration;
	TimeSinceLastDose = 0.0f;
	bInWithdrawal = false;

	float AddictionGain = bRefined ? AddictionPerRefinedDose : AddictionPerRawDose;
	AddictionLevel = FMath::Min(AddictionLevel + AddictionGain, 100.0f);
	UpdateAddictionStage();

	return true;
}

// --- Addiction ---

EKronoleAddictionStage UKronoleComponent::GetAddictionStage() const
{
	return CachedStage;
}

EWithdrawalSeverity UKronoleComponent::GetWithdrawalSeverity() const
{
	if (!bInWithdrawal) return EWithdrawalSeverity::None;

	float TimePastOnset = TimeSinceLastDose - GetWithdrawalOnsetTime();
	if (TimePastOnset <= 0.0f) return EWithdrawalSeverity::None;

	if (TimePastOnset < 60.0f) return EWithdrawalSeverity::Mild;
	if (TimePastOnset < 180.0f) return EWithdrawalSeverity::Moderate;
	if (TimePastOnset < 360.0f) return EWithdrawalSeverity::Severe;
	return EWithdrawalSeverity::Critical;
}

// --- Buff Modifiers ---

float UKronoleComponent::GetDamageMultiplier() const
{
	if (!bIsActive) return 1.0f;
	return bLastDoseWasRefined ? 1.5f : 1.25f;
}

float UKronoleComponent::GetTimeDilationFactor() const
{
	if (!bIsActive) return 1.0f;
	return bLastDoseWasRefined ? 0.5f : 0.7f;
}

float UKronoleComponent::GetDamageResistance() const
{
	if (!bIsActive) return 0.0f;
	return bLastDoseWasRefined ? 0.4f : 0.25f;
}

// --- Withdrawal Penalties ---

float UKronoleComponent::GetWithdrawalStaminaPenalty() const
{
	switch (GetWithdrawalSeverity())
	{
	case EWithdrawalSeverity::Mild:     return 0.8f;
	case EWithdrawalSeverity::Moderate: return 0.6f;
	case EWithdrawalSeverity::Severe:   return 0.35f;
	case EWithdrawalSeverity::Critical: return 0.15f;
	default: return 1.0f;
	}
}

float UKronoleComponent::GetWithdrawalMoraleDrain() const
{
	switch (GetWithdrawalSeverity())
	{
	case EWithdrawalSeverity::Mild:     return 0.05f;
	case EWithdrawalSeverity::Moderate: return 0.15f;
	case EWithdrawalSeverity::Severe:   return 0.3f;
	case EWithdrawalSeverity::Critical: return 0.5f;
	default: return 0.0f;
	}
}

float UKronoleComponent::GetWithdrawalCombatPenalty() const
{
	switch (GetWithdrawalSeverity())
	{
	case EWithdrawalSeverity::Mild:     return 0.9f;
	case EWithdrawalSeverity::Moderate: return 0.7f;
	case EWithdrawalSeverity::Severe:   return 0.5f;
	case EWithdrawalSeverity::Critical: return 0.3f;
	default: return 1.0f;
	}
}

// --- Serialization ---

void UKronoleComponent::SetAddictionState(float InAddictionLevel, float InTimeSinceLastDose)
{
	AddictionLevel = FMath::Clamp(InAddictionLevel, 0.0f, 100.0f);
	TimeSinceLastDose = InTimeSinceLastDose;
	UpdateAddictionStage();
}

// --- Private ---

void UKronoleComponent::TickBuff(float DeltaTime)
{
	if (!bIsActive) return;

	ActiveBuffTimer -= DeltaTime;
	if (ActiveBuffTimer <= 0.0f)
	{
		bIsActive = false;
		ActiveBuffTimer = 0.0f;
	}
}

void UKronoleComponent::TickAddiction(float DeltaTime)
{
	if (AddictionLevel <= 0.0f) return;

	if (!bIsActive)
	{
		TimeSinceLastDose += DeltaTime;
	}

	if (!bInWithdrawal && TimeSinceLastDose > GetWithdrawalOnsetTime() * 3.0f)
	{
		AddictionLevel = FMath::Max(0.0f, AddictionLevel - AddictionDecayRate * DeltaTime);
		UpdateAddictionStage();
	}
}

void UKronoleComponent::TickWithdrawal(float DeltaTime)
{
	if (bIsActive || AddictionLevel <= 0.0f)
	{
		bInWithdrawal = false;
		return;
	}

	float OnsetTime = GetWithdrawalOnsetTime();
	bInWithdrawal = (TimeSinceLastDose >= OnsetTime);
}

void UKronoleComponent::UpdateAddictionStage()
{
	EKronoleAddictionStage NewStage;
	if (AddictionLevel <= 0.0f)       NewStage = EKronoleAddictionStage::Clean;
	else if (AddictionLevel <= 20.0f) NewStage = EKronoleAddictionStage::Casual;
	else if (AddictionLevel <= 45.0f) NewStage = EKronoleAddictionStage::Dependent;
	else if (AddictionLevel <= 75.0f) NewStage = EKronoleAddictionStage::Addicted;
	else                              NewStage = EKronoleAddictionStage::Terminal;

	if (NewStage != CachedStage)
	{
		EKronoleAddictionStage OldStage = CachedStage;
		CachedStage = NewStage;
		OnAddictionStageChanged.Broadcast(OldStage, NewStage);
	}
}

float UKronoleComponent::GetWithdrawalOnsetTime() const
{
	int32 StageIndex = static_cast<int32>(CachedStage);
	if (WithdrawalOnsetTimes.IsValidIndex(StageIndex))
	{
		return WithdrawalOnsetTimes[StageIndex];
	}
	return 9999.0f;
}
