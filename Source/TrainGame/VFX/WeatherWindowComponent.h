// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "WeatherWindowComponent.generated.h"

class UNiagaraComponent;
class UVFXSubsystem;

// ============================================================================
// UWeatherWindowComponent
//
// Attach to window actors to display exterior weather: blizzard snow particles,
// aurora light, clear sky, or white-out conditions. Manages window frost
// material overlay and exterior light color/intensity.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UWeatherWindowComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeatherWindowComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Force a specific weather type on this window (overrides zone default) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetWeatherOverride(EWeatherType Weather);

	/** Clear per-window override, return to zone default */
	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void ClearWeatherOverride();

	/** Get current effective weather on this window */
	UFUNCTION(BlueprintPure, Category = "VFX|Weather")
	EWeatherType GetCurrentWeather() const;

	/** Set frost intensity on this window (0 = clear, 1 = opaque frost) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetFrostIntensity(float Intensity);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Blizzard snow particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	UNiagaraSystem* BlizzardEffect = nullptr;

	/** Aurora particle/light effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	UNiagaraSystem* AuroraEffect = nullptr;

	/** Clear sky ambient light (no particles, just light color) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FLinearColor ClearSkyColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.f);

	/** White-out blinding snow */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	UNiagaraSystem* WhiteOutEffect = nullptr;

	/** Frost material for window glass overlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	UMaterialInterface* FrostMaterial = nullptr;

	/** Window exterior light component name to drive color from weather */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FName ExteriorLightComponentName = FName("ExteriorLight");

	/** Snow particle spawn volume extent (local space, behind window) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FVector ParticleVolumeExtent = FVector(50.f, 100.f, 100.f);

private:
	UPROPERTY()
	UNiagaraComponent* ActiveWeatherComp = nullptr;

	TWeakObjectPtr<UVFXSubsystem> VFXSubsystemRef;

	bool bHasWeatherOverride = false;
	EWeatherType WeatherOverride = EWeatherType::Blizzard;
	EWeatherType LastAppliedWeather = EWeatherType::Blizzard;
	float CurrentFrost = 0.f;
	float TargetFrost = 0.f;

	void ApplyWeather(EWeatherType Weather);
	void SpawnWeatherEffect(UNiagaraSystem* Effect);
	void ClearActiveWeatherEffect();
	void UpdateFrost(float DeltaTime);

	UFUNCTION()
	void OnWeatherChanged(EWeatherType NewWeather, EWeatherType OldWeather);
};
