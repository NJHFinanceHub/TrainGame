// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "KronoleVFXComponent.generated.h"

class UNiagaraComponent;
class UPostProcessComponent;
class UMaterialInstanceDynamic;
class UVFXSubsystem;

// ============================================================================
// UKronoleVFXComponent
//
// Visual distortion effects for Kronole drug use: chromatic aberration,
// radial blur, color desaturation, slow-mo visual cues, and ambient
// floating particle motes. Reads state from KronoleComponent and drives
// post-process + Niagara effects accordingly.
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UKronoleVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKronoleVFXComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Set VFX intensity stage (typically called by KronoleComponent) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Kronole")
	void SetIntensity(EKronoleVFXIntensity NewIntensity);

	/** Get current intensity */
	UFUNCTION(BlueprintPure, Category = "VFX|Kronole")
	EKronoleVFXIntensity GetIntensity() const { return CurrentIntensity; }

	/** Set slow-motion time dilation visual factor */
	UFUNCTION(BlueprintCallable, Category = "VFX|Kronole")
	void SetTimeDilationVisual(float TimeDilation);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Ambient floating mote particles during Kronole high */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kronole|VFX")
	UNiagaraSystem* AmbientMoteEffect = nullptr;

	/** Speed lines / motion blur Niagara system for slow-mo perception */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kronole|VFX")
	UNiagaraSystem* SlowMoLinesEffect = nullptr;

	/** Transition speed between intensity stages (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kronole|VFX")
	float TransitionDuration = 1.5f;

	/** Post-process settings for peak Kronole effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kronole|VFX")
	FKronolePostProcessParams PeakParams;

	/** Post-process settings for withdrawal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kronole|VFX")
	FKronolePostProcessParams WithdrawalParams;

private:
	UPROPERTY()
	UPostProcessComponent* PostProcessComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* MoteComp = nullptr;

	UPROPERTY()
	UNiagaraComponent* SlowMoComp = nullptr;

	EKronoleVFXIntensity CurrentIntensity = EKronoleVFXIntensity::None;
	EKronoleVFXIntensity TargetIntensity = EKronoleVFXIntensity::None;
	float TransitionAlpha = 0.f;
	float TimeDilation = 1.f;

	FKronolePostProcessParams CurrentParams;
	FKronolePostProcessParams SourceParams;
	FKronolePostProcessParams TargetParams;

	void CreatePostProcessComponent();
	void CreateMoteComponent();
	void ApplyPostProcessParams(const FKronolePostProcessParams& Params);
	FKronolePostProcessParams GetParamsForIntensity(EKronoleVFXIntensity Intensity) const;
	FKronolePostProcessParams LerpParams(const FKronolePostProcessParams& A, const FKronolePostProcessParams& B, float Alpha) const;

	TWeakObjectPtr<UVFXSubsystem> VFXSubsystemRef;
};
