// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEVFXSubsystem.cpp - Global VFX state management

#include "SEEVFXSubsystem.h"
#include "SEEWeatherVFXComponent.h"

void USEEVFXSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USEEVFXSubsystem::Deinitialize()
{
	RegisteredWeatherComponents.Empty();
	Super::Deinitialize();
}

void USEEVFXSubsystem::SetGlobalWeather(ESEEWeatherType Weather)
{
	if (GlobalWeather == Weather) return;
	GlobalWeather = Weather;
	BroadcastWeatherToComponents();
}

void USEEVFXSubsystem::SetGlobalWindParameters(FVector Direction, float Speed)
{
	GlobalWindDirection = Direction.GetSafeNormal();
	GlobalWindSpeed = Speed;

	for (auto It = RegisteredWeatherComponents.CreateIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			It->Get()->SetWindParameters(GlobalWindDirection, GlobalWindSpeed);
		}
		else
		{
			It.RemoveCurrent();
		}
	}
}

void USEEVFXSubsystem::SetParticleDensityScale(float Scale)
{
	ParticleDensityScale = FMath::Clamp(Scale, 0.5f, 1.5f);
}

void USEEVFXSubsystem::SetMaxConcurrentEmitters(int32 Max)
{
	MaxConcurrentEmitters = FMath::Clamp(Max, 5, 30);
}

void USEEVFXSubsystem::RegisterWeatherComponent(USEEWeatherVFXComponent* Component)
{
	if (!Component) return;
	RegisteredWeatherComponents.AddUnique(Component);

	// Sync new component to current global state
	Component->SetWeather(GlobalWeather);
	Component->SetWindParameters(GlobalWindDirection, GlobalWindSpeed);
}

void USEEVFXSubsystem::UnregisterWeatherComponent(USEEWeatherVFXComponent* Component)
{
	RegisteredWeatherComponents.RemoveAll([Component](const TWeakObjectPtr<USEEWeatherVFXComponent>& Weak)
	{
		return !Weak.IsValid() || Weak.Get() == Component;
	});
}

void USEEVFXSubsystem::BroadcastWeatherToComponents()
{
	for (auto It = RegisteredWeatherComponents.CreateIterator(); It; ++It)
	{
		if (It->IsValid())
		{
			It->Get()->SetWeather(GlobalWeather);
		}
		else
		{
			It.RemoveCurrent();
		}
	}
}
