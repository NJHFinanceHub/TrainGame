// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// VFXTypes.h - Core type definitions for the visual effects system

#pragma once

#include "CoreMinimal.h"
#include "VFXTypes.generated.h"

class UNiagaraSystem;
class UMaterialInterface;

// Categories of environmental VFX emitters
UENUM(BlueprintType)
enum class ESEEEnvironmentVFXType : uint8
{
	SteamVent       UMETA(DisplayName = "Steam Vent"),
	SteamPipe       UMETA(DisplayName = "Steam Pipe Leak"),
	KitchenSteam    UMETA(DisplayName = "Kitchen Steam"),
	ElectricSparks  UMETA(DisplayName = "Electric Sparks"),
	CombatSparks    UMETA(DisplayName = "Combat Sparks (Metal)"),
	WeldingSparks   UMETA(DisplayName = "Welding Sparks"),
	Dripping        UMETA(DisplayName = "Condensation Drips"),
	Dust            UMETA(DisplayName = "Dust Motes"),
	Smoke           UMETA(DisplayName = "Smoke"),
	Embers          UMETA(DisplayName = "Embers")
};

// Weather conditions visible through windows
UENUM(BlueprintType)
enum class ESEEWeatherType : uint8
{
	Blizzard        UMETA(DisplayName = "Blizzard"),
	HeavySnow      UMETA(DisplayName = "Heavy Snow"),
	LightSnow      UMETA(DisplayName = "Light Snow"),
	Aurora          UMETA(DisplayName = "Aurora Borealis"),
	ClearSky        UMETA(DisplayName = "Clear Sky"),
	WhiteOut        UMETA(DisplayName = "White-Out")
};

// Types of combat hit VFX
UENUM(BlueprintType)
enum class ESEEHitVFXType : uint8
{
	BloodSplatter   UMETA(DisplayName = "Blood Splatter"),
	MetalSparks     UMETA(DisplayName = "Metal Sparks"),
	ClothRip        UMETA(DisplayName = "Cloth Rip"),
	BoneImpact      UMETA(DisplayName = "Bone Impact"),
	Stagger         UMETA(DisplayName = "Stagger Flash")
};

// Kronole visual effect intensity stages
UENUM(BlueprintType)
enum class ESEEKronoleVFXStage : uint8
{
	None            UMETA(DisplayName = "None"),
	Onset           UMETA(DisplayName = "Onset (subtle)"),
	Peak            UMETA(DisplayName = "Peak (full distortion)"),
	Comedown        UMETA(DisplayName = "Comedown (fading)"),
	Withdrawal      UMETA(DisplayName = "Withdrawal (harsh)")
};

// Lighting flicker patterns for different environments
UENUM(BlueprintType)
enum class ESEEFlickerPattern : uint8
{
	Steady          UMETA(DisplayName = "Steady (no flicker)"),
	Fluorescent     UMETA(DisplayName = "Fluorescent Buzz"),
	Damaged         UMETA(DisplayName = "Damaged (erratic)"),
	Emergency       UMETA(DisplayName = "Emergency (red pulse)"),
	Dying           UMETA(DisplayName = "Dying (intermittent)")
};

// Configuration for a single VFX emitter
USTRUCT(BlueprintType)
struct FSEEVFXEmitterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float SpawnRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Scale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FLinearColor ColorTint = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	float Lifetime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bLooping = true;
};

// Configuration for lighting flicker behavior
USTRUCT(BlueprintType)
struct FSEEFlickerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	ESEEFlickerPattern Pattern = ESEEFlickerPattern::Steady;

	// Min intensity during flicker (0-1 relative to base intensity)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinIntensity = 0.0f;

	// Average interval between flickers in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float FlickerInterval = 0.5f;

	// Randomness applied to interval (0 = exact, 1 = fully random)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Randomness = 0.3f;

	// Duration of each flicker event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float FlickerDuration = 0.05f;

	// Color shift during flicker (multiplied with light color)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	FLinearColor FlickerColor = FLinearColor::White;
};

// Blood/damage decal configuration
USTRUCT(BlueprintType)
struct FSEEDecalConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	TSoftObjectPtr<UMaterialInterface> DecalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	FVector DecalSize = FVector(50.0f, 50.0f, 50.0f);

	// How long the decal persists before fading (0 = permanent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	float FadeDelay = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decal")
	float FadeDuration = 5.0f;
};
