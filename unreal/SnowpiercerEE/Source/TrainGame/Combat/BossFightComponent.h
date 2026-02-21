// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "BossFightComponent.generated.h"

class UCombatComponent;

// ============================================================================
// UBossFightComponent
//
// Manages boss fight mechanics: health phases, special attacks, phase
// transitions, and enrage timers. Each zone boss on Snowpiercer has unique
// mechanics — the Tail Warden uses brute force, the Engine Guardian
// controls the environment. Attached alongside CombatComponent on boss actors.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhaseChanged, EBossPhase, OldPhase, EBossPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossAttackWindUp, const FBossAttack&, Attack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossAttackExecuted, const FBossAttack&, Attack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossEnraged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDefeated);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBossFightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossFightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Get current boss phase */
	UFUNCTION(BlueprintPure, Category = "Boss")
	EBossPhase GetCurrentPhase() const { return CurrentPhase; }

	/** Get health percentage (0-1) */
	UFUNCTION(BlueprintPure, Category = "Boss")
	float GetHealthPercent() const;

	/** Select and begin a special attack */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	bool TrySpecialAttack(AActor* Target);

	/** Is the boss currently winding up an attack? */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsWindingUp() const { return bIsWindingUp; }

	/** Is the boss in enraged state? */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsEnraged() const { return CurrentPhase == EBossPhase::Enraged; }

	/** Is the boss defeated? */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsDefeated() const { return CurrentPhase == EBossPhase::Defeated; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossAttackWindUp OnBossAttackWindUp;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossAttackExecuted OnBossAttackExecuted;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossEnraged OnBossEnraged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossDefeated OnBossDefeated;

	/** Boss name for UI display */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FName BossName = FName("Boss");

	/** Phase configurations — define health thresholds and modifiers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TArray<FBossPhaseConfig> PhaseConfigs;

	/** Special attacks available to this boss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TArray<FBossAttack> SpecialAttacks;

	/** Time between special attack attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float SpecialAttackInterval = 6.f;

	/** Time before enrage kicks in (0 = no enrage timer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageTimer = 180.f;

	/** Damage multiplier when enraged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageDamageMultiplier = 2.f;

	/** Does the boss become invulnerable during phase transitions? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	bool bInvulnerableDuringTransition = true;

	/** Duration of phase transition invulnerability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float TransitionDuration = 2.f;

protected:
	virtual void BeginPlay() override;

private:
	void CheckPhaseTransition();
	void TransitionToPhase(EBossPhase NewPhase);
	void UpdateSpecialAttackCooldowns(float DeltaTime);
	void ExecuteSpecialAttack(FBossAttack& Attack, AActor* Target);
	FBossAttack* SelectBestAttack(AActor* Target);
	const FBossPhaseConfig* GetPhaseConfig(EBossPhase Phase) const;

	UPROPERTY()
	UCombatComponent* CombatComp = nullptr;

	EBossPhase CurrentPhase = EBossPhase::Phase1;
	float SpecialAttackTimer = 0.f;
	float EnrageCountdown = 0.f;
	bool bIsWindingUp = false;
	float WindUpTimer = 0.f;
	FBossAttack* PendingAttack = nullptr;
	AActor* PendingTarget = nullptr;
	bool bInTransition = false;
	float TransitionTimer = 0.f;
};
