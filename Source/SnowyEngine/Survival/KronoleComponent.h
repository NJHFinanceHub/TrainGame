// KronoleComponent.h - Kronole drug system: buffs, addiction, and withdrawal
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SurvivalTypes.h"
#include "KronoleComponent.generated.h"

/**
 * UKronoleComponent
 *
 * Manages the Kronole drug mechanic. Kronole provides powerful temporary combat buffs
 * (slow-mo perception, damage resistance, pain suppression) but builds addiction.
 * Withdrawal escalates with addiction stage and imposes harsh stat penalties.
 *
 * Addiction stages progress: Clean -> Casual -> Dependent -> Addicted -> Terminal.
 * Each dose advances the addiction meter. Time without a dose triggers withdrawal.
 * Withdrawal severity scales with addiction stage.
 *
 * Works alongside USurvivalComponent to apply stat modifiers during buff/withdrawal.
 */
UCLASS(ClassGroup=(Survival), meta=(BlueprintSpawnableComponent))
class SNOWYENGINE_API UKronoleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKronoleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Kronole Usage ---

	/** Take a dose of Kronole. Returns true if successfully consumed. */
	UFUNCTION(BlueprintCallable, Category = "Kronole")
	bool TakeDose(bool bRefined = false);

	/** Returns true if the character is currently under the effects of Kronole. */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	bool IsUnderEffect() const { return bIsActive; }

	/** Returns remaining buff duration in seconds. */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetRemainingBuffDuration() const { return ActiveBuffTimer; }

	// --- Addiction ---

	UFUNCTION(BlueprintPure, Category = "Kronole")
	EKronoleAddictionStage GetAddictionStage() const;

	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetAddictionLevel() const { return AddictionLevel; }

	UFUNCTION(BlueprintPure, Category = "Kronole")
	EWithdrawalSeverity GetWithdrawalSeverity() const;

	UFUNCTION(BlueprintPure, Category = "Kronole")
	bool IsInWithdrawal() const { return bInWithdrawal; }

	// --- Buff Modifiers (for combat/perception systems to query) ---

	/** Combat damage multiplier while buffed. */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetDamageMultiplier() const;

	/** Time dilation factor while buffed (< 1.0 = slow-mo perception). */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetTimeDilationFactor() const;

	/** Damage resistance while buffed (0-1, fraction of damage negated). */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetDamageResistance() const;

	// --- Withdrawal Penalties (for survival/companion systems to query) ---

	/** Stamina regen penalty during withdrawal (multiplier, <1.0 = slower). */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetWithdrawalStaminaPenalty() const;

	/** Morale penalty during withdrawal (flat negative applied per tick). */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetWithdrawalMoraleDrain() const;

	/** Combat effectiveness penalty during withdrawal. */
	UFUNCTION(BlueprintPure, Category = "Kronole")
	float GetWithdrawalCombatPenalty() const;

	// --- Serialization ---

	UFUNCTION(BlueprintCallable, Category = "Kronole")
	void SetAddictionState(float InAddictionLevel, float InTimeSinceLastDose);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Kronole")
	FOnAddictionStageChanged OnAddictionStageChanged;

protected:
	virtual void BeginPlay() override;

	// --- Config ---

	/** Duration of Kronole buff in seconds (raw). */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	float RawBuffDuration = 30.0f;

	/** Duration of Kronole buff in seconds (refined). */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	float RefinedBuffDuration = 60.0f;

	/** Addiction gained per raw dose. */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	float AddictionPerRawDose = 8.0f;

	/** Addiction gained per refined dose. */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	float AddictionPerRefinedDose = 15.0f;

	/** Addiction decay rate per second when not using. */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	float AddictionDecayRate = 0.01f;

	/** Seconds without a dose before withdrawal begins (per addiction stage). */
	UPROPERTY(EditAnywhere, Category = "Kronole|Config")
	TArray<float> WithdrawalOnsetTimes;

	// --- Runtime State ---

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	float AddictionLevel = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	float TimeSinceLastDose = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	float ActiveBuffTimer = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	bool bIsActive = false;

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	bool bInWithdrawal = false;

	UPROPERTY(VisibleAnywhere, Category = "Kronole|Runtime")
	bool bLastDoseWasRefined = false;

private:
	EKronoleAddictionStage CachedStage = EKronoleAddictionStage::Clean;

	void TickBuff(float DeltaTime);
	void TickAddiction(float DeltaTime);
	void TickWithdrawal(float DeltaTime);
	void UpdateAddictionStage();
	float GetWithdrawalOnsetTime() const;
};
