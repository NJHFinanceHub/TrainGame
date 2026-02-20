// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AudioTypes.h"
#include "ZoneAudioProfile.generated.h"

// ============================================================================
// Zone Audio Profile — Data Asset defining a zone's complete sonic identity
// ============================================================================

/**
 * UZoneAudioProfile
 *
 * Configurable data asset that defines the complete sonic identity for a train zone.
 * Each zone (Tail, Third Class, etc.) has one of these defining its music, ambience,
 * and transition behavior. Create instances in the editor per zone.
 */
UCLASS(BlueprintType)
class TRAINGAME_API UZoneAudioProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Which zone this profile defines */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	EAudioZone Zone = EAudioZone::None;

	/** Display name for debug/UI */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone")
	FText ZoneDisplayName;

	// --- Musical Identity ---

	/** Base tempo in BPM (Tail=72, ThirdClass=80-90, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music")
	float BaseTempoBPM = 72.f;

	/** Musical key description (for documentation/debug) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music")
	FString MusicalKey;

	// --- Music Layers ---

	/** Zone base layer — always playing at minimum volume */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music|Layers")
	FMusicLayerConfig ZoneBaseLayer;

	/** Exploration melodic content */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music|Layers")
	FMusicLayerConfig ExplorationLayer;

	/** Tension/suspicion layer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music|Layers")
	FMusicLayerConfig TensionLayer;

	/** Combat music layer */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Music|Layers")
	FMusicLayerConfig CombatLayer;

	// --- Ambience ---

	/** Ambient sound loops for this zone (layered on top of train constant) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Ambience")
	TArray<USoundBase*> AmbientLoops;

	/** Zone-specific one-shot ambient sounds (random triggers) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Ambience")
	TArray<USoundBase*> AmbientOneShots;

	/** Min/max interval for random one-shot triggers (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Ambience")
	FVector2D OneShotIntervalRange = FVector2D(5.f, 15.f);

	/** Volume multiplier for zone ambience relative to global */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Ambience", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float AmbienceVolumeMultiplier = 1.f;

	// --- Detection Audio ---

	/** Zone-specific detection alarm sound (pipe banging, factory whistle, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Detection")
	USoundBase* DetectionAlarmSound = nullptr;

	/** Zone-specific chase percussion variant */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Detection")
	USoundBase* ChaseMusicVariant = nullptr;

	// --- Transition ---

	/** Duration of crossfade when entering this zone (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Transition")
	float ZoneEntryCrossfadeDuration = 3.f;

	/** Stinger sound played when entering this zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|Transition")
	USoundBase* ZoneEntryStinger = nullptr;

	// --- Post-Combat ---

	/** Post-combat wind-down sound (tinged with cost, zone-appropriate) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|PostCombat")
	USoundBase* PostCombatSound = nullptr;

	/** Post-combat mood: mercy shown */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|PostCombat")
	USoundBase* PostCombatMercyResolution = nullptr;

	/** Post-combat mood: all enemies killed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zone|PostCombat")
	USoundBase* PostCombatLethalResolution = nullptr;

	// --- Helpers ---

	/** Get the car range for this zone */
	FInt32Range GetCarRange() const;

	/** Check if a car number belongs to this zone */
	bool ContainsCar(int32 CarNumber) const;
};
