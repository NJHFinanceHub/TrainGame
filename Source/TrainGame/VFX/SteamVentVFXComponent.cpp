// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SteamVentVFXComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

USteamVentVFXComponent::USteamVentVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20Hz is sufficient for steam
}

void USteamVentVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	CreateNiagaraComponent();

	if (bAutoStart)
	{
		AutoStartTimer = FMath::RandRange(AutoStartDelayRange.X, AutoStartDelayRange.Y);
		if (AutoStartTimer <= 0.f)
		{
			StartEmitting();
		}
		else
		{
			bAutoStartPending = true;
		}
	}

	if (bIntermittent)
	{
		IntermittentDuration = FMath::RandRange(IntermittentOnRange.X, IntermittentOnRange.Y);
		IntermittentTimer = 0.f;
		bIntermittentOn = true;
	}
}

void USteamVentVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (NiagaraComp)
	{
		NiagaraComp->DestroyComponent();
		NiagaraComp = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void USteamVentVFXComponent::CreateNiagaraComponent()
{
	if (!Config.SteamEffect || !GetOwner()) return;

	NiagaraComp = NewObject<UNiagaraComponent>(GetOwner());
	if (!NiagaraComp) return;

	NiagaraComp->SetAsset(Config.SteamEffect);
	NiagaraComp->SetAutoActivate(false);
	NiagaraComp->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform);
	NiagaraComp->RegisterComponent();

	// Set initial parameters
	NiagaraComp->SetColorParameter(FName("Color"), Config.ColorTint);
	NiagaraComp->SetFloatParameter(FName("EmissionRate"), Config.EmissionRate);
	NiagaraComp->SetFloatParameter(FName("SpreadAngle"), Config.SpreadAngle);
	NiagaraComp->SetVectorParameter(FName("EmitDirection"), Config.EmitDirection);
}

void USteamVentVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoStartPending)
	{
		AutoStartTimer -= DeltaTime;
		if (AutoStartTimer <= 0.f)
		{
			bAutoStartPending = false;
			StartEmitting();
		}
	}

	if (bIntermittent && bIsEmitting)
	{
		TickIntermittent(DeltaTime);
	}

	if (bBurstActive)
	{
		TickBurst(DeltaTime);
	}
}

void USteamVentVFXComponent::StartEmitting()
{
	if (bIsEmitting) return;
	bIsEmitting = true;

	if (NiagaraComp)
	{
		NiagaraComp->Activate(true);
	}
}

void USteamVentVFXComponent::StopEmitting()
{
	if (!bIsEmitting) return;
	bIsEmitting = false;

	if (NiagaraComp)
	{
		NiagaraComp->Deactivate();
	}
}

void USteamVentVFXComponent::FireBurst(float BurstDuration)
{
	bBurstActive = true;
	BurstTimer = BurstDuration;

	if (NiagaraComp)
	{
		// Spike emission rate for burst
		NiagaraComp->SetFloatParameter(FName("EmissionRate"), Config.EmissionRate * 3.f);
		NiagaraComp->Activate(true);
	}
}

void USteamVentVFXComponent::SetEmissionRate(float NewRate)
{
	Config.EmissionRate = NewRate;
	if (NiagaraComp)
	{
		NiagaraComp->SetFloatParameter(FName("EmissionRate"), NewRate);
	}
}

void USteamVentVFXComponent::TickIntermittent(float DeltaTime)
{
	IntermittentTimer += DeltaTime;

	if (IntermittentTimer >= IntermittentDuration)
	{
		IntermittentTimer = 0.f;
		bIntermittentOn = !bIntermittentOn;

		if (bIntermittentOn)
		{
			IntermittentDuration = FMath::RandRange(IntermittentOnRange.X, IntermittentOnRange.Y);
			if (NiagaraComp) NiagaraComp->Activate(true);
		}
		else
		{
			IntermittentDuration = FMath::RandRange(IntermittentOffRange.X, IntermittentOffRange.Y);
			if (NiagaraComp) NiagaraComp->Deactivate();
		}
	}
}

void USteamVentVFXComponent::TickBurst(float DeltaTime)
{
	BurstTimer -= DeltaTime;

	if (BurstTimer <= 0.f)
	{
		bBurstActive = false;

		if (NiagaraComp)
		{
			// Restore normal emission rate
			NiagaraComp->SetFloatParameter(FName("EmissionRate"), Config.EmissionRate);

			if (!bIsEmitting)
			{
				NiagaraComp->Deactivate();
			}
		}
	}
}
