// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "BossFightComponent.generated.h"

class UCombatComponent;
class UEnvironmentalHazardComponent;

// ============================================================================
// UBossFightComponent
//
// Manages boss fight mechanics: health phases, special attacks, phase
// transitions, enrage timers, and minion summoning. Attach alongside
// UCombatComponent on boss enemies. Data-driven via FBossData.
//
// The 7 zone bosses of Snowpiercer:
// Zone 0 (Tail): The Pitmaster — brutal melee, arena hazards
// Zone 1 (Third Class): Jackboot Commander Vasek — formation + ranged
// Zone 2 (Second Class): Dr. Klimov — chemical weapons, sleep gas
// Zone 3 (Agriculture): The Beekeeper — swarm summons, DOT
// Zone 4 (First Class): Lilah Folger — First Class guards, counter-fighting
// Zone 5 (Engine Approach): The Last Jackboot — full military arsenal
// Zone 6 (Engine): The Eternal Engineer — Wilford's final protector
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, EBossPhase, NewPhase, int32, PhaseIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpecialAttack, EBossAttackType, AttackType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossEnraged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDefeated);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBossFightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossFightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize boss with data configuration */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void InitBoss(const FBossData& InBossData);

	/** Trigger a specific special attack */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformSpecialAttack(EBossAttackType AttackType);

	/** Force phase transition (for scripted events) */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ForcePhaseTransition(int32 PhaseIndex);

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Boss")
	EBossPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetCurrentPhaseIndex() const { return CurrentPhaseIndex; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsEnraged() const { return bIsEnraged; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	const FBossData& GetBossData() const { return BossData; }

	UFUNCTION(BlueprintPure, Category = "Boss")
	float GetPhaseProgress() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossSpecialAttack OnBossSpecialAttack;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossEnraged OnBossEnraged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossDefeated OnBossDefeated;

protected:
	virtual void BeginPlay() override;

	/** Boss configuration data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	FBossData BossData;

	/** Time before boss enrages if fight takes too long */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageTimer = 180.f;

	/** Damage multiplier when enraged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float EnrageDamageMultiplier = 2.f;

	/** Minimum time between special attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float SpecialAttackCooldown = 8.f;

	/** Class to spawn for summoned minions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TSubclassOf<APawn> MinionClass;

	/** Invulnerability during phase transitions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float PhaseTransitionDuration = 2.f;

private:
	void CheckPhaseTransition();
	void TransitionToPhase(int32 PhaseIndex);
	void ApplyPhaseModifiers(const FBossPhaseConfig& PhaseConfig);
	void UpdateEnrageTimer(float DeltaTime);
	void UpdateSpecialAttackTimer(float DeltaTime);
	void AttemptSpecialAttack();
	void PerformAOEAttack();
	void PerformChargeAttack();
	void PerformGrabThrow();
	void SummonMinions(int32 Count);
	void TriggerArenaHazard();

	UPROPERTY()
	UCombatComponent* CombatComp = nullptr;

	EBossPhase CurrentPhase = EBossPhase::Phase1;
	int32 CurrentPhaseIndex = 0;
	float FightTimer = 0.f;
	float SpecialAttackTimer = 0.f;
	bool bIsEnraged = false;
	bool bIsTransitioning = false;
	float TransitionTimer = 0.f;
	bool bIsInitialized = false;

	UPROPERTY()
	TArray<AActor*> SpawnedMinions;
};
