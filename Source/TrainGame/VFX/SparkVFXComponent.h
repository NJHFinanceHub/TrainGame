// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "SparkVFXComponent.generated.h"

class UNiagaraComponent;

// ============================================================================
// USparkVFXComponent
//
// Electrical spark effects for damaged wiring, faulty panels, and combat
// impacts on metal surfaces. Supports ambient periodic sparking and
// on-demand burst sparks from combat/hazard triggers.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API USparkVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USparkVFXComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Fire a single spark burst */
	UFUNCTION(BlueprintCallable, Category = "VFX|Sparks")
	void FireSparkBurst();

	/** Fire a spark burst at a specific world location (for combat impacts) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Sparks")
	void FireSparkBurstAtLocation(FVector Location, FVector Normal);

	/** Enable/disable ambient periodic sparking */
	UFUNCTION(BlueprintCallable, Category = "VFX|Sparks")
	void SetAmbientSparking(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "VFX|Sparks")
	bool IsAmbientSparkingEnabled() const { return bAmbientSparkingEnabled; }

protected:
	virtual void BeginPlay() override;

	/** Spark configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparks")
	FSparkConfig Config;

	/** Start ambient sparking on BeginPlay? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparks")
	bool bAutoStart = true;

	/** Point light flash on spark burst */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparks")
	bool bEmitLight = true;

	/** Light flash intensity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparks", meta = (EditCondition = "bEmitLight"))
	float LightIntensity = 5000.f;

	/** Light flash radius (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sparks", meta = (EditCondition = "bEmitLight"))
	float LightRadius = 300.f;

private:
	bool bAmbientSparkingEnabled = false;
	float NextBurstTimer = 0.f;

	void ScheduleNextBurst();
};
