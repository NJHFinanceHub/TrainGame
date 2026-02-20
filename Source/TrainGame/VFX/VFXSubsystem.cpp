// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "VFXSubsystem.h"
#include "ZoneVFXProfile.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"

void UVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentKronoleParams = FKronolePostProcessParams();
}

void UVFXSubsystem::Deinitialize()
{
	ZoneProfiles.Empty();
	ActiveProfile = nullptr;
	Super::Deinitialize();
}

void UVFXSubsystem::RegisterZoneProfile(UZoneVFXProfile* Profile)
{
	if (!Profile) return;
	ZoneProfiles.Add(Profile->Zone, Profile);
}

void UVFXSubsystem::SetActiveZone(EVFXZone NewZone)
{
	if (NewZone == ActiveZone) return;

	const EVFXZone OldZone = ActiveZone;
	ActiveZone = NewZone;

	UZoneVFXProfile** Found = ZoneProfiles.Find(NewZone);
	if (Found && *Found)
	{
		TransitionToProfile(*Found);
	}

	OnZoneVFXChanged.Broadcast(NewZone, OldZone);
}

void UVFXSubsystem::TransitionToProfile(UZoneVFXProfile* NewProfile)
{
	ActiveProfile = NewProfile;
}

// --- Weather ---

void UVFXSubsystem::SetWeatherOverride(EWeatherType Weather)
{
	const EWeatherType OldWeather = GetCurrentWeather();
	bWeatherOverrideActive = true;
	WeatherOverride = Weather;

	const EWeatherType NewWeather = GetCurrentWeather();
	if (NewWeather != OldWeather)
	{
		OnWeatherChanged.Broadcast(NewWeather, OldWeather);
	}
}

void UVFXSubsystem::ClearWeatherOverride()
{
	const EWeatherType OldWeather = GetCurrentWeather();
	bWeatherOverrideActive = false;

	const EWeatherType NewWeather = GetCurrentWeather();
	if (NewWeather != OldWeather)
	{
		OnWeatherChanged.Broadcast(NewWeather, OldWeather);
	}
}

EWeatherType UVFXSubsystem::GetCurrentWeather() const
{
	if (bWeatherOverrideActive)
	{
		return WeatherOverride;
	}
	if (ActiveProfile)
	{
		return ActiveProfile->DefaultWeather;
	}
	return EWeatherType::Blizzard;
}

// --- Combat VFX ---

void UVFXSubsystem::SpawnImpactVFX(FVector Location, FRotator ImpactNormal, EImpactSurface Surface)
{
	if (!ActiveProfile) return;

	const FImpactVFXConfig* Config = ActiveProfile->FindImpactConfig(Surface);
	if (!Config) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (Config->ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, Config->ImpactEffect, Location, ImpactNormal,
			FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
	}

	if (Config->DecalMaterial)
	{
		SpawnImpactDecal(Config->DecalMaterial, Location, ImpactNormal, Config->DecalSizeRange, Config->DecalLifetime);
	}
}

void UVFXSubsystem::SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity)
{
	if (!ActiveProfile) return;

	UWorld* World = GetWorld();
	if (!World) return;

	if (ActiveProfile->BloodSplatterEffect)
	{
		const FRotator Rotation = Direction.Rotation();
		UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, ActiveProfile->BloodSplatterEffect, Location, Rotation,
			FVector(Intensity), true, true, ENCPoolMethod::AutoRelease);

		if (NC)
		{
			NC->SetFloatParameter(FName("Intensity"), Intensity);
		}
	}

	if (ActiveProfile->BloodDecalMaterial)
	{
		const FRotator DecalRot = FRotationMatrix::MakeFromZ(-Direction).Rotator();
		const float Size = FMath::Lerp(8.f, 40.f, Intensity);
		SpawnImpactDecal(ActiveProfile->BloodDecalMaterial, Location, DecalRot,
			FVector2D(Size * 0.8f, Size * 1.2f), 60.f);
	}
}

void UVFXSubsystem::SpawnImpactDecal(UMaterialInterface* Material, FVector Location, FRotator Rotation, FVector2D SizeRange, float Lifetime)
{
	UWorld* World = GetWorld();
	if (!World || !Material) return;

	const float Size = FMath::RandRange(SizeRange.X, SizeRange.Y);
	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
		World, Material, FVector(Size), Location, Rotation, Lifetime);

	if (Decal && Lifetime > 0.f)
	{
		Decal->SetFadeScreenSize(0.001f);
		Decal->SetFadeOut(Lifetime * 0.8f, Lifetime * 0.2f);
	}
}

// --- Kronole VFX ---

void UVFXSubsystem::SetKronoleIntensity(EKronoleVFXIntensity Intensity)
{
	KronoleIntensity = Intensity;

	if (!ActiveProfile)
	{
		CurrentKronoleParams = FKronolePostProcessParams();
		return;
	}

	switch (Intensity)
	{
	case EKronoleVFXIntensity::None:
		CurrentKronoleParams = FKronolePostProcessParams();
		break;

	case EKronoleVFXIntensity::Onset:
	{
		const FKronolePostProcessParams& Peak = ActiveProfile->KronolePeakParams;
		CurrentKronoleParams.ChromaticAberration = Peak.ChromaticAberration * 0.3f;
		CurrentKronoleParams.RadialBlur = Peak.RadialBlur * 0.2f;
		CurrentKronoleParams.Saturation = FMath::Lerp(1.f, Peak.Saturation, 0.3f);
		CurrentKronoleParams.ColorTint = FLinearColor::LerpUsingHSV(FLinearColor::White, Peak.ColorTint, 0.3f);
		CurrentKronoleParams.VignetteIntensity = Peak.VignetteIntensity * 0.2f;
		CurrentKronoleParams.FilmGrain = Peak.FilmGrain * 0.1f;
		break;
	}

	case EKronoleVFXIntensity::Peak:
		CurrentKronoleParams = ActiveProfile->KronolePeakParams;
		break;

	case EKronoleVFXIntensity::Comedown:
	{
		const FKronolePostProcessParams& Peak = ActiveProfile->KronolePeakParams;
		CurrentKronoleParams.ChromaticAberration = Peak.ChromaticAberration * 0.5f;
		CurrentKronoleParams.RadialBlur = Peak.RadialBlur * 0.1f;
		CurrentKronoleParams.Saturation = FMath::Lerp(1.f, Peak.Saturation, 0.5f);
		CurrentKronoleParams.ColorTint = FLinearColor::LerpUsingHSV(FLinearColor::White, Peak.ColorTint, 0.4f);
		CurrentKronoleParams.VignetteIntensity = Peak.VignetteIntensity * 0.3f;
		CurrentKronoleParams.FilmGrain = Peak.FilmGrain * 0.3f;
		break;
	}

	case EKronoleVFXIntensity::Withdrawal:
		CurrentKronoleParams = ActiveProfile->KronoleWithdrawalParams;
		break;
	}
}
