// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "TrainAmbienceComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UTrainAmbienceComponent::UTrainAmbienceComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20Hz update for smooth fades
}

void UTrainAmbienceComponent::BeginPlay()
{
	Super::BeginPlay();
	StartAmbience();
}

void UTrainAmbienceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAmbience();
	Super::EndPlay(EndPlayReason);
}

void UTrainAmbienceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVolumeFades(DeltaTime);
}

void UTrainAmbienceComponent::StartAmbience()
{
	StopAmbience();

	// Create the 5 ambience sub-layers in order matching EAmbienceLayer
	USoundBase* Sounds[] = { WheelRhythmLoop, EngineHumLoop, MetalCreakLoop, WindExteriorLoop, VentilationLoop };
	float InitialVolumes[] = { 0.7f, 0.3f, 0.4f, 0.2f, 0.2f };

	CurrentVolumes.Reset();
	TargetVolumes.Reset();

	for (int32 i = 0; i < 5; ++i)
	{
		CreateAmbienceLayer(Sounds[i], InitialVolumes[i]);
		CurrentVolumes.Add(InitialVolumes[i]);
		TargetVolumes.Add(InitialVolumes[i]);
	}

	// Apply zone mix if we already have a zone set
	if (CurrentZone != EAudioZone::None)
	{
		ApplyZoneMix(CurrentZone);
	}
}

void UTrainAmbienceComponent::StopAmbience()
{
	for (UAudioComponent* Comp : AmbienceLayers)
	{
		if (Comp)
		{
			Comp->Stop();
			Comp->DestroyComponent();
		}
	}
	AmbienceLayers.Empty();
	CurrentVolumes.Empty();
	TargetVolumes.Empty();
}

void UTrainAmbienceComponent::SetZone(EAudioZone NewZone)
{
	if (CurrentZone == NewZone)
	{
		return;
	}

	CurrentZone = NewZone;
	ApplyZoneMix(NewZone);
}

void UTrainAmbienceComponent::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UTrainAmbienceComponent::CreateAmbienceLayer(USoundBase* Sound, float InitialVolume)
{
	UAudioComponent* AudioComp = nullptr;

	if (Sound && GetOwner())
	{
		AudioComp = UGameplayStatics::SpawnSoundAttached(
			Sound,
			GetOwner()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			false, // bStopWhenAttachedToDestroyed
			InitialVolume * MasterVolume,
			1.f, // Pitch
			0.f, // StartTime
			nullptr, // Attenuation
			nullptr, // Concurrency
			true // bAutoDestroy - false, we manage lifecycle
		);

		if (AudioComp)
		{
			AudioComp->bAutoDestroy = false;
			AudioComp->bIsUISound = true; // Non-spatialized, always audible
		}
	}

	AmbienceLayers.Add(AudioComp);
}

void UTrainAmbienceComponent::UpdateVolumeFades(float DeltaTime)
{
	for (int32 i = 0; i < AmbienceLayers.Num() && i < CurrentVolumes.Num(); ++i)
	{
		if (FMath::IsNearlyEqual(CurrentVolumes[i], TargetVolumes[i], 0.01f))
		{
			CurrentVolumes[i] = TargetVolumes[i];
		}
		else
		{
			CurrentVolumes[i] = FMath::FInterpTo(CurrentVolumes[i], TargetVolumes[i], DeltaTime, ZoneCrossfadeSpeed);
		}

		if (AmbienceLayers[i])
		{
			AmbienceLayers[i]->SetVolumeMultiplier(CurrentVolumes[i] * MasterVolume);
		}
	}
}

void UTrainAmbienceComponent::ApplyZoneMix(EAudioZone Zone)
{
	float Wheel, Engine, Creak, Wind, Vent;
	GetDefaultZoneMix(Zone, Wheel, Engine, Creak, Wind, Vent);

	if (TargetVolumes.Num() >= 5)
	{
		TargetVolumes[0] = Wheel;
		TargetVolumes[1] = Engine;
		TargetVolumes[2] = Creak;
		TargetVolumes[3] = Wind;
		TargetVolumes[4] = Vent;
	}
}

void UTrainAmbienceComponent::GetDefaultZoneMix(EAudioZone Zone, float& OutWheel, float& OutEngine, float& OutCreak, float& OutWind, float& OutVent) const
{
	// Default mix tuned per zone's sonic character
	switch (Zone)
	{
	case EAudioZone::Tail:
		// Bumpy, rattling, windy — the train at its rawest
		OutWheel = 0.9f; OutEngine = 0.1f; OutCreak = 0.8f; OutWind = 0.7f; OutVent = 0.1f;
		break;

	case EAudioZone::ThirdClass:
		// Industrial, machinery, less wind
		OutWheel = 0.7f; OutEngine = 0.3f; OutCreak = 0.5f; OutWind = 0.3f; OutVent = 0.3f;
		break;

	case EAudioZone::SecondClass:
		// Dampened, more civilized — train noise reduced
		OutWheel = 0.4f; OutEngine = 0.2f; OutCreak = 0.2f; OutWind = 0.2f; OutVent = 0.5f;
		break;

	case EAudioZone::WorkingSpine:
		// Clean turbine, pressurized air — mechanical precision
		OutWheel = 0.5f; OutEngine = 0.7f; OutCreak = 0.1f; OutWind = 0.1f; OutVent = 0.8f;
		break;

	case EAudioZone::FirstClass:
		// Luxurious sound isolation — train barely audible
		OutWheel = 0.2f; OutEngine = 0.1f; OutCreak = 0.05f; OutWind = 0.1f; OutVent = 0.3f;
		break;

	case EAudioZone::Sanctum:
		// Deep, resonant — the train becomes cathedral-like
		OutWheel = 0.3f; OutEngine = 0.4f; OutCreak = 0.1f; OutWind = 0.05f; OutVent = 0.2f;
		break;

	case EAudioZone::Engine:
		// The engine IS the sound — everything else fades
		OutWheel = 0.4f; OutEngine = 1.0f; OutCreak = 0.05f; OutWind = 0.0f; OutVent = 0.1f;
		break;

	default:
		OutWheel = 0.5f; OutEngine = 0.2f; OutCreak = 0.3f; OutWind = 0.3f; OutVent = 0.3f;
		break;
	}
}
