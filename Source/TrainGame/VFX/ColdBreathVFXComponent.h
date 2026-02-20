// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "ColdBreathVFXComponent.generated.h"

class UNiagaraComponent;

// ============================================================================
// UColdBreathVFXComponent
//
// Visible breath condensation in cold sections (freezer cars, near window
// breaches, unheated Tail areas). Attaches to character head bone and
// emits breath-synced puffs. Intensity scales with zone temperature.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UColdBreathVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UColdBreathVFXComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Enable cold breath effect */
	UFUNCTION(BlueprintCallable, Category = "VFX|Cold")
	void EnableBreathEffect();

	/** Disable cold breath effect */
	UFUNCTION(BlueprintCallable, Category = "VFX|Cold")
	void DisableBreathEffect();

	/** Set breath intensity (0 = off, 1 = heavy freezer breath) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Cold")
	void SetBreathIntensity(float Intensity);

	/** Set whether character is exerting (running, fighting) for heavier breath */
	UFUNCTION(BlueprintCallable, Category = "VFX|Cold")
	void SetExertion(bool bExerting);

	UFUNCTION(BlueprintPure, Category = "VFX|Cold")
	bool IsBreathVisible() const { return bBreathActive; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Niagara system for breath puffs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	UNiagaraSystem* BreathEffect = nullptr;

	/** Socket/bone name to attach breath emitter to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	FName AttachBoneName = FName("head");

	/** Offset from bone (forward and slightly down from mouth) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	FVector AttachOffset = FVector(15.f, 0.f, -3.f);

	/** Base breath cycle interval (seconds) — inhale/exhale rhythm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	float BreathCycleInterval = 3.f;

	/** Exertion breath cycle (faster breathing when running/fighting) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	float ExertionBreathInterval = 1.2f;

	/** Frost overlay Niagara effect for screen edges in extreme cold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold")
	UNiagaraSystem* FrostScreenEffect = nullptr;

private:
	UPROPERTY()
	UNiagaraComponent* BreathComp = nullptr;

	bool bBreathActive = false;
	bool bExerting = false;
	float BreathIntensity = 1.f;
	float BreathTimer = 0.f;

	void CreateBreathComponent();
	void PulseBreath();
	float GetCurrentBreathInterval() const;
};
