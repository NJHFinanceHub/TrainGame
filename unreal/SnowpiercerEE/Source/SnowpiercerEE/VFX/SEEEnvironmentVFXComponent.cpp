// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEEnvironmentVFXComponent.cpp - Environmental VFX: steam, sparks, flickers

#include "SEEEnvironmentVFXComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PointLightComponent.h"

USEEEnvironmentVFXComponent::USEEEnvironmentVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void USEEEnvironmentVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoDiscoverLights)
	{
		DiscoverLights();
	}

	// Cache base light intensities for flicker calculations
	BaseLightIntensities.Reset();
	for (UPointLightComponent* Light : FlickerTargetLights)
	{
		if (Light)
		{
			BaseLightIntensities.Add(Light->Intensity);
		}
		else
		{
			BaseLightIntensities.Add(0.0f);
		}
	}

	// Random phase offset for cyclic effects
	if (bCyclic)
	{
		CycleTimer = FMath::RandRange(0.0f, CycleRandomOffset);
	}

	NextFlickerTime = GetNextFlickerInterval();

	if (bStartActive)
	{
		ActivateEffect();
	}
}

void USEEEnvironmentVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyNiagaraEffect();
	Super::EndPlay(EndPlayReason);
}

void USEEEnvironmentVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCyclic)
	{
		UpdateCyclic(DeltaTime);
	}

	if (FlickerConfig.Pattern != ESEEFlickerPattern::Steady)
	{
		UpdateFlicker(DeltaTime);
	}
}

void USEEEnvironmentVFXComponent::ActivateEffect()
{
	if (bIsVFXActive) return;
	bIsVFXActive = true;
	SpawnNiagaraEffect();
}

void USEEEnvironmentVFXComponent::DeactivateEffect()
{
	if (!bIsVFXActive) return;
	bIsVFXActive = false;
	DestroyNiagaraEffect();
}

void USEEEnvironmentVFXComponent::SetEffectIntensity(float Intensity)
{
	CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 2.0f);

	if (ActiveNiagara)
	{
		ActiveNiagara->SetFloatParameter(FName("Intensity"), CurrentIntensity);
		ActiveNiagara->SetFloatParameter(FName("SpawnRate"),
			EmitterConfig.SpawnRate * CurrentIntensity);
	}
}

void USEEEnvironmentVFXComponent::TriggerBurst()
{
	UNiagaraSystem* System = EmitterConfig.NiagaraSystem.LoadSynchronous();
	if (!System) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(), System,
		Owner->GetActorLocation(),
		Owner->GetActorRotation(),
		EmitterConfig.Scale, true, true,
		ENCPoolMethod::AutoRelease);
}

// --- Internal ---

void USEEEnvironmentVFXComponent::SpawnNiagaraEffect()
{
	UNiagaraSystem* System = EmitterConfig.NiagaraSystem.LoadSynchronous();
	if (!System) return;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetRootComponent()) return;

	ActiveNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System, Owner->GetRootComponent(), NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset, true,
		true, ENCPoolMethod::None);

	if (ActiveNiagara)
	{
		ActiveNiagara->SetWorldScale3D(EmitterConfig.Scale);
		ActiveNiagara->SetColorParameter(FName("ColorTint"), EmitterConfig.ColorTint);
		ActiveNiagara->SetFloatParameter(FName("SpawnRate"), EmitterConfig.SpawnRate);
		ActiveNiagara->SetFloatParameter(FName("Lifetime"), EmitterConfig.Lifetime);
		ActiveNiagara->SetFloatParameter(FName("Intensity"), CurrentIntensity);
	}
}

void USEEEnvironmentVFXComponent::DestroyNiagaraEffect()
{
	if (ActiveNiagara)
	{
		ActiveNiagara->Deactivate();
		ActiveNiagara->DestroyComponent();
		ActiveNiagara = nullptr;
	}
}

void USEEEnvironmentVFXComponent::UpdateCyclic(float DeltaTime)
{
	CycleTimer += DeltaTime;

	float FullCycle = ActiveDuration + InactiveDuration;
	float PhaseTime = FMath::Fmod(CycleTimer, FullCycle);

	bool bShouldBeActive = (PhaseTime < ActiveDuration);

	if (bShouldBeActive && !bIsVFXActive)
	{
		ActivateEffect();
	}
	else if (!bShouldBeActive && bIsVFXActive)
	{
		DeactivateEffect();
	}
}

void USEEEnvironmentVFXComponent::UpdateFlicker(float DeltaTime)
{
	if (FlickerTargetLights.Num() == 0) return;

	FlickerTimer += DeltaTime;

	if (bFlickerActive)
	{
		FlickerEventTimer -= DeltaTime;
		if (FlickerEventTimer <= 0.0f)
		{
			// End flicker — restore base intensities
			bFlickerActive = false;
			for (int32 i = 0; i < FlickerTargetLights.Num(); ++i)
			{
				if (FlickerTargetLights[i] && i < BaseLightIntensities.Num())
				{
					FlickerTargetLights[i]->SetIntensity(BaseLightIntensities[i]);
					FlickerTargetLights[i]->SetLightColor(FLinearColor::White);
				}
			}
		}
	}
	else if (FlickerTimer >= NextFlickerTime)
	{
		// Start flicker event
		bFlickerActive = true;

		float FlickerDuration = FlickerConfig.FlickerDuration;
		// Erratic patterns get variable duration
		if (FlickerConfig.Pattern == ESEEFlickerPattern::Damaged)
		{
			FlickerDuration *= FMath::RandRange(0.5f, 3.0f);
		}
		FlickerEventTimer = FlickerDuration;

		float IntensityMult = FlickerConfig.MinIntensity;
		// Emergency pattern pulses rather than goes dark
		if (FlickerConfig.Pattern == ESEEFlickerPattern::Emergency)
		{
			IntensityMult = FMath::RandRange(0.3f, 0.8f);
		}
		// Dying pattern sometimes goes completely dark
		else if (FlickerConfig.Pattern == ESEEFlickerPattern::Dying)
		{
			IntensityMult = FMath::RandRange(0.0f, 0.2f);
		}

		for (int32 i = 0; i < FlickerTargetLights.Num(); ++i)
		{
			if (FlickerTargetLights[i] && i < BaseLightIntensities.Num())
			{
				FlickerTargetLights[i]->SetIntensity(BaseLightIntensities[i] * IntensityMult);
				FlickerTargetLights[i]->SetLightColor(FlickerConfig.FlickerColor);
			}
		}

		FlickerTimer = 0.0f;
		NextFlickerTime = GetNextFlickerInterval();
	}
}

void USEEEnvironmentVFXComponent::DiscoverLights()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	TArray<UPointLightComponent*> Lights;
	Owner->GetComponents<UPointLightComponent>(Lights);
	for (UPointLightComponent* Light : Lights)
	{
		FlickerTargetLights.AddUnique(Light);
	}
}

float USEEEnvironmentVFXComponent::GetNextFlickerInterval() const
{
	float Base = FlickerConfig.FlickerInterval;
	float RandomRange = Base * FlickerConfig.Randomness;
	return Base + FMath::RandRange(-RandomRange, RandomRange);
}
