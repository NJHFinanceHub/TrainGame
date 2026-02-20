// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VFXTypes.h"
#include "ZoneVFXProfile.generated.h"

// ============================================================================
// Zone VFX Profile — Data Asset defining a zone's complete visual effects identity
// Mirrors ZoneAudioProfile: one data asset per zone, configured in the editor.
// ============================================================================

UCLASS(BlueprintType)
class TRAINGAME_API UZoneVFXProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Which zone this profile defines */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	EVFXZone Zone = EVFXZone::None;

	/** Display name for debug/UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	FText ZoneDisplayName;

	// --- Ambient Steam/Smoke ---

	/** Ambient steam emitters active in this zone (pipe leaks, kitchen vents, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Steam")
	TArray<FSteamEmitterConfig> AmbientSteamEmitters;

	/** Overall steam density multiplier for this zone (Tail = heavy, FirstClass = none) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Steam", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float SteamDensityMultiplier = 1.f;

	// --- Ambient Sparks ---

	/** Ambient spark sources (damaged wiring, faulty panels) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Sparks")
	TArray<FSparkConfig> AmbientSparkSources;

	// --- Cold/Breath ---

	/** Whether characters have visible cold breath in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Cold")
	bool bColdBreathVisible = false;

	/** Cold breath Niagara system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Cold", meta = (EditCondition = "bColdBreathVisible"))
	UNiagaraSystem* ColdBreathEffect = nullptr;

	/** Frost overlay material for surfaces near breaches/freezer walls */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Cold")
	UMaterialInterface* FrostOverlayMaterial = nullptr;

	/** Frost particle effect (ice crystals in air) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Cold")
	UNiagaraSystem* FrostParticleEffect = nullptr;

	// --- Combat Impact Profiles ---

	/** Impact VFX per surface type for this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Combat")
	TArray<FImpactVFXConfig> ImpactProfiles;

	/** Blood splatter Niagara system */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Combat")
	UNiagaraSystem* BloodSplatterEffect = nullptr;

	/** Blood decal material for floor/wall splatter */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Combat")
	UMaterialInterface* BloodDecalMaterial = nullptr;

	// --- Kronole Effects ---

	/** Post-process parameters at Kronole peak effect */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Kronole")
	FKronolePostProcessParams KronolePeakParams;

	/** Post-process parameters during withdrawal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Kronole")
	FKronolePostProcessParams KronoleWithdrawalParams;

	/** Kronole particle trail (floating embers/motes during high) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Kronole")
	UNiagaraSystem* KronoleAmbientMotes = nullptr;

	// --- Weather (Windows) ---

	/** Default weather type visible through windows in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Weather")
	EWeatherType DefaultWeather = EWeatherType::Blizzard;

	/** Weather particle system for window exteriors */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Weather")
	UNiagaraSystem* WeatherParticleEffect = nullptr;

	/** Exterior lighting color for window light shafts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Weather")
	FLinearColor ExteriorLightColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

	/** Window frost intensity (0 = clear, 1 = completely frosted) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WindowFrostIntensity = 0.5f;

	// --- Lighting ---

	/** Default flicker pattern for fluorescent lights in this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Lighting")
	EFlickerPattern DefaultFlickerPattern = EFlickerPattern::Steady;

	/** Percentage of lights in this zone that flicker (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Lighting", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FlickerLightPercentage = 0.f;

	/** Base light color temperature for this zone (Kelvin) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Lighting")
	float LightTemperatureKelvin = 4000.f;

	// --- Helpers ---

	/** Get the car range for this zone */
	FInt32Range GetCarRange() const;

	/** Check if a car number belongs to this zone */
	bool ContainsCar(int32 CarNumber) const;

	/** Find impact config for a given surface type */
	const FImpactVFXConfig* FindImpactConfig(EImpactSurface Surface) const;
};
