// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DeckSecurityDevice.h"
#include "TransportDeckSubsystem.h"
#include "TimerManager.h"

ADeckSecurityDevice::ADeckSecurityDevice()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f; // 4 Hz detection check
}

void ADeckSecurityDevice::BeginPlay()
{
	Super::BeginPlay();
}

void ADeckSecurityDevice::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bActive || bDisabled || bJammed || bTriggered)
	{
		return;
	}

	// Passive detection — scan for player within range
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return;
	}

	CheckDetection(PC->GetPawn());
}

bool ADeckSecurityDevice::CheckDetection(AActor* Target)
{
	if (!Target || !bActive || bDisabled || bJammed || bTriggered)
	{
		return false;
	}

	float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	float DetectionRangeCm = DetectionRadius * 100.0f;

	if (Dist > DetectionRangeCm)
	{
		return false;
	}

	bool bDetected = false;

	switch (SecurityType)
	{
	case EDeckSecurityType::MotionSensor:
		// Detects any movement within radius
		bDetected = (Target->GetVelocity().SizeSquared() > 100.0f);
		break;

	case EDeckSecurityType::LaserTripwire:
		// Simple proximity — always triggers if in range
		bDetected = true;
		break;

	case EDeckSecurityType::PatrolDrone:
	case EDeckSecurityType::SecurityCamera:
		// Cone-based detection
		bDetected = IsInDetectionCone(Target->GetActorLocation());
		break;

	case EDeckSecurityType::PressurePlate:
		// Only triggers if very close (standing on it)
		bDetected = (Dist <= 150.0f); // 1.5m
		break;
	}

	if (bDetected)
	{
		TriggerAlert();
	}

	return bDetected;
}

bool ADeckSecurityDevice::Disable(int32 CunningScore)
{
	if (bDisabled)
	{
		return true;
	}

	// Alert level increases difficulty
	int32 EffectiveDC = DisableDifficulty;
	UWorld* World = GetWorld();
	if (World)
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub)
		{
			switch (DeckSub->GetAlertLevel())
			{
			case EDeckAlertLevel::Caution: EffectiveDC += 2; break;
			case EDeckAlertLevel::Alert: EffectiveDC += 4; break;
			case EDeckAlertLevel::Lockdown: EffectiveDC += 8; break;
			default: break;
			}
		}
	}

	if (CunningScore >= EffectiveDC)
	{
		bDisabled = true;
		bActive = false;
		return true;
	}

	// Failed attempt triggers alert
	TriggerAlert();
	return false;
}

void ADeckSecurityDevice::JamDevice(float Duration)
{
	bJammed = true;

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			JamTimerHandle,
			[this]() { bJammed = false; },
			Duration,
			false
		);
	}
}

void ADeckSecurityDevice::ForceTrigger()
{
	if (bActive && !bDisabled && !bJammed)
	{
		TriggerAlert();
	}
}

void ADeckSecurityDevice::Reset()
{
	bTriggered = false;
}

void ADeckSecurityDevice::TriggerAlert()
{
	bTriggered = true;

	OnTriggered.Broadcast(SecurityType, GetActorLocation());

	// Raise deck alert
	UWorld* World = GetWorld();
	if (World)
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub)
		{
			DeckSub->RaiseAlert(EDeckAlertLevel::Alert);
		}
	}

	// Auto-reset after timeout
	if (ResetTime > 0.0f && World)
	{
		World->GetTimerManager().SetTimer(
			ResetTimerHandle,
			this,
			&ADeckSecurityDevice::Reset,
			ResetTime,
			false
		);
	}
}

bool ADeckSecurityDevice::IsInDetectionCone(const FVector& TargetLocation) const
{
	FVector ToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
	FVector Forward = GetActorForwardVector();

	float DotProduct = FVector::DotProduct(Forward, ToTarget);
	float AngleRad = FMath::Acos(DotProduct);
	float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	return AngleDeg <= DetectionConeAngle;
}
