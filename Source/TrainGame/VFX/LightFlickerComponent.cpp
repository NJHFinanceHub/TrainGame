// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "LightFlickerComponent.h"
#include "Components/LightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"

ULightFlickerComponent::ULightFlickerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.02f; // 50Hz for smooth flicker
}

void ULightFlickerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Find the light component on our owner
	if (AActor* Owner = GetOwner())
	{
		TargetLight = Owner->FindComponentByClass<ULightComponent>();
		if (TargetLight)
		{
			BaseIntensity = TargetLight->Intensity;
			NormalColor = TargetLight->GetLightColor();
		}
	}

	if (bRandomPhaseOffset)
	{
		PhaseOffset = FMath::FRand() * 100.f;
	}
}

void ULightFlickerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetLight) return;

	FlickerTimer += DeltaTime;
	const float Time = FlickerTimer + PhaseOffset;

	// Handle forced state
	if (bForced)
	{
		if (ForcedTimer > 0.f)
		{
			ForcedTimer -= DeltaTime;
			if (ForcedTimer <= 0.f)
			{
				bForced = false;
			}
		}

		CurrentIntensity = bForcedOn ? 1.f : 0.f;
		TargetLight->SetIntensity(BaseIntensity * CurrentIntensity);
		return;
	}

	// Handle damage flicker (overrides normal pattern temporarily)
	if (bDamageFlickering)
	{
		DamageFlickerTimer -= DeltaTime;
		if (DamageFlickerTimer <= 0.f)
		{
			bDamageFlickering = false;
		}
		else
		{
			// Rapid chaotic flicker during damage
			CurrentIntensity = FMath::FRand() > 0.4f ? 1.f : MinIntensity;
			TargetLight->SetIntensity(BaseIntensity * CurrentIntensity);

			if (bColorShiftOnFlicker)
			{
				const FLinearColor Color = FLinearColor::LerpUsingHSV(NormalColor, FlickerColor, 1.f - CurrentIntensity);
				TargetLight->SetLightColor(Color);
			}
			return;
		}
	}

	// Normal pattern evaluation
	if (Pattern == EFlickerPattern::Steady)
	{
		CurrentIntensity = 1.f;
	}
	else
	{
		CurrentIntensity = EvaluateFlicker(Time);
	}

	TargetLight->SetIntensity(BaseIntensity * FMath::Lerp(MinIntensity, 1.f, CurrentIntensity));

	if (bColorShiftOnFlicker && Pattern != EFlickerPattern::Steady)
	{
		const FLinearColor Color = FLinearColor::LerpUsingHSV(FlickerColor, NormalColor, CurrentIntensity);
		TargetLight->SetLightColor(Color);
	}
}

void ULightFlickerComponent::SetFlickerPattern(EFlickerPattern NewPattern)
{
	Pattern = NewPattern;
}

void ULightFlickerComponent::ForceState(bool bOn, float Duration)
{
	bForced = true;
	bForcedOn = bOn;
	ForcedTimer = Duration;
}

void ULightFlickerComponent::ClearForcedState()
{
	bForced = false;
}

void ULightFlickerComponent::TriggerDamageFlicker(float Duration)
{
	bDamageFlickering = true;
	DamageFlickerTimer = Duration;
}

float ULightFlickerComponent::EvaluateFlicker(float Time) const
{
	switch (Pattern)
	{
	case EFlickerPattern::Occasional:	return EvaluateOccasional(Time);
	case EFlickerPattern::Nervous:		return EvaluateNervous(Time);
	case EFlickerPattern::Dying:		return EvaluateDying(Time);
	case EFlickerPattern::Strobing:		return EvaluateStrobing(Time);
	default:							return 1.f;
	}
}

float ULightFlickerComponent::EvaluateOccasional(float Time) const
{
	// Mostly on, with random single blinks every few seconds
	// Use a hash-like function to create pseudo-random blinks
	const float Cycle = FMath::Fmod(Time * 0.3f, 1.f);
	const float Hash = FMath::Fmod(FMath::Sin(Time * 12.9898f) * 43758.5453f, 1.f);

	// ~5% of the time, blink off briefly
	if (FMath::Abs(Hash) < 0.05f && Cycle < 0.1f)
	{
		return 0.f;
	}
	return 1.f;
}

float ULightFlickerComponent::EvaluateNervous(float Time) const
{
	// Rapid irregular stuttering — damaged fluorescent tube
	const float Fast = FMath::Sin(Time * 30.f);
	const float Medium = FMath::Sin(Time * 7.3f);
	const float Slow = FMath::Sin(Time * 2.1f);

	const float Combined = (Fast * 0.3f + Medium * 0.4f + Slow * 0.3f);
	return FMath::Clamp(Combined + 0.3f, 0.f, 1.f);
}

float ULightFlickerComponent::EvaluateDying(float Time) const
{
	// Long periods off, brief flashes on — tube about to die
	const float Cycle = FMath::Fmod(Time * 0.15f, 1.f);

	// On for ~20% of cycle, off for ~80%
	if (Cycle < 0.15f)
	{
		// Brief on flash with a flicker at start
		const float FlashProgress = Cycle / 0.15f;
		return FlashProgress > 0.1f ? 1.f : FMath::FRand();
	}
	else if (Cycle < 0.2f)
	{
		// Quick flicker as it dies again
		return FMath::FRand() > 0.6f ? 0.5f : 0.f;
	}

	return 0.f;
}

float ULightFlickerComponent::EvaluateStrobing(float Time) const
{
	// Regular on/off pulsing — alarm state, emergency
	const float StrobeRate = 4.f; // Hz
	return FMath::Sin(Time * StrobeRate * PI * 2.f) > 0.f ? 1.f : 0.f;
}
