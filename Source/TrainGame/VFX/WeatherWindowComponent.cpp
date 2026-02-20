// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "WeatherWindowComponent.h"
#include "VFXSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/LightComponent.h"
#include "Engine/World.h"

UWeatherWindowComponent::UWeatherWindowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UWeatherWindowComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		VFXSubsystemRef = World->GetSubsystem<UVFXSubsystem>();

		if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
		{
			VFX->OnWeatherChanged.AddDynamic(this, &UWeatherWindowComponent::OnWeatherChanged);
			ApplyWeather(VFX->GetCurrentWeather());
		}
	}
}

void UWeatherWindowComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		VFX->OnWeatherChanged.RemoveDynamic(this, &UWeatherWindowComponent::OnWeatherChanged);
	}

	ClearActiveWeatherEffect();
	Super::EndPlay(EndPlayReason);
}

void UWeatherWindowComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateFrost(DeltaTime);
}

EWeatherType UWeatherWindowComponent::GetCurrentWeather() const
{
	if (bHasWeatherOverride)
	{
		return WeatherOverride;
	}
	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		return VFX->GetCurrentWeather();
	}
	return EWeatherType::Blizzard;
}

void UWeatherWindowComponent::SetWeatherOverride(EWeatherType Weather)
{
	bHasWeatherOverride = true;
	WeatherOverride = Weather;
	ApplyWeather(Weather);
}

void UWeatherWindowComponent::ClearWeatherOverride()
{
	bHasWeatherOverride = false;

	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		ApplyWeather(VFX->GetCurrentWeather());
	}
}

void UWeatherWindowComponent::SetFrostIntensity(float Intensity)
{
	TargetFrost = FMath::Clamp(Intensity, 0.f, 1.f);
}

void UWeatherWindowComponent::OnWeatherChanged(EWeatherType NewWeather, EWeatherType OldWeather)
{
	if (!bHasWeatherOverride)
	{
		ApplyWeather(NewWeather);
	}
}

void UWeatherWindowComponent::ApplyWeather(EWeatherType Weather)
{
	if (Weather == LastAppliedWeather) return;
	LastAppliedWeather = Weather;

	ClearActiveWeatherEffect();

	switch (Weather)
	{
	case EWeatherType::Blizzard:
		if (BlizzardEffect) SpawnWeatherEffect(BlizzardEffect);
		TargetFrost = 0.7f;
		break;

	case EWeatherType::Aurora:
		if (AuroraEffect) SpawnWeatherEffect(AuroraEffect);
		TargetFrost = 0.3f;
		break;

	case EWeatherType::ClearSky:
		// No particle effect — just light color
		TargetFrost = 0.1f;
		break;

	case EWeatherType::WhiteOut:
		if (WhiteOutEffect) SpawnWeatherEffect(WhiteOutEffect);
		TargetFrost = 0.9f;
		break;

	case EWeatherType::NightSky:
		TargetFrost = 0.2f;
		break;
	}

	// Update exterior light color
	if (AActor* Owner = GetOwner())
	{
		UActorComponent* LightActorComp = Owner->FindComponentByTag<ULightComponent>(ExteriorLightComponentName);
		ULightComponent* LightComp = Cast<ULightComponent>(LightActorComp);
		if (LightComp)
		{
			FLinearColor LightColor;
			switch (Weather)
			{
			case EWeatherType::Blizzard:	LightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.f); break;
			case EWeatherType::Aurora:		LightColor = FLinearColor(0.3f, 0.9f, 0.5f, 1.f); break;
			case EWeatherType::ClearSky:	LightColor = ClearSkyColor; break;
			case EWeatherType::WhiteOut:	LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.f); break;
			case EWeatherType::NightSky:	LightColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.f); break;
			}
			LightComp->SetLightColor(LightColor);
		}
	}
}

void UWeatherWindowComponent::SpawnWeatherEffect(UNiagaraSystem* Effect)
{
	if (!Effect || !GetOwner()) return;

	ActiveWeatherComp = NewObject<UNiagaraComponent>(GetOwner());
	if (!ActiveWeatherComp) return;

	ActiveWeatherComp->SetAsset(Effect);
	ActiveWeatherComp->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform);
	ActiveWeatherComp->RegisterComponent();
	ActiveWeatherComp->SetVectorParameter(FName("VolumeExtent"), ParticleVolumeExtent);
	ActiveWeatherComp->Activate(true);
}

void UWeatherWindowComponent::ClearActiveWeatherEffect()
{
	if (ActiveWeatherComp)
	{
		ActiveWeatherComp->DestroyComponent();
		ActiveWeatherComp = nullptr;
	}
}

void UWeatherWindowComponent::UpdateFrost(float DeltaTime)
{
	if (FMath::IsNearlyEqual(CurrentFrost, TargetFrost, 0.01f)) return;

	CurrentFrost = FMath::FInterpTo(CurrentFrost, TargetFrost, DeltaTime, 2.f);

	// Update frost material parameter on window mesh
	if (AActor* Owner = GetOwner())
	{
		UMeshComponent* Mesh = Owner->FindComponentByClass<UMeshComponent>();
		if (Mesh)
		{
			Mesh->SetScalarParameterValueOnMaterials(FName("FrostIntensity"), CurrentFrost);
		}
	}
}
