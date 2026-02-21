// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioTypes.h"
#include "ZoneAudioSubsystem.generated.h"

class UZoneAudioProfile;
class UAdaptiveMusicComponent;
class UTrainAmbienceComponent;

// ============================================================================
// Zone Audio Subsystem — Central manager for the train's sonic landscape
// ============================================================================

/**
 * UZoneAudioSubsystem
 *
 * Game instance subsystem that manages the overall audio landscape of the train.
 * Responsibilities:
 * - Maps car numbers to audio zones
 * - Owns and distributes zone audio profiles
 * - Coordinates zone transitions between adaptive music and train ambience
 * - Manages the "train constant" (the ever-present train heartbeat at 72 BPM)
 * - Handles cross-zone crossfade stingers
 *
 * This is the single point of contact for gameplay systems that need to
 * inform the audio system about state changes (zone transitions, combat, etc.)
 */
UCLASS()
class TRAINGAME_API UZoneAudioSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Zone Mapping ---

	/** Get the audio zone for a given car number */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	static EAudioZone GetZoneForCar(int32 CarNumber);

	/** Get the car range for a zone */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	static FInt32Range GetCarRangeForZone(EAudioZone Zone);

	/** Get human-readable zone name */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	static FString GetZoneName(EAudioZone Zone);

	// --- Profile Management ---

	/** Register a zone audio profile */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void RegisterZoneProfile(UZoneAudioProfile* Profile);

	/** Get the profile for a zone */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	UZoneAudioProfile* GetZoneProfile(EAudioZone Zone) const;

	// --- Active Component Registration ---

	/** Register the player's adaptive music component */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void RegisterMusicComponent(UAdaptiveMusicComponent* Component);

	/** Register the player's train ambience component */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void RegisterAmbienceComponent(UTrainAmbienceComponent* Component);

	// --- Zone Transition ---

	/** Notify that the player has entered a new car (triggers zone check) */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void OnPlayerEnteredCar(int32 CarNumber);

	/** Get the current audio zone */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	EAudioZone GetCurrentZone() const { return CurrentZone; }

	/** Get current car number */
	UFUNCTION(BlueprintPure, Category = "Zone Audio")
	int32 GetCurrentCar() const { return CurrentCarNumber; }

	// --- Global Audio State ---

	/** Set global music volume (0.0-1.0) */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void SetMusicVolume(float Volume);

	/** Set global ambience volume (0.0-1.0) */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void SetAmbienceVolume(float Volume);

	/** Set global SFX volume (0.0-1.0) */
	UFUNCTION(BlueprintCallable, Category = "Zone Audio")
	void SetSFXVolume(float Volume);

	float GetMusicVolume() const { return MusicVolume; }
	float GetAmbienceVolume() const { return AmbienceVolume; }
	float GetSFXVolume() const { return SFXVolume; }

private:
	/** Registered zone profiles */
	UPROPERTY()
	TMap<EAudioZone, UZoneAudioProfile*> ZoneProfiles;

	/** Active music component (player) */
	UPROPERTY()
	UAdaptiveMusicComponent* ActiveMusicComponent = nullptr;

	/** Active ambience component (player) */
	UPROPERTY()
	UTrainAmbienceComponent* ActiveAmbienceComponent = nullptr;

	EAudioZone CurrentZone = EAudioZone::None;
	int32 CurrentCarNumber = 0;

	float MusicVolume = 0.8f;
	float AmbienceVolume = 0.8f;
	float SFXVolume = 1.0f;

	/** Crossfade stingers for zone boundaries */
	UPROPERTY()
	TArray<FZoneCrossfadeStinger> CrossfadeStingers;

	void PlayCrossfadeStinger(EAudioZone FromZone, EAudioZone ToZone);
};
