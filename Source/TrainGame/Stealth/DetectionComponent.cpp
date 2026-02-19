// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DetectionComponent.h"

UDetectionComponent::UDetectionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDetectionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDetectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSightDetection(DeltaTime);
	DrainDetectionMeter(DeltaTime);
	UpdateDetectionState();
	UpdateSearchTimer(DeltaTime);

	// Tick down recent sound counter
	if (RecentSoundCount > 0)
	{
		RecentSoundTimer += DeltaTime;
		if (RecentSoundTimer > 10.f)
		{
			RecentSoundCount = 0;
			RecentSoundTimer = 0.f;
		}
	}

	// Tick radio alert delay
	if (bRadioAlertPending)
	{
		RadioAlertTimer += DeltaTime;
		if (RadioAlertTimer >= RadioAlertDelay)
		{
			PropagateRadioAlert();
			bRadioAlertPending = false;
			RadioAlertTimer = 0.f;
		}
	}
}

bool UDetectionComponent::ProcessNoiseEvent(const FNoiseEvent& NoiseEvent)
{
	// TODO: Check distance against noise radius (accounting for environmental masking)
	// TODO: Turn toward sound origin
	// TODO: Escalate based on intensity (High → Suspicious, Combat → Alerted)
	// TODO: Track recent sounds for double-sound escalation

	OnNoiseHeard.Broadcast(NoiseEvent);
	return false;
}

bool UDetectionComponent::IsTargetInVisionCone(const AActor* Target) const
{
	// TODO: Calculate angle to target, check against inner/outer cone
	// TODO: Account for corridor clamping in narrow spaces
	return false;
}

bool UDetectionComponent::HasLineOfSightTo(const AActor* Target) const
{
	// TODO: Line trace from NPC eye height to target, check for blocking geometry
	return false;
}

void UDetectionComponent::NotifyBodyDiscovered(AActor* Body, EBodyState BodyState)
{
	// TODO: Escalate based on body state (Dead → COMBAT, Unconscious → ALERTED)
	// TODO: Trigger alert propagation
	OnBodyDiscovered.Broadcast(Body);
}

void UDetectionComponent::ReceiveAlert(EDetectionState MinimumState, FVector InvestigationPoint)
{
	// TODO: Set detection state to at least MinimumState
	// TODO: Set investigation target to InvestigationPoint
	LastKnownTargetLocation = InvestigationPoint;
}

void UDetectionComponent::UpdateSightDetection(float DeltaTime)
{
	// TODO: Find player, check vision cone, check LOS
	// TODO: Calculate fill rate based on distance band
	// TODO: Apply lighting, movement, and disguise modifiers
	// TODO: Apply heightened awareness bonus if flagged
	// TODO: Fill or drain detection meter
}

void UDetectionComponent::UpdateDetectionState()
{
	EDetectionState OldState = CurrentState;

	// TODO: Transition based on meter thresholds (25%, 60%, confirmed visual)
	// TODO: Enforce minimum hold times before de-escalation
	// TODO: Set bHeightenedAwareness on first COMBAT entry

	if (OldState != CurrentState)
	{
		OnDetectionStateChanged.Broadcast(OldState, CurrentState);

		// Trigger alert propagation on escalation
		if (CurrentState >= EDetectionState::Alerted)
		{
			PropagateVocalAlert();
			if (bHasRadio && !bRadioAlertPending)
			{
				bRadioAlertPending = true;
				RadioAlertTimer = 0.f;
			}
		}
	}
}

void UDetectionComponent::UpdateSearchTimer(float DeltaTime)
{
	// TODO: Count down search duration based on current state
	// TODO: De-escalate when timer expires (ALERTED → SUSPICIOUS → UNAWARE)
	// TODO: Reset timer on any new detection event
}

void UDetectionComponent::DrainDetectionMeter(float DeltaTime)
{
	// TODO: Drain meter when target is out of sight (after delay)
	// TODO: Preserve state thresholds even when meter drains
}

void UDetectionComponent::PropagateVocalAlert()
{
	// TODO: Find all NPCs within VocalAlertRadius
	// TODO: NPCs within 8m → ALERTED, within 15m → SUSPICIOUS
}

void UDetectionComponent::PropagateRadioAlert()
{
	// TODO: Find all NPCs in the current train car
	// TODO: Set them to ALERTED after delay
	// TODO: Check if car intercom is disabled (blocks radio propagation)
}

float UDetectionComponent::CalculateSightFillRate(float Distance, bool bInInnerCone) const
{
	// Distance bands: Close (0-300) = instant, Near (300-800) = 40%/s,
	// Medium (800-1500) = 20%/s, Far (1500-2500) = 8%/s
	// Outer cone applies 0.5x multiplier

	// TODO: Implement distance band lookup
	return 0.f;
}

float UDetectionComponent::GetLightingModifier() const
{
	// TODO: Query lighting level at NPC position
	// Bright = 1.0, Normal = 0.8, Dim = 0.5, Dark = 0.2, Flickering = oscillate
	return 1.f;
}

float UDetectionComponent::GetMovementModifier(const AActor* Target) const
{
	// TODO: Query target movement speed
	// Stationary = 0.3, CrouchWalk = 0.6, Walk = 1.0, Run = 1.5, Sprint = 2.0
	return 1.f;
}

float UDetectionComponent::GetDisguiseModifier(const AActor* Target) const
{
	// TODO: Query target's disguise component
	// Apply zone familiarity bonus (home zone +50%, security +100%)
	return 1.f;
}
