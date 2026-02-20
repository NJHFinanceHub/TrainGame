// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEWeatherVFXComponent.cpp - Weather through windows

#include "SEEWeatherVFXComponent.h"
#include "SEEVFXSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

USEEWeatherVFXComponent::USEEWeatherVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20Hz is sufficient for weather transitions
}

void USEEWeatherVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create dynamic material instance for glass
	if (WindowGlassMesh && WindowGlassMesh->GetMaterial(0))
	{
		GlassMID = WindowGlassMesh->CreateAndSetMaterialInstanceDynamic(0);
	}

	CurrentFrost = FrostCoverage;
	TargetFrost = FrostCoverage;

	SpawnWeatherEffect();

	// Register with global VFX subsystem
	if (UWorld* World = GetWorld())
	{
		if (USEEVFXSubsystem* VFXSub = World->GetSubsystem<USEEVFXSubsystem>())
		{
			VFXSub->RegisterWeatherComponent(this);
		}
	}
}

void USEEWeatherVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USEEVFXSubsystem* VFXSub = World->GetSubsystem<USEEVFXSubsystem>())
		{
			VFXSub->UnregisterWeatherComponent(this);
		}
	}

	DestroyWeatherEffect();
	Super::EndPlay(EndPlayReason);
}

void USEEWeatherVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Transition blending
	if (WeatherTransitionAlpha < 1.0f)
	{
		WeatherTransitionAlpha = FMath::Min(WeatherTransitionAlpha + DeltaTime * 0.5f, 1.0f);

		if (ActiveWeatherNiagara)
		{
			ActiveWeatherNiagara->SetFloatParameter(FName("TransitionAlpha"), WeatherTransitionAlpha);
		}
	}

	UpdateGlassMaterial(DeltaTime);
}

void USEEWeatherVFXComponent::SetWeather(ESEEWeatherType NewWeather)
{
	if (CurrentWeather == NewWeather) return;

	ESEEWeatherType OldWeather = CurrentWeather;
	PreviousWeather = CurrentWeather;
	CurrentWeather = NewWeather;
	WeatherTransitionAlpha = 0.0f;

	DestroyWeatherEffect();
	SpawnWeatherEffect();

	// Adjust frost based on weather severity
	switch (NewWeather)
	{
	case ESEEWeatherType::Blizzard:
	case ESEEWeatherType::WhiteOut:
		TargetFrost = 0.95f;
		break;
	case ESEEWeatherType::HeavySnow:
		TargetFrost = 0.7f;
		break;
	case ESEEWeatherType::LightSnow:
		TargetFrost = 0.5f;
		break;
	case ESEEWeatherType::Aurora:
	case ESEEWeatherType::ClearSky:
		TargetFrost = 0.3f;
		break;
	}

	OnWeatherChanged.Broadcast(OldWeather, NewWeather);
}

void USEEWeatherVFXComponent::SetFrostCoverage(float Coverage)
{
	TargetFrost = FMath::Clamp(Coverage, 0.0f, 1.0f);
}

void USEEWeatherVFXComponent::SetWindParameters(FVector Direction, float Speed)
{
	WindDirection = Direction.GetSafeNormal();
	WindSpeed = Speed;

	if (ActiveWeatherNiagara)
	{
		ActiveWeatherNiagara->SetVectorParameter(FName("WindDirection"), WindDirection);
		ActiveWeatherNiagara->SetFloatParameter(FName("WindSpeed"), WindSpeed);
	}
}

// --- Internal ---

void USEEWeatherVFXComponent::SpawnWeatherEffect()
{
	UNiagaraSystem* System = GetSystemForWeather(CurrentWeather);
	if (!System) return;

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->GetRootComponent()) return;

	// Spawn outside the window (offset in window's forward direction)
	ActiveWeatherNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
		System, Owner->GetRootComponent(), NAME_None,
		FVector(200.0f, 0.0f, 0.0f), FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset, true,
		true, ENCPoolMethod::None);

	if (ActiveWeatherNiagara)
	{
		ActiveWeatherNiagara->SetVectorParameter(FName("WindDirection"), WindDirection);
		ActiveWeatherNiagara->SetFloatParameter(FName("WindSpeed"), WindSpeed);
		ActiveWeatherNiagara->SetFloatParameter(FName("TransitionAlpha"), 0.0f);
	}
}

void USEEWeatherVFXComponent::DestroyWeatherEffect()
{
	if (ActiveWeatherNiagara)
	{
		ActiveWeatherNiagara->Deactivate();
		ActiveWeatherNiagara->DestroyComponent();
		ActiveWeatherNiagara = nullptr;
	}
}

void USEEWeatherVFXComponent::UpdateGlassMaterial(float DeltaTime)
{
	if (!GlassMID) return;

	CurrentFrost = FMath::FInterpTo(CurrentFrost, TargetFrost, DeltaTime, 0.5f);
	GlassMID->SetScalarParameterValue(FName("FrostCoverage"), CurrentFrost);

	FLinearColor AmbientColor = GetAmbientColorForWeather(CurrentWeather);
	if (WeatherTransitionAlpha < 1.0f)
	{
		FLinearColor PrevColor = GetAmbientColorForWeather(PreviousWeather);
		AmbientColor = FMath::Lerp(PrevColor, AmbientColor, WeatherTransitionAlpha);
	}
	GlassMID->SetVectorParameterValue(FName("ExteriorAmbient"), AmbientColor);

	// Visibility through frost
	float Visibility = 1.0f - (CurrentFrost * 0.8f);
	GlassMID->SetScalarParameterValue(FName("ExteriorVisibility"), Visibility);
}

UNiagaraSystem* USEEWeatherVFXComponent::GetSystemForWeather(ESEEWeatherType Weather) const
{
	switch (Weather)
	{
	case ESEEWeatherType::Blizzard:
	case ESEEWeatherType::WhiteOut:
		return BlizzardSystem.LoadSynchronous();
	case ESEEWeatherType::HeavySnow:
		return HeavySnowSystem.LoadSynchronous();
	case ESEEWeatherType::LightSnow:
		return LightSnowSystem.LoadSynchronous();
	case ESEEWeatherType::Aurora:
		return AuroraSystem.LoadSynchronous();
	case ESEEWeatherType::ClearSky:
		return nullptr; // No particles for clear sky
	}
	return nullptr;
}

FLinearColor USEEWeatherVFXComponent::GetAmbientColorForWeather(ESEEWeatherType Weather) const
{
	switch (Weather)
	{
	case ESEEWeatherType::Blizzard:
	case ESEEWeatherType::WhiteOut:
		return FLinearColor(0.7f, 0.75f, 0.85f, 1.0f); // Cold white-blue
	case ESEEWeatherType::HeavySnow:
		return FLinearColor(0.5f, 0.55f, 0.65f, 1.0f); // Dim grey-blue
	case ESEEWeatherType::LightSnow:
		return FLinearColor(0.6f, 0.65f, 0.75f, 1.0f); // Soft grey
	case ESEEWeatherType::Aurora:
		return FLinearColor(0.2f, 0.8f, 0.4f, 1.0f);   // Green aurora glow
	case ESEEWeatherType::ClearSky:
		return FLinearColor(0.1f, 0.12f, 0.2f, 1.0f);   // Dark night sky
	}
	return FLinearColor::White;
}
