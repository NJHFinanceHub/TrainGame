// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SparkVFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"

USparkVFXComponent::USparkVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void USparkVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStart)
	{
		SetAmbientSparking(true);
	}
}

void USparkVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bAmbientSparkingEnabled) return;

	NextBurstTimer -= DeltaTime;
	if (NextBurstTimer <= 0.f)
	{
		FireSparkBurst();
		ScheduleNextBurst();
	}
}

void USparkVFXComponent::FireSparkBurst()
{
	if (!Config.SparkEffect || !GetOwner()) return;

	const FVector Location = GetOwner()->GetActorLocation();
	const FRotator Rotation = GetOwner()->GetActorRotation();

	UWorld* World = GetWorld();
	if (!World) return;

	UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, Config.SparkEffect, Location, Rotation,
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);

	if (NC)
	{
		NC->SetIntParameter(FName("SparkCount"), Config.SparkCount);
		NC->SetFloatParameter(FName("SparkVelocity"), Config.SparkVelocity);
		NC->SetColorParameter(FName("SparkColor"), Config.SparkColor);
		NC->SetFloatParameter(FName("LifetimeMin"), Config.LifetimeRange.X);
		NC->SetFloatParameter(FName("LifetimeMax"), Config.LifetimeRange.Y);
	}

	if (bEmitLight)
	{
		// Spawn a brief point light flash — auto-destroys via timer
		UPointLightComponent* Light = NewObject<UPointLightComponent>(GetOwner());
		if (Light)
		{
			Light->SetWorldLocation(Location);
			Light->SetIntensity(LightIntensity);
			Light->SetAttenuationRadius(LightRadius);
			Light->SetLightColor(FLinearColor(Config.SparkColor));
			Light->RegisterComponent();

			FTimerHandle TimerHandle;
			World->GetTimerManager().SetTimer(TimerHandle, [Light]()
			{
				if (Light && Light->IsValidLowLevel())
				{
					Light->DestroyComponent();
				}
			}, 0.15f, false);
		}
	}
}

void USparkVFXComponent::FireSparkBurstAtLocation(FVector Location, FVector Normal)
{
	if (!Config.SparkEffect) return;

	UWorld* World = GetWorld();
	if (!World) return;

	const FRotator Rotation = Normal.Rotation();

	UNiagaraComponent* NC = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, Config.SparkEffect, Location, Rotation,
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);

	if (NC)
	{
		NC->SetIntParameter(FName("SparkCount"), Config.SparkCount);
		NC->SetFloatParameter(FName("SparkVelocity"), Config.SparkVelocity);
		NC->SetColorParameter(FName("SparkColor"), Config.SparkColor);
	}
}

void USparkVFXComponent::SetAmbientSparking(bool bEnabled)
{
	bAmbientSparkingEnabled = bEnabled;
	if (bEnabled)
	{
		ScheduleNextBurst();
	}
}

void USparkVFXComponent::ScheduleNextBurst()
{
	NextBurstTimer = FMath::RandRange(Config.BurstIntervalRange.X, Config.BurstIntervalRange.Y);
}
