// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AudioTypes.generated.h"

// ============================================================================
// Audio System Type Definitions
// Snowpiercer: Eternal Engine - Zone Audio & Adaptive Music
// ============================================================================

/** Audio zone identifiers matching the train's 7 distinct sonic regions */
UENUM(BlueprintType)
enum class EAudioZone : uint8
{
	None			UMETA(DisplayName = "None"),
	Tail			UMETA(DisplayName = "Tail (Cars 1-15)"),
	ThirdClass		UMETA(DisplayName = "Third Class (Cars 16-30)"),
	SecondClass		UMETA(DisplayName = "Second Class (Cars 31-48)"),
	WorkingSpine	UMETA(DisplayName = "Working Spine (Cars 49-62)"),
	FirstClass		UMETA(DisplayName = "First Class (Cars 63-82)"),
	Sanctum			UMETA(DisplayName = "Sanctum (Cars 83-95)"),
	Engine			UMETA(DisplayName = "Engine (Cars 96-103)")
};

/** Adaptive music state — drives layer activation and transitions */
UENUM(BlueprintType)
enum class EMusicState : uint8
{
	Exploration		UMETA(DisplayName = "Exploration"),
	Suspicion		UMETA(DisplayName = "Suspicion"),
	Detection		UMETA(DisplayName = "Detection"),
	Combat			UMETA(DisplayName = "Combat"),
	PostCombat		UMETA(DisplayName = "Post-Combat"),
	MoralChoice		UMETA(DisplayName = "Moral Choice")
};

/** Music layer indices in the adaptive stack (bottom to top) */
UENUM(BlueprintType)
enum class EMusicLayer : uint8
{
	TrainAmbience	UMETA(DisplayName = "Train Ambience"),
	ZoneBase		UMETA(DisplayName = "Zone Base"),
	Exploration		UMETA(DisplayName = "Exploration"),
	Tension			UMETA(DisplayName = "Tension"),
	Combat			UMETA(DisplayName = "Combat"),
	Narrative		UMETA(DisplayName = "Narrative"),
	Count			UMETA(Hidden)
};

/** Ambience sub-layers for the constant train sound bed */
UENUM(BlueprintType)
enum class EAmbienceLayer : uint8
{
	WheelRhythm		UMETA(DisplayName = "Wheel-on-Rail Rhythm"),
	EngineHum		UMETA(DisplayName = "Engine Hum"),
	MetalCreak		UMETA(DisplayName = "Metal Creaking"),
	WindExterior	UMETA(DisplayName = "Exterior Wind"),
	Ventilation		UMETA(DisplayName = "Ventilation/Air"),
	Count			UMETA(Hidden)
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** Configuration for a single music stem within a layer */
USTRUCT(BlueprintType)
struct FStemConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stem")
	USoundBase* Sound = nullptr;

	/** Intensity threshold at which this stem activates (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stem", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActivationThreshold = 0.f;

	/** Volume multiplier for this stem */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stem", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VolumeMultiplier = 1.f;
};

/** Configuration for a music layer (collection of stems) */
USTRUCT(BlueprintType)
struct FMusicLayerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
	EMusicLayer Layer = EMusicLayer::ZoneBase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
	TArray<FStemConfig> Stems;

	/** Crossfade duration when this layer activates (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
	float FadeInDuration = 2.f;

	/** Crossfade duration when this layer deactivates (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Layer")
	float FadeOutDuration = 2.f;
};

/** Transition rules between music states */
USTRUCT(BlueprintType)
struct FMusicTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transition")
	EMusicState FromState = EMusicState::Exploration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transition")
	EMusicState ToState = EMusicState::Suspicion;

	/** Crossfade duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transition")
	float CrossfadeDuration = 2.f;

	/** If true, transition ignores bar quantization for responsiveness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transition")
	bool bEmergencyTransition = false;
};

/** Zone crossfade stinger played at zone boundaries */
USTRUCT(BlueprintType)
struct FZoneCrossfadeStinger
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stinger")
	EAudioZone FromZone = EAudioZone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stinger")
	EAudioZone ToZone = EAudioZone::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Stinger")
	USoundBase* StingerSound = nullptr;
};

/** Runtime state for a playing music layer */
USTRUCT()
struct FMusicLayerState
{
	GENERATED_BODY()

	float CurrentVolume = 0.f;
	float TargetVolume = 0.f;
	float FadeSpeed = 0.f;
	bool bActive = false;

	UPROPERTY()
	TArray<UAudioComponent*> ActiveStemComponents;
};

/** Runtime snapshot of the full adaptive music state */
USTRUCT(BlueprintType)
struct FAdaptiveMusicSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EMusicState CurrentState = EMusicState::Exploration;

	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	EAudioZone CurrentZone = EAudioZone::None;

	/** Overall music intensity (0.0-1.0), drives stem activation */
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	float Intensity = 0.f;

	/** Enemy awareness level driving suspicion/detection (0.0-1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	float EnemyAwareness = 0.f;

	/** Number of active enemies in combat */
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	int32 ActiveEnemyCount = 0;

	/** Player health ratio (0.0-1.0) */
	UPROPERTY(BlueprintReadOnly, Category = "Audio")
	float PlayerHealthRatio = 1.f;
};
