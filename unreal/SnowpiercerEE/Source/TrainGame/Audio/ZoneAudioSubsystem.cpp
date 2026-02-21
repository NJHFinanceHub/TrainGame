// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ZoneAudioSubsystem.h"
#include "ZoneAudioProfile.h"
#include "AdaptiveMusicComponent.h"
#include "TrainAmbienceComponent.h"
#include "Kismet/GameplayStatics.h"

void UZoneAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("ZoneAudioSubsystem initialized — ready for zone profiles"));
}

void UZoneAudioSubsystem::Deinitialize()
{
	ActiveMusicComponent = nullptr;
	ActiveAmbienceComponent = nullptr;
	ZoneProfiles.Empty();
	Super::Deinitialize();
}

// --- Static Zone Mapping ---

EAudioZone UZoneAudioSubsystem::GetZoneForCar(int32 CarNumber)
{
	if (CarNumber >= 1 && CarNumber <= 15)		return EAudioZone::Tail;
	if (CarNumber >= 16 && CarNumber <= 30)		return EAudioZone::ThirdClass;
	if (CarNumber >= 31 && CarNumber <= 48)		return EAudioZone::SecondClass;
	if (CarNumber >= 49 && CarNumber <= 62)		return EAudioZone::WorkingSpine;
	if (CarNumber >= 63 && CarNumber <= 82)		return EAudioZone::FirstClass;
	if (CarNumber >= 83 && CarNumber <= 95)		return EAudioZone::Sanctum;
	if (CarNumber >= 96 && CarNumber <= 103)	return EAudioZone::Engine;
	return EAudioZone::None;
}

FInt32Range UZoneAudioSubsystem::GetCarRangeForZone(EAudioZone Zone)
{
	switch (Zone)
	{
	case EAudioZone::Tail:			return FInt32Range(1, 15);
	case EAudioZone::ThirdClass:	return FInt32Range(16, 30);
	case EAudioZone::SecondClass:	return FInt32Range(31, 48);
	case EAudioZone::WorkingSpine:	return FInt32Range(49, 62);
	case EAudioZone::FirstClass:	return FInt32Range(63, 82);
	case EAudioZone::Sanctum:		return FInt32Range(83, 95);
	case EAudioZone::Engine:		return FInt32Range(96, 103);
	default:						return FInt32Range(0, 0);
	}
}

FString UZoneAudioSubsystem::GetZoneName(EAudioZone Zone)
{
	switch (Zone)
	{
	case EAudioZone::Tail:			return TEXT("The Tail");
	case EAudioZone::ThirdClass:	return TEXT("Third Class");
	case EAudioZone::SecondClass:	return TEXT("Second Class");
	case EAudioZone::WorkingSpine:	return TEXT("The Working Spine");
	case EAudioZone::FirstClass:	return TEXT("First Class");
	case EAudioZone::Sanctum:		return TEXT("The Sanctum");
	case EAudioZone::Engine:		return TEXT("The Engine");
	default:						return TEXT("Unknown");
	}
}

// --- Profile Management ---

void UZoneAudioSubsystem::RegisterZoneProfile(UZoneAudioProfile* Profile)
{
	if (Profile && Profile->Zone != EAudioZone::None)
	{
		ZoneProfiles.Add(Profile->Zone, Profile);
		UE_LOG(LogTemp, Log, TEXT("ZoneAudioSubsystem: Registered profile for zone %s"),
			*GetZoneName(Profile->Zone));

		// If we have an active music component, update its profiles
		if (ActiveMusicComponent)
		{
			ActiveMusicComponent->ZoneProfiles.Add(Profile->Zone, Profile);
		}
	}
}

UZoneAudioProfile* UZoneAudioSubsystem::GetZoneProfile(EAudioZone Zone) const
{
	const UZoneAudioProfile* const* Found = ZoneProfiles.Find(Zone);
	return Found ? const_cast<UZoneAudioProfile*>(*Found) : nullptr;
}

// --- Component Registration ---

void UZoneAudioSubsystem::RegisterMusicComponent(UAdaptiveMusicComponent* Component)
{
	ActiveMusicComponent = Component;

	if (ActiveMusicComponent)
	{
		// Sync all registered profiles
		for (auto& Pair : ZoneProfiles)
		{
			ActiveMusicComponent->ZoneProfiles.Add(Pair.Key, Pair.Value);
		}

		// Set current zone if known
		if (CurrentZone != EAudioZone::None)
		{
			ActiveMusicComponent->SetAudioZone(CurrentZone);
		}
	}
}

void UZoneAudioSubsystem::RegisterAmbienceComponent(UTrainAmbienceComponent* Component)
{
	ActiveAmbienceComponent = Component;

	if (ActiveAmbienceComponent && CurrentZone != EAudioZone::None)
	{
		ActiveAmbienceComponent->SetZone(CurrentZone);
	}
}

// --- Zone Transition ---

void UZoneAudioSubsystem::OnPlayerEnteredCar(int32 CarNumber)
{
	CurrentCarNumber = CarNumber;
	EAudioZone NewZone = GetZoneForCar(CarNumber);

	if (NewZone == CurrentZone || NewZone == EAudioZone::None)
	{
		return;
	}

	EAudioZone OldZone = CurrentZone;
	CurrentZone = NewZone;

	UE_LOG(LogTemp, Log, TEXT("ZoneAudioSubsystem: Zone transition %s -> %s (car %d)"),
		*GetZoneName(OldZone), *GetZoneName(NewZone), CarNumber);

	// Play crossfade stinger
	if (OldZone != EAudioZone::None)
	{
		PlayCrossfadeStinger(OldZone, NewZone);
	}

	// Update active components
	if (ActiveMusicComponent)
	{
		ActiveMusicComponent->SetAudioZone(NewZone);
	}

	if (ActiveAmbienceComponent)
	{
		ActiveAmbienceComponent->SetZone(NewZone);
	}
}

// --- Volume Controls ---

void UZoneAudioSubsystem::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UZoneAudioSubsystem::SetAmbienceVolume(float Volume)
{
	AmbienceVolume = FMath::Clamp(Volume, 0.f, 1.f);
	if (ActiveAmbienceComponent)
	{
		ActiveAmbienceComponent->SetMasterVolume(AmbienceVolume);
	}
}

void UZoneAudioSubsystem::SetSFXVolume(float Volume)
{
	SFXVolume = FMath::Clamp(Volume, 0.f, 1.f);
}

void UZoneAudioSubsystem::PlayCrossfadeStinger(EAudioZone FromZone, EAudioZone ToZone)
{
	// Check for a custom stinger
	for (const FZoneCrossfadeStinger& Stinger : CrossfadeStingers)
	{
		if (Stinger.FromZone == FromZone && Stinger.ToZone == ToZone && Stinger.StingerSound)
		{
			UGameplayStatics::PlaySound2D(GetWorld(), Stinger.StingerSound, 1.f, 1.f);
			return;
		}
	}

	// If entering zone has a profile with an entry stinger, play that
	UZoneAudioProfile* ToProfile = GetZoneProfile(ToZone);
	if (ToProfile && ToProfile->ZoneEntryStinger)
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ToProfile->ZoneEntryStinger, 1.f, 1.f);
	}
}
