// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFXTypes.h"
#include "VFXSubsystem.generated.h"

class UZoneVFXProfile;
class UNiagaraComponent;

// ============================================================================
// UVFXSubsystem — World subsystem managing zone-based VFX state
//
// Central coordinator for visual effects across the train. Handles zone
// transitions, weather changes, and provides API for combat/gameplay systems
// to request VFX spawning. Mirrors the ZoneAudioSubsystem pattern.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, EWeatherType, NewWeather, EWeatherType, OldWeather);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneVFXChanged, EVFXZone, NewZone, EVFXZone, OldZone);

UCLASS()
class TRAINGAME_API UVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Zone Management ---

	/** Set the active zone — triggers VFX profile swap and transitions */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetActiveZone(EVFXZone NewZone);

	/** Get the currently active zone */
	UFUNCTION(BlueprintPure, Category = "VFX")
	EVFXZone GetActiveZone() const { return ActiveZone; }

	/** Get the active VFX profile */
	UFUNCTION(BlueprintPure, Category = "VFX")
	UZoneVFXProfile* GetActiveProfile() const { return ActiveProfile; }

	/** Register a zone VFX profile (call during level load) */
	UFUNCTION(BlueprintCallable, Category = "VFX")
	void RegisterZoneProfile(UZoneVFXProfile* Profile);

	// --- Weather ---

	/** Override the weather type (for scripted events, time-of-day, etc.) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetWeatherOverride(EWeatherType Weather);

	/** Clear weather override — returns to zone default */
	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void ClearWeatherOverride();

	/** Get current effective weather */
	UFUNCTION(BlueprintPure, Category = "VFX|Weather")
	EWeatherType GetCurrentWeather() const;

	// --- Combat VFX Spawning ---

	/** Spawn impact VFX at a world location */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void SpawnImpactVFX(FVector Location, FRotator ImpactNormal, EImpactSurface Surface);

	/** Spawn blood splatter at location with direction */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity = 1.f);

	// --- Kronole VFX ---

	/** Set Kronole visual intensity (called by KronoleComponent) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Kronole")
	void SetKronoleIntensity(EKronoleVFXIntensity Intensity);

	/** Get current Kronole VFX parameters (interpolated) */
	UFUNCTION(BlueprintPure, Category = "VFX|Kronole")
	FKronolePostProcessParams GetCurrentKronoleParams() const { return CurrentKronoleParams; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "VFX")
	FOnWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category = "VFX")
	FOnZoneVFXChanged OnZoneVFXChanged;

private:
	UPROPERTY()
	TMap<EVFXZone, UZoneVFXProfile*> ZoneProfiles;

	UPROPERTY()
	UZoneVFXProfile* ActiveProfile = nullptr;

	EVFXZone ActiveZone = EVFXZone::None;

	bool bWeatherOverrideActive = false;
	EWeatherType WeatherOverride = EWeatherType::Blizzard;

	EKronoleVFXIntensity KronoleIntensity = EKronoleVFXIntensity::None;
	FKronolePostProcessParams CurrentKronoleParams;

	void TransitionToProfile(UZoneVFXProfile* NewProfile);
	void UpdateKronoleParams(float DeltaTime);

	/** Spawn a decal at location with material from the active profile */
	void SpawnImpactDecal(UMaterialInterface* Material, FVector Location, FRotator Rotation, FVector2D SizeRange, float Lifetime);
};
