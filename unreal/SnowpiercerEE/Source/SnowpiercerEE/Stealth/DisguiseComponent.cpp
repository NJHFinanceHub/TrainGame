// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DisguiseComponent.h"

UDisguiseComponent::UDisguiseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDisguiseComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDisguiseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (HasDisguise())
	{
		UpdatePassiveDegradation(DeltaTime);
	}

	TimeInCurrentZone += DeltaTime;
	UpdateChallengeTimer(DeltaTime);
}

bool UDisguiseComponent::EquipDisguise(const FDisguiseData& NewDisguise)
{
	if (NewDisguise.Quality == EDisguiseQuality::None)
	{
		return false;
	}

	// TODO: Play equip animation (EquipTime duration, interruptible)
	// TODO: Check if disguise type is burned in current car

	FDisguiseData OldDisguise = CurrentDisguise;
	CurrentDisguise = NewDisguise;
	bDisguiseBlown = false;
	OnDisguiseChanged.Broadcast(OldDisguise, CurrentDisguise);
	return true;
}

void UDisguiseComponent::RemoveDisguise()
{
	FDisguiseData OldDisguise = CurrentDisguise;
	CurrentDisguise = FDisguiseData();
	bDisguiseBlown = false;
	OnDisguiseChanged.Broadcast(OldDisguise, CurrentDisguise);
}

void UDisguiseComponent::DegradeDisguise(float Amount)
{
	if (!HasDisguise())
	{
		return;
	}

	CurrentDisguise.Degradation = FMath::Clamp(CurrentDisguise.Degradation + Amount, 0.f, 100.f);
	OnDisguiseDegraded.Broadcast(CurrentDisguise.Degradation);

	// At 100% degradation the disguise is effectively destroyed
	if (CurrentDisguise.Degradation >= 100.f)
	{
		RemoveDisguise();
	}
}

void UDisguiseComponent::BlowDisguise()
{
	bDisguiseBlown = true;
	OnDisguiseBlown.Broadcast();
	// TODO: Mark this disguise type as burned for the current car
	// TODO: Propagate face description via radio if challenger has radio
}

bool UDisguiseComponent::ResolveChallenge(AActor* Challenger)
{
	OnChallengeInitiated.Broadcast(Challenger);

	// TODO: Open dialogue with skill check based on zone
	// TODO: Check primary stat first, fallback to secondary
	// TODO: On success: grant 30s grace period, increment pass count
	// TODO: On failure: blow disguise, NPC escalates to ALERTED

	// Check if this NPC has been passed enough times to stop challenging
	if (int32* Count = ChallengePassCount.Find(Challenger))
	{
		if (*Count >= MaxChallengesPerNPC)
		{
			return true; // NPC "knows" the player now
		}
	}

	// TODO: Actual skill check resolution (placeholder)
	return false;
}

float UDisguiseComponent::GetDetectionModifier(EStealthTrainZone CurrentZone) const
{
	if (!HasDisguise())
	{
		// No disguise in an upper zone = very conspicuous
		return 2.0f;
	}

	if (bDisguiseBlown)
	{
		return 1.5f; // Blown disguise offers marginal benefit at best
	}

	if (!MatchesZone(CurrentZone))
	{
		return 1.5f; // Wrong zone attire
	}

	// Modifier based on effective quality
	switch (CurrentDisguise.GetEffectiveQuality())
	{
	case EDisguiseQuality::Perfect:	return 0.1f;
	case EDisguiseQuality::Good:	return 0.3f;
	case EDisguiseQuality::Partial:	return 0.6f;
	default:						return 1.0f;
	}
}

bool UDisguiseComponent::MatchesZone(EStealthTrainZone Zone) const
{
	return HasDisguise() && CurrentDisguise.TargetZone == Zone;
}

void UDisguiseComponent::UpdatePassiveDegradation(float DeltaTime)
{
	DegradeDisguise(PassiveDegradationRate * DeltaTime);
}

void UDisguiseComponent::UpdateChallengeTimer(float DeltaTime)
{
	if (!HasDisguise() || bDisguiseBlown)
	{
		return;
	}

	ChallengeTimer += DeltaTime;

	// TODO: Check if it's time to issue a challenge
	// First challenge after FirstChallengeDelay, then every ChallengeInterval
	// Find nearest NPC that hasn't exceeded MaxChallengesPerNPC
}
