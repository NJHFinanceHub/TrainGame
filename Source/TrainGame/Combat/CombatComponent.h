// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "CombatComponent.generated.h"

class UWeaponComponent;

// ============================================================================
// UCombatComponent
//
// Core combat component attached to any combatant (player or NPC).
// Handles attacks, blocks, dodges, stamina, fatigue, and Kronole mode.
// Designed for tight corridor positional combat on a train.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceChanged, ECombatStance, NewStance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitLanded, const FHitResult_Combat&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReceived, const FHitResult_Combat&, HitResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, NewStamina);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKronoleModeActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKronoleModeDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Actions ---

	/** Initiate a melee attack in the given direction */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FHitResult_Combat PerformAttack(EAttackDirection Direction);

	/** Begin blocking in a direction. Block is held until released. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock(EBlockDirection Direction);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	/** Execute a dodge. In tight corridors, this is a sidestep or backstep. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PerformDodge(FVector DodgeDirection);

	/** Receive damage from another combatant or environment */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	FHitResult_Combat ReceiveAttack(float IncomingDamage, EAttackDirection Direction, EDamageType DamageType, AActor* Attacker);

	// --- Kronole System ---

	/** Activate Kronole mode (slow-motion perception, costs health) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Kronole")
	bool ActivateKronoleMode();

	UFUNCTION(BlueprintCallable, Category = "Combat|Kronole")
	void DeactivateKronoleMode();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Combat")
	ECombatStance GetCurrentStance() const { return CurrentStance; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentStamina() const { return StaminaState.CurrentStamina; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetFatigueLevel() const { return StaminaState.FatigueLevel; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInKronoleMode() const { return bKronoleModeActive; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsBlocking() const { return CurrentStance == ECombatStance::Blocking; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsStaggered() const { return CurrentStance == ECombatStance::Staggered; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnStanceChanged OnStanceChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitLanded OnHitLanded;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnHitReceived OnHitReceived;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnStaminaDepleted OnStaminaDepleted;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnKronoleModeActivated OnKronoleModeActivated;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnKronoleModeDeactivated OnKronoleModeDeactivated;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnDeath OnDeath;

protected:
	virtual void BeginPlay() override;

	// --- Health ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health")
	float CurrentHealth = 100.f;

	// --- Stamina & Fatigue ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	FStaminaState StaminaState;

	/** Stamina cost to dodge */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float DodgeStaminaCost = 20.f;

	/** Stamina cost per second while blocking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float BlockStaminaDrainPerSecond = 5.f;

	/** Fatigue gained per attack performed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float FatiguePerAttack = 2.f;

	/** Fatigue recovery rate when not in combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float FatigueRecoveryRate = 5.f;

	/** Time since last combat action before fatigue starts recovering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float FatigueRecoveryDelay = 5.f;

	// --- Blocking ---

	/** Damage reduction when blocking in the correct direction (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block", meta = (ClampMin = "0", ClampMax = "1"))
	float BlockDamageReduction = 0.8f;

	/** Damage reduction when blocking in the wrong direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block", meta = (ClampMin = "0", ClampMax = "1"))
	float PartialBlockReduction = 0.3f;

	/** Stamina damage taken when blocking a hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	float BlockStaminaCost = 10.f;

	/** If stamina is drained while blocking, combatant is staggered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	float StaggerDuration = 1.5f;

	// --- Dodge ---

	/** Invulnerability window during dodge (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeIFrameDuration = 0.3f;

	/** Dodge distance in tight corridor (shorter than open field) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeDistance = 200.f;

	/** Cooldown between dodges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeCooldown = 0.8f;

	// --- Kronole Mode ---

	/** Health cost per second while Kronole mode is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Kronole")
	float KronoleHealthCostPerSecond = 5.f;

	/** Time dilation factor during Kronole mode (0.3 = 30% speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Kronole")
	float KronoleTimeDilation = 0.3f;

	/** Maximum duration Kronole mode can be active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Kronole")
	float KronoleMaxDuration = 5.f;

	/** Cooldown after Kronole mode ends */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Kronole")
	float KronoleCooldown = 15.f;

	/** Minimum health required to activate Kronole mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Kronole")
	float KronoleMinHealthRequired = 20.f;

	// --- Attack Parameters ---

	/** Base attack cooldown (modified by weapon speed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float BaseAttackCooldown = 0.5f;

	/** Unarmed base damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float UnarmedDamage = 8.f;

	/** Critical hit chance (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float CriticalHitChance = 0.1f;

	/** Critical hit damage multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Attack")
	float CriticalHitMultiplier = 2.f;

private:
	void SetStance(ECombatStance NewStance);
	void ConsumeStamina(float Amount);
	void RegenerateStamina(float DeltaTime);
	void UpdateFatigue(float DeltaTime);
	void UpdateKronoleMode(float DeltaTime);
	void ApplyDamage(float Damage, AActor* DamageSource);
	void Die(AActor* Killer);
	float CalculateBlockReduction(EAttackDirection AttackDir) const;
	bool DoesBlockMatchAttack(EAttackDirection AttackDir) const;

	/** Perform a melee trace to find targets */
	AActor* PerformMeleeTrace(float TraceRange) const;

	ECombatStance CurrentStance = ECombatStance::Neutral;
	EBlockDirection CurrentBlockDirection = EBlockDirection::Mid;

	bool bKronoleModeActive = false;
	float KronoleModeTimer = 0.f;
	float KronoleCooldownTimer = 0.f;

	bool bDodgeOnCooldown = false;
	float DodgeCooldownTimer = 0.f;
	bool bInIFrames = false;
	float IFrameTimer = 0.f;

	float AttackCooldownTimer = 0.f;
	float StaggerTimer = 0.f;
	float TimeSinceLastCombatAction = 0.f;
};
