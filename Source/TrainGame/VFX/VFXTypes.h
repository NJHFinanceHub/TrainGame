// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "VFXTypes.generated.h"

// ============================================================================
// VFX System Type Definitions
// Snowpiercer: Eternal Engine — Visual Effects System
// ============================================================================

/** VFX zone identifiers matching the train's visual regions */
UENUM(BlueprintType)
enum class EVFXZone : uint8
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

/** Weather types visible through windows */
UENUM(BlueprintType)
enum class EWeatherType : uint8
{
	Blizzard		UMETA(DisplayName = "Blizzard"),
	Aurora			UMETA(DisplayName = "Aurora Borealis"),
	ClearSky		UMETA(DisplayName = "Clear Sky"),
	WhiteOut		UMETA(DisplayName = "White-Out"),
	NightSky		UMETA(DisplayName = "Night Sky")
};

/** Kronole visual intensity stages */
UENUM(BlueprintType)
enum class EKronoleVFXIntensity : uint8
{
	None			UMETA(DisplayName = "None"),
	Onset			UMETA(DisplayName = "Onset — Subtle edges"),
	Peak			UMETA(DisplayName = "Peak — Full distortion"),
	Comedown		UMETA(DisplayName = "Comedown — Fading"),
	Withdrawal		UMETA(DisplayName = "Withdrawal — Jittery, desaturated")
};

/** Flicker pattern for light fixtures */
UENUM(BlueprintType)
enum class EFlickerPattern : uint8
{
	Steady			UMETA(DisplayName = "Steady (no flicker)"),
	Occasional		UMETA(DisplayName = "Occasional — Random single blinks"),
	Nervous			UMETA(DisplayName = "Nervous — Rapid irregular stuttering"),
	Dying			UMETA(DisplayName = "Dying — Long offs, brief ons"),
	Strobing		UMETA(DisplayName = "Strobing — Regular on/off pulsing")
};

/** Combat impact surface types for spark/splatter selection */
UENUM(BlueprintType)
enum class EImpactSurface : uint8
{
	Metal			UMETA(DisplayName = "Metal"),
	Flesh			UMETA(DisplayName = "Flesh"),
	Wood			UMETA(DisplayName = "Wood"),
	Glass			UMETA(DisplayName = "Glass"),
	Fabric			UMETA(DisplayName = "Fabric")
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** Configuration for a steam/smoke emitter point */
USTRUCT(BlueprintType)
struct FSteamEmitterConfig
{
	GENERATED_BODY()

	/** Niagara system to spawn for this emitter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam")
	UNiagaraSystem* SteamEffect = nullptr;

	/** Emission rate (particles per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam", meta = (ClampMin = "1.0", ClampMax = "500.0"))
	float EmissionRate = 30.f;

	/** Steam velocity direction (local space) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam")
	FVector EmitDirection = FVector(0.f, 0.f, 1.f);

	/** Steam spread angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float SpreadAngle = 15.f;

	/** Base color tint for the steam/smoke */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam")
	FLinearColor ColorTint = FLinearColor(0.8f, 0.8f, 0.85f, 0.6f);

	/** Whether this emitter is a burst (pipe leak) or continuous (vent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam")
	bool bBurstMode = false;
};

/** Configuration for spark emitter behavior */
USTRUCT(BlueprintType)
struct FSparkConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks")
	UNiagaraSystem* SparkEffect = nullptr;

	/** Number of spark particles per burst */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks", meta = (ClampMin = "1", ClampMax = "100"))
	int32 SparkCount = 12;

	/** Spark velocity (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks", meta = (ClampMin = "10.0"))
	float SparkVelocity = 400.f;

	/** Spark lifetime range (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks")
	FVector2D LifetimeRange = FVector2D(0.1f, 0.4f);

	/** Spark color — hot metal orange by default */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks")
	FLinearColor SparkColor = FLinearColor(1.f, 0.6f, 0.1f, 1.f);

	/** Min/max interval between spark bursts for ambient sparking (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Sparks")
	FVector2D BurstIntervalRange = FVector2D(2.f, 8.f);
};

/** Configuration for combat impact VFX */
USTRUCT(BlueprintType)
struct FImpactVFXConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	EImpactSurface Surface = EImpactSurface::Metal;

	/** Niagara system for this impact type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	UNiagaraSystem* ImpactEffect = nullptr;

	/** Decal material for surface marks (blood splatter, dents, scorch) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	UMaterialInterface* DecalMaterial = nullptr;

	/** Decal size range (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	FVector2D DecalSizeRange = FVector2D(10.f, 30.f);

	/** Decal lifetime before fade (seconds, 0 = permanent) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Impact")
	float DecalLifetime = 30.f;
};

/** Flicker keyframe for light animation */
USTRUCT(BlueprintType)
struct FFlickerKeyframe
{
	GENERATED_BODY()

	/** Time offset within the pattern cycle (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Time = 0.f;

	/** Intensity multiplier at this keyframe (0 = off, 1 = full) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker", meta = (ClampMin = "0.0", ClampMax = "1.5"))
	float Intensity = 1.f;
};

/** Kronole post-process parameter set */
USTRUCT(BlueprintType)
struct FKronolePostProcessParams
{
	GENERATED_BODY()

	/** Chromatic aberration intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float ChromaticAberration = 0.f;

	/** Radial blur intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RadialBlur = 0.f;

	/** Color saturation multiplier (< 1 = desaturated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float Saturation = 1.f;

	/** Color tint overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole")
	FLinearColor ColorTint = FLinearColor(1.f, 1.f, 1.f, 0.f);

	/** Vignette intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float VignetteIntensity = 0.f;

	/** Film grain intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FilmGrain = 0.f;
};
