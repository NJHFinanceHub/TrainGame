// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioTypes.h"
#include "TrainAmbienceComponent.generated.h"

// ============================================================================
// Train Ambience Component — The constant sonic bed of the train
// ============================================================================

/**
 * UTrainAmbienceComponent
 *
 * Manages the ever-present train sound bed: wheel rhythm, engine hum, metal
 * creaking, exterior wind, and ventilation. These sounds are ALWAYS playing
 * (Layer 0 in the adaptive stack) and only vary in volume/mix based on zone
 * and game state. The 72 BPM wheel-on-rail cadence is the game's metronome.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRAINGAME_API UTrainAmbienceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTrainAmbienceComponent();

	// --- Configuration ---

	/** Wheel-on-rail rhythmic loop (72 BPM base pulse) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Sounds")
	USoundBase* WheelRhythmLoop = nullptr;

	/** Engine hum — varies by proximity to engine car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Sounds")
	USoundBase* EngineHumLoop = nullptr;

	/** Metal creaking — structural stress sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Sounds")
	USoundBase* MetalCreakLoop = nullptr;

	/** Exterior wind — louder near windows, doors, breaches */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Sounds")
	USoundBase* WindExteriorLoop = nullptr;

	/** Ventilation/air system — cleaner in upper classes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Sounds")
	USoundBase* VentilationLoop = nullptr;

	/** Master volume for all train ambience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MasterVolume = 0.8f;

	// --- Zone Mix Presets ---

	/** Per-zone volume multipliers for each ambience sub-layer.
	 *  Index matches EAmbienceLayer order: Wheel, Engine, Creak, Wind, Vent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Train Ambience|Zone Mix")
	TMap<EAudioZone, FVector> ZoneWheelCreakWindMix;

	// --- Runtime API ---

	/** Start all ambience layers */
	UFUNCTION(BlueprintCallable, Category = "Train Ambience")
	void StartAmbience();

	/** Stop all ambience layers */
	UFUNCTION(BlueprintCallable, Category = "Train Ambience")
	void StopAmbience();

	/** Update mix for current zone — call when player changes zone */
	UFUNCTION(BlueprintCallable, Category = "Train Ambience")
	void SetZone(EAudioZone NewZone);

	/** Set overall ambience volume (e.g. reduce during combat) */
	UFUNCTION(BlueprintCallable, Category = "Train Ambience")
	void SetMasterVolume(float Volume);

	/** Get current zone */
	UFUNCTION(BlueprintPure, Category = "Train Ambience")
	EAudioZone GetCurrentZone() const { return CurrentZone; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/** Audio components for each sub-layer */
	UPROPERTY()
	TArray<UAudioComponent*> AmbienceLayers;

	EAudioZone CurrentZone = EAudioZone::None;

	/** Per-layer current and target volumes for smooth fading */
	TArray<float> CurrentVolumes;
	TArray<float> TargetVolumes;

	/** Crossfade speed when changing zones */
	float ZoneCrossfadeSpeed = 0.5f;

	void CreateAmbienceLayer(USoundBase* Sound, float InitialVolume);
	void UpdateVolumeFades(float DeltaTime);
	void ApplyZoneMix(EAudioZone Zone);

	/** Get default mix for a zone (if no custom preset set) */
	void GetDefaultZoneMix(EAudioZone Zone, float& OutWheel, float& OutEngine, float& OutCreak, float& OutWind, float& OutVent) const;
};
