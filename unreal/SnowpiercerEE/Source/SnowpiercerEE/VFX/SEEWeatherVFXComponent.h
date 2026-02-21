// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEWeatherVFXComponent.h - Weather visible through windows: blizzard, aurora, clear sky

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "SEEWeatherVFXComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeatherChanged, ESEEWeatherType, OldWeather, ESEEWeatherType, NewWeather);

/**
 * USEEWeatherVFXComponent
 *
 * Attach to window actors alongside UWindowViewComponent. Renders weather
 * effects visible through train windows: blizzard snow particles, aurora
 * color shifts, clear sky lighting. Uses a combination of Niagara particles
 * outside the window and dynamic material parameters on the glass.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEWeatherVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEWeatherVFXComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Configuration ---

	// Current weather type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	ESEEWeatherType CurrentWeather = ESEEWeatherType::Blizzard;

	// Niagara systems for each weather type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	TSoftObjectPtr<UNiagaraSystem> BlizzardSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	TSoftObjectPtr<UNiagaraSystem> HeavySnowSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	TSoftObjectPtr<UNiagaraSystem> LightSnowSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	TSoftObjectPtr<UNiagaraSystem> AuroraSystem;

	// Wind direction for particle drift (world space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	FVector WindDirection = FVector(-1.0f, 0.3f, -0.1f);

	// Wind speed multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	float WindSpeed = 1.0f;

	// Glass mesh to apply frost/weather material to
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather")
	TObjectPtr<UStaticMeshComponent> WindowGlassMesh;

	// Frost coverage (0 = clear, 1 = fully frosted)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Weather",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FrostCoverage = 0.8f;

	// --- Interface ---

	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetWeather(ESEEWeatherType NewWeather);

	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetFrostCoverage(float Coverage);

	UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
	void SetWindParameters(FVector Direction, float Speed);

	UFUNCTION(BlueprintPure, Category = "VFX|Weather")
	ESEEWeatherType GetCurrentWeather() const { return CurrentWeather; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "VFX|Weather")
	FOnWeatherChanged OnWeatherChanged;

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveWeatherNiagara;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> GlassMID;

private:
	ESEEWeatherType PreviousWeather = ESEEWeatherType::Blizzard;
	float WeatherTransitionAlpha = 1.0f;
	float TargetFrost = 0.8f;
	float CurrentFrost = 0.0f;

	void SpawnWeatherEffect();
	void DestroyWeatherEffect();
	void UpdateGlassMaterial(float DeltaTime);
	UNiagaraSystem* GetSystemForWeather(ESEEWeatherType Weather) const;
	FLinearColor GetAmbientColorForWeather(ESEEWeatherType Weather) const;
};
