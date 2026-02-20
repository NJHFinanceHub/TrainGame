// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "CombatComponent.generated.h"

class UWeaponComponent;
class AProjectileBase;

// ============================================================================
// UCombatComponent
//
// Core combat component attached to any combatant (player or NPC).
// Handles melee attacks, ranged attacks, blocks, dodges, stamina, fatigue,
// Kronole mode, and stealth takedown reception.
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStealthTakedown, AActor*, Attacker, ETakedownType, TakedownType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrappled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrappleEscaped);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Melee Actions ---

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

	// --- Ranged Actions ---

	/** Fire a ranged weapon, spawning a projectile actor */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	bool PerformRangedAttack(FVector AimDirection);

	/** Throw current weapon (consumes it) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Ranged")
	bool ThrowWeapon(FVector ThrowDirection);

	// --- Stealth Takedown Reception ---

	/** Receive a stealth takedown from behind */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stealth")
	FHitResult_Combat ReceiveStealthTakedown(AActor* Attacker, ETakedownType TakedownType, bool bLethal);

	/** Attempt to grapple this combatant (grab from behind) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stealth")
	bool BeginGrapple(AActor* Grappler);

	/** Escape from being grappled (mash to escape) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stealth")
	bool AttemptGrappleEscape();

	// --- Kronole System ---

	/** Activate Kronole mode (slow-motion perception, costs health) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Kronole")
	bool ActivateKronoleMode();

	UFUNCTION(BlueprintCallable, Category = "Combat|Kronole")
	void DeactivateKronoleMode();

	// --- Damage Resistance ---

	/** Set resistance to a specific damage type (0-1) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetDamageResistance(EDamageType Type, float Resistance);

	/** Set stagger resistance (0 = normal, 1 = immune) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetStaggerResistance(float Resistance);

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Combat")
	ECombatStance GetCurrentStance() const { return CurrentStance; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

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

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsGrappled() const { return CurrentStance == ECombatStance::Grappled; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanBeTakenDown() const { return bCanBeTakenDown; }

	/** Set the health (for boss phase transitions, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetMaxHealth(float NewMaxHealth);

	/** Heal the combatant */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Heal(float Amount);

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

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnStealthTakedown OnStealthTakedown;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnGrappled OnGrappled;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnGrappleEscaped OnGrappleEscaped;

protected:
	virtual void BeginPlay() override;

	// --- Health ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Health")
	float CurrentHealth = 100.f;

	/** Whether this combatant can be stealth takedown'd */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
	bool bCanBeTakenDown = true;

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

	// --- Grapple ---

	/** Time to escape a grapple (button mashing reduces this) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Grapple")
	float GrappleEscapeTime = 3.f;

	/** Resistance to stagger (0 = normal, 1 = immune) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Defense", meta = (ClampMin = "0", ClampMax = "1"))
	float StaggerResistance = 0.f;

	// --- Ranged ---

	/** Projectile class to spawn for ranged attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Ranged")
	TSubclassOf<AProjectileBase> ProjectileClass;

private:
	void SetStance(ECombatStance NewStance);
	void ConsumeStamina(float Amount);
	void RegenerateStamina(float DeltaTime);
	void UpdateFatigue(float DeltaTime);
	void UpdateKronoleMode(float DeltaTime);
	void UpdateGrapple(float DeltaTime);
	void ApplyDamage(float Damage, AActor* DamageSource);
	void Die(AActor* Killer);
	float CalculateBlockReduction(EAttackDirection AttackDir) const;
	bool DoesBlockMatchAttack(EAttackDirection AttackDir) const;
	float ApplyDamageResistance(float Damage, EDamageType Type) const;

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

	// Grapple state
	UPROPERTY()
	AActor* Grappler = nullptr;
	float GrappleTimer = 0.f;
	int32 GrappleEscapeProgress = 0;

	// Damage resistances
	TMap<EDamageType, float> DamageResistances;
};
