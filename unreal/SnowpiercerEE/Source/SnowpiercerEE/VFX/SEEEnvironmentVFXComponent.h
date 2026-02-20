// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEEnvironmentVFXComponent.h - Placeable environmental VFX: steam, sparks, weather, flickers

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "SEEEnvironmentVFXComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UPointLightComponent;

/**
 * USEEEnvironmentVFXComponent
 *
 * Place on actors in the world to create environmental visual effects.
 * Handles steam vents, electrical sparks, weather through windows,
 * and fluorescent light flicker patterns.
 *
 * Each instance runs a single primary effect type. For compound effects
 * (e.g., steam vent + sparks), use multiple components or actors.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEEnvironmentVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEEnvironmentVFXComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Primary Effect ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
	ESEEEnvironmentVFXType EffectType = ESEEEnvironmentVFXType::SteamVent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Environment")
	FSEEVFXEmitterConfig EmitterConfig;

	// --- Activation ---

	// Whether effect starts active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Activation")
	bool bStartActive = true;

	// If true, effect activates/deactivates on a timer cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Activation")
	bool bCyclic = false;

	// Duration effect is active per cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Activation",
		meta = (EditCondition = "bCyclic"))
	float ActiveDuration = 3.0f;

	// Duration effect is inactive per cycle
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Activation",
		meta = (EditCondition = "bCyclic"))
	float InactiveDuration = 5.0f;

	// Random offset to stagger multiple cyclic effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Activation",
		meta = (EditCondition = "bCyclic"))
	float CycleRandomOffset = 2.0f;

	// --- Steam-Specific ---

	// Warning hiss delay before steam burst (from COMBAT_DESIGN.md: 1.5s warning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam",
		meta = (EditCondition = "EffectType == ESEEEnvironmentVFXType::SteamVent"))
	float SteamWarningDelay = 1.5f;

	// Whether this steam vent deals damage (environmental hazard)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam")
	bool bDealsDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Steam",
		meta = (EditCondition = "bDealsDamage"))
	float DamagePerSecond = 10.0f;

	// --- Lighting Flicker ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
	FSEEFlickerConfig FlickerConfig;

	// Light components to drive (set in editor or found automatically)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
	TArray<TObjectPtr<UPointLightComponent>> FlickerTargetLights;

	// Auto-discover lights on owner actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Flicker")
	bool bAutoDiscoverLights = true;

	// --- Interface ---

	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void ActivateEffect();

	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void DeactivateEffect();

	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void SetEffectIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category = "VFX|Environment")
	bool IsEffectActive() const { return bIsActive; }

	UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
	void TriggerBurst();

protected:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveNiagara;

private:
	bool bIsActive = false;
	float CycleTimer = 0.0f;
	float CurrentIntensity = 1.0f;

	// Flicker state
	float FlickerTimer = 0.0f;
	float NextFlickerTime = 0.0f;
	bool bFlickerActive = false;
	float FlickerEventTimer = 0.0f;
	TArray<float> BaseLightIntensities;

	void SpawnNiagaraEffect();
	void DestroyNiagaraEffect();
	void UpdateCyclic(float DeltaTime);
	void UpdateFlicker(float DeltaTime);
	void DiscoverLights();
	float GetNextFlickerInterval() const;
};
