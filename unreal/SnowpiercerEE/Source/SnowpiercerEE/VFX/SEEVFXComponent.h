// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEVFXComponent.h - Character-attached visual effects (cold breath, damage, Kronole)

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "SEEVFXComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UPostProcessComponent;
class UMaterialInstanceDynamic;
class USEEHealthComponent;
class USEEColdComponent;
class USEECombatComponent;
class UKronoleComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKronoleVFXStageChanged, ESEEKronoleVFXStage, OldStage, ESEEKronoleVFXStage, NewStage);

/**
 * USEEVFXComponent
 *
 * Manages all visual effects attached to a character. Listens to existing
 * gameplay component delegates (health, cold, combat, Kronole) and spawns
 * appropriate Niagara effects and post-process overrides.
 *
 * Attach to ASEECharacter or ASEENPCCharacter.
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEVFXComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Cold Breath ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cold Breath")
	TSoftObjectPtr<UNiagaraSystem> ColdBreathSystem;

	// Temperature below which cold breath becomes visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cold Breath")
	float ColdBreathThreshold = 32.0f;

	// Breath rate scales with exertion (sprinting, combat)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cold Breath")
	float BaseBreathRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cold Breath")
	float ExertedBreathRate = 3.0f;

	// --- Damage / Blood ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Damage")
	TSoftObjectPtr<UNiagaraSystem> BloodSplatterSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Damage")
	TSoftObjectPtr<UNiagaraSystem> MetalSparkHitSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Damage")
	FSEEDecalConfig BloodDecalConfig;

	// Screen blood overlay intensity curve (0-1, based on health percent)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Damage")
	float LowHealthOverlayThreshold = 0.3f;

	// --- Kronole Post-Process ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole")
	TSoftObjectPtr<UMaterialInterface> KronolePostProcessMaterial;

	// Distortion intensity during peak effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole")
	float KronolePeakDistortion = 0.8f;

	// Color fringing intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole")
	float KronoleChromaticAberration = 1.5f;

	// Withdrawal screen shake intensity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Kronole")
	float WithdrawalShakeIntensity = 0.3f;

	// --- Combat VFX ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	TSoftObjectPtr<UNiagaraSystem> ParryFlashSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Combat")
	TSoftObjectPtr<UNiagaraSystem> BlockBreakSystem;

	// --- Manual Triggers ---

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnHitEffect(ESEEHitVFXType HitType, FVector Location, FVector Normal);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SpawnBloodDecal(FVector Location, FVector Normal, float Scale = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "VFX")
	void SetKronoleVFXStage(ESEEKronoleVFXStage NewStage);

	UFUNCTION(BlueprintPure, Category = "VFX")
	ESEEKronoleVFXStage GetKronoleVFXStage() const { return CurrentKronoleStage; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "VFX")
	FOnKronoleVFXStageChanged OnKronoleVFXStageChanged;

protected:
	// Cached component references
	UPROPERTY()
	TObjectPtr<USEEHealthComponent> HealthComp;

	UPROPERTY()
	TObjectPtr<USEEColdComponent> ColdComp;

	UPROPERTY()
	TObjectPtr<USEECombatComponent> CombatComp;

	UPROPERTY()
	TObjectPtr<UKronoleComponent> KronoleComp;

	// Active Niagara components
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ColdBreathNiagara;

	// Post-process
	UPROPERTY()
	TObjectPtr<UPostProcessComponent> KronolePostProcess;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> KronoleMID;

private:
	ESEEKronoleVFXStage CurrentKronoleStage = ESEEKronoleVFXStage::None;
	float KronoleBlendTarget = 0.0f;
	float KronoleBlendCurrent = 0.0f;
	float LowHealthOverlayIntensity = 0.0f;

	void BindDelegates();

	// Delegate handlers
	UFUNCTION()
	void OnDamageTaken(float Damage, ESEEDamageType DamageType, AActor* Instigator);

	UFUNCTION()
	void OnFrostbiteStageChanged(ESEEFrostbiteStage NewStage);

	UFUNCTION()
	void OnCombatStateChanged(ESEECombatState NewState);

	UFUNCTION()
	void OnParrySuccess();

	UFUNCTION()
	void OnBlockBroken();

	UFUNCTION()
	void OnHealthChanged(float NewHealthPercent);

	void UpdateColdBreath(float DeltaTime);
	void UpdateKronoleVFX(float DeltaTime);
	void UpdateLowHealthOverlay(float DeltaTime);
};
