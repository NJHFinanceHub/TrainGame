// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatVFXComponent.h"
#include "VFXSubsystem.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"

UCombatVFXComponent::UCombatVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (World)
	{
		VFXSubsystemRef = World->GetSubsystem<UVFXSubsystem>();
	}
}

void UCombatVFXComponent::PlayHitImpact(FVector HitLocation, FVector HitNormal, EImpactSurface Surface, float DamageAmount)
{
	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		VFX->SpawnImpactVFX(HitLocation, HitNormal.Rotation(), Surface);
	}

	// Blood on flesh hits
	if (Surface == EImpactSurface::Flesh)
	{
		const float Intensity = FMath::Clamp(DamageAmount / 50.f, 0.3f, 1.f);
		PlayBloodSplatter(HitLocation, -HitNormal, Intensity);
	}
}

void UCombatVFXComponent::PlayBloodSplatter(FVector HitLocation, FVector Direction, float Intensity)
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		VFX->SpawnBloodSplatter(HitLocation, Direction, Intensity);
	}
	else if (BloodSplatterOverride)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, BloodSplatterOverride, HitLocation, Direction.Rotation(),
			FVector(Intensity), true, true, ENCPoolMethod::AutoRelease);
	}
}

void UCombatVFXComponent::PlayBlockImpact(FVector ContactPoint, FVector Normal)
{
	if (!BlockSparkEffect) return;

	UWorld* World = GetWorld();
	if (!World) return;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, BlockSparkEffect, ContactPoint, Normal.Rotation(),
		FVector(1.f), true, true, ENCPoolMethod::AutoRelease);
}

void UCombatVFXComponent::PlayEnvironmentalKillVFX(FVector Location, EImpactSurface Surface)
{
	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		// Spawn a larger impact for environmental kills
		VFX->SpawnImpactVFX(Location, FRotator::ZeroRotator, Surface);

		// Extra blood for environmental kills on flesh
		if (Surface == EImpactSurface::Flesh)
		{
			VFX->SpawnBloodSplatter(Location, FVector::DownVector, 1.f);
		}
	}
}

void UCombatVFXComponent::PlayDamageFlash(float DamageRatio)
{
	if (DamageRatio < DamageFlashThreshold) return;

	// The actual screen flash is handled by the HUD widget system.
	// This component just validates the threshold and could trigger
	// additional particle effects (blood drops on screen, etc.).
}
