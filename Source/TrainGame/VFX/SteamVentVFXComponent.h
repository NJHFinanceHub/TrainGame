// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "SteamVentVFXComponent.generated.h"

class UNiagaraComponent;

// ============================================================================
// USteamVentVFXComponent
//
// Attach to pipe joints, vents, kitchen equipment, and steam infrastructure.
// Spawns continuous or burst Niagara steam/smoke particles. Can be triggered
// by environmental hazards or run as ambient background effects.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API USteamVentVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USteamVentVFXComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Start emitting steam */
	UFUNCTION(BlueprintCallable, Category = "VFX|Steam")
	void StartEmitting();

	/** Stop emitting steam */
	UFUNCTION(BlueprintCallable, Category = "VFX|Steam")
	void StopEmitting();

	/** Fire a single burst of steam (for pipe breaks, valve releases) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Steam")
	void FireBurst(float BurstDuration = 1.5f);

	/** Is the emitter currently active? */
	UFUNCTION(BlueprintPure, Category = "VFX|Steam")
	bool IsEmitting() const { return bIsEmitting; }

	/** Set emission rate at runtime */
	UFUNCTION(BlueprintCallable, Category = "VFX|Steam")
	void SetEmissionRate(float NewRate);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Steam emitter configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam")
	FSteamEmitterConfig Config;

	/** Start emitting on BeginPlay? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam")
	bool bAutoStart = true;

	/** Random delay range before auto-start (seconds) — prevents all vents starting in sync */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam")
	FVector2D AutoStartDelayRange = FVector2D(0.f, 2.f);

	/** Intermittent mode: cycle on/off with random intervals */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam")
	bool bIntermittent = false;

	/** On duration range for intermittent mode (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam", meta = (EditCondition = "bIntermittent"))
	FVector2D IntermittentOnRange = FVector2D(3.f, 8.f);

	/** Off duration range for intermittent mode (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steam", meta = (EditCondition = "bIntermittent"))
	FVector2D IntermittentOffRange = FVector2D(2.f, 6.f);

private:
	UPROPERTY()
	UNiagaraComponent* NiagaraComp = nullptr;

	bool bIsEmitting = false;
	float AutoStartTimer = 0.f;
	bool bAutoStartPending = false;

	// Intermittent state
	bool bIntermittentOn = true;
	float IntermittentTimer = 0.f;
	float IntermittentDuration = 0.f;

	// Burst state
	bool bBurstActive = false;
	float BurstTimer = 0.f;

	void CreateNiagaraComponent();
	void TickIntermittent(float DeltaTime);
	void TickBurst(float DeltaTime);
};
