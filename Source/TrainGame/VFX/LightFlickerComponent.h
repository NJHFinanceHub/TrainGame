// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "LightFlickerComponent.generated.h"

class ULightComponent;

// ============================================================================
// ULightFlickerComponent
//
// Fluorescent light flicker for Tail sections and damaged areas. Attach to
// any actor with a light component. Supports multiple flicker patterns:
// occasional blinks, nervous stuttering, dying tubes, and strobing.
// Integrates with zone profiles for per-zone flicker density.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API ULightFlickerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULightFlickerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Set flicker pattern at runtime */
	UFUNCTION(BlueprintCallable, Category = "VFX|Lighting")
	void SetFlickerPattern(EFlickerPattern NewPattern);

	/** Get current pattern */
	UFUNCTION(BlueprintPure, Category = "VFX|Lighting")
	EFlickerPattern GetFlickerPattern() const { return Pattern; }

	/** Force the light to a specific state (for scripted events) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Lighting")
	void ForceState(bool bOn, float Duration = -1.f);

	/** Clear forced state */
	UFUNCTION(BlueprintCallable, Category = "VFX|Lighting")
	void ClearForcedState();

	/** Trigger a damage flicker sequence (combat near this light) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Lighting")
	void TriggerDamageFlicker(float Duration = 2.f);

protected:
	virtual void BeginPlay() override;

	/** Which flicker pattern to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	EFlickerPattern Pattern = EFlickerPattern::Occasional;

	/** Base intensity of the light when fully on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	float BaseIntensity = 1.f;

	/** Minimum intensity during flicker (0 = full off) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MinIntensity = 0.f;

	/** Random phase offset to desync lights in the same room */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	bool bRandomPhaseOffset = true;

	/** Buzzing sound during flicker (fluorescent tube buzz) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	USoundBase* FlickerBuzzSound = nullptr;

	/** Color shift during flicker (cold white → warm yellow for dying tubes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	bool bColorShiftOnFlicker = true;

	/** Color when light is flickering/dying */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker", meta = (EditCondition = "bColorShiftOnFlicker"))
	FLinearColor FlickerColor = FLinearColor(1.f, 0.9f, 0.7f, 1.f);

	/** Normal light color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flicker")
	FLinearColor NormalColor = FLinearColor(0.9f, 0.95f, 1.f, 1.f);

private:
	UPROPERTY()
	ULightComponent* TargetLight = nullptr;

	float PhaseOffset = 0.f;
	float FlickerTimer = 0.f;
	float CurrentIntensity = 1.f;

	// Forced state
	bool bForced = false;
	bool bForcedOn = true;
	float ForcedTimer = -1.f;

	// Damage flicker
	bool bDamageFlickering = false;
	float DamageFlickerTimer = 0.f;

	float EvaluateFlicker(float Time) const;
	float EvaluateOccasional(float Time) const;
	float EvaluateNervous(float Time) const;
	float EvaluateDying(float Time) const;
	float EvaluateStrobing(float Time) const;
};
