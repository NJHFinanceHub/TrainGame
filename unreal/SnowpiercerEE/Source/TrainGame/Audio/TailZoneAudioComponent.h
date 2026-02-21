// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioTypes.h"
#include "TailZoneAudioComponent.generated.h"

// ============================================================================
// Tail Zone Audio — Proof-of-concept zone-specific audio implementation
// ============================================================================

/**
 * UTailZoneAudioComponent
 *
 * Zone 1 (The Tail, Cars 1-15) sonic identity implementation.
 *
 * Sonic character: Sparse, percussive, industrial
 * - Found-object percussion (pipes, metal sheets, chains)
 * - Body percussion (claps, stomps, chest-beats)
 * - Low drones from processed train rumble
 * - Sparse melodic content (solo voice humming, bowed scrap metal)
 * - Deliberate silence as compositional element
 * - Revolutionary motif: rising 4-note phrase emerging as revolution builds
 *
 * Key: Ambiguous/modal — no clear tonal center
 * Tempo: 72 BPM (locked to wheel-on-rail cadence)
 *
 * This component manages the zone-specific ambient sounds and one-shots
 * that layer on top of the adaptive music system. It handles:
 * - Random ambient one-shot triggers (creaking, chain rattles, dripping)
 * - Environmental audio reactions (player proximity to walls, vents, etc.)
 * - Revolutionary motif emergence based on story progression
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRAINGAME_API UTailZoneAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTailZoneAudioComponent();

	// --- Ambient Loops ---

	/** Constant metal creaking/rattling ambience */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Loops")
	USoundBase* MetalRattleLoop = nullptr;

	/** Wind whistling through gaps in the hull */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Loops")
	USoundBase* WindWhistleLoop = nullptr;

	/** Distant crowd murmur (tail residents) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Loops")
	USoundBase* CrowdMurmurLoop = nullptr;

	/** Low drone from train rumble (processed, musical) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Loops")
	USoundBase* TrainDroneLoop = nullptr;

	// --- One-Shot Ambient Sounds ---

	/** Random pipe clanks and metal impacts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|OneShots")
	TArray<USoundBase*> PipeClankSounds;

	/** Chain rattle and drag sounds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|OneShots")
	TArray<USoundBase*> ChainRattleSounds;

	/** Water dripping from condensation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|OneShots")
	TArray<USoundBase*> WaterDripSounds;

	/** Distant voices, coughs, muttering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|OneShots")
	TArray<USoundBase*> DistantVoiceSounds;

	/** Metal stress groans (structural) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|OneShots")
	TArray<USoundBase*> MetalStressSounds;

	// --- Detection Alarm ---

	/** Pipe-banging alarm rhythm (zone-specific detection) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Detection")
	USoundBase* PipeBangingAlarm = nullptr;

	// --- Revolutionary Motif ---

	/** Rising 4-note revolutionary motif (percussive version for Tail) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Motif")
	USoundBase* RevolutionaryMotif = nullptr;

	/** Revolution progress (0.0-1.0) — affects motif frequency and intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Motif", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RevolutionProgress = 0.f;

	// --- Configuration ---

	/** Minimum interval between random one-shot triggers (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Config")
	float MinOneShotInterval = 3.f;

	/** Maximum interval between random one-shot triggers (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Config")
	float MaxOneShotInterval = 12.f;

	/** Volume range for one-shots (min, max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Config")
	FVector2D OneShotVolumeRange = FVector2D(0.3f, 0.8f);

	/** Pitch variation range for one-shots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Config")
	FVector2D OneShotPitchRange = FVector2D(0.9f, 1.1f);

	/** Maximum distance for spatialized one-shots from player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tail Audio|Config")
	float OneShotMaxDistance = 1500.f;

	// --- Runtime ---

	/** Activate tail zone audio (called when entering zone) */
	UFUNCTION(BlueprintCallable, Category = "Tail Audio")
	void ActivateZoneAudio();

	/** Deactivate tail zone audio (called when leaving zone) */
	UFUNCTION(BlueprintCallable, Category = "Tail Audio")
	void DeactivateZoneAudio(float FadeOutDuration = 2.f);

	/** Set revolution progress (affects motif emergence) */
	UFUNCTION(BlueprintCallable, Category = "Tail Audio")
	void SetRevolutionProgress(float Progress);

	/** Is this zone audio currently active? */
	UFUNCTION(BlueprintPure, Category = "Tail Audio")
	bool IsZoneAudioActive() const { return bIsZoneAudioActive; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bIsZoneAudioActive = false;

	/** Active ambient loop audio components */
	UPROPERTY()
	TArray<UAudioComponent*> ActiveLoops;

	/** Timer for next random one-shot */
	float NextOneShotTimer = 0.f;

	/** Timer for revolutionary motif check */
	float MotifTimer = 0.f;

	/** Cooldown before motif can play again */
	float MotifCooldown = 0.f;

	void ScheduleNextOneShot();
	void TriggerRandomOneShot();
	void TriggerMotifCheck();
	USoundBase* PickRandomSound(const TArray<USoundBase*>& Sounds) const;
	FVector GetRandomSpatialOffset() const;

	/** Start a looping ambient sound */
	UAudioComponent* StartLoop(USoundBase* Sound, float Volume);
};
