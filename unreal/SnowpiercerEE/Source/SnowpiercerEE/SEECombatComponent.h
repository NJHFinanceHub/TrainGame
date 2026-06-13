#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEECombatComponent.generated.h"

class ASEEWeaponBase;
class ASEECharacter;
class USEEHealthComponent;
class USEEStatsComponent;

UENUM(BlueprintType)
enum class ESEECombatState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Blocking	UMETA(DisplayName = "Blocking"),
	Parrying	UMETA(DisplayName = "Parrying"),
	Dodging		UMETA(DisplayName = "Dodging"),
	Staggered	UMETA(DisplayName = "Staggered"),
	Recovering	UMETA(DisplayName = "Recovering")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttackHit, AActor*, HitActor, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnParrySuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, ESEECombatState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEECombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEECombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Actions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LightAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Dodge(FVector Direction);

	// Receiving damage (called before HealthComponent)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float ProcessIncomingDamage(float BaseDamage, AActor* Attacker);

	// Receiving damage with full hit context (block-breaking heavies).
	// ProcessIncomingDamage remains as a thin wrapper for existing callers.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	float ProcessIncomingHit(float BaseDamage, AActor* Attacker, bool bBreaksBlock);

	// Weapon management
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeapon(ASEEWeaponBase* Weapon);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UnequipWeapon();

	UFUNCTION(BlueprintPure, Category = "Combat")
	ASEEWeaponBase* GetEquippedWeapon() const { return EquippedWeapon; }

	// Queries
	UFUNCTION(BlueprintPure, Category = "Combat")
	ESEECombatState GetCombatState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInCombat() const { return bInCombat; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsBlocking() const { return CurrentState == ESEECombatState::Blocking; }

	/** Seconds since the player's last landed melee hit (huge if never). Drives the HUD hitmarker. */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetTimeSinceHitLanded() const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackHit OnAttackHit;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnParrySuccess OnParrySuccess;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnBlockBroken OnBlockBroken;

	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnCombatStateChanged OnCombatStateChanged;

protected:
	void SetCombatState(ESEECombatState NewState);
	void PerformWeaponTrace(float DamageMultiplier);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ParryWindowDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DodgeDistance = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DodgeIFrameDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float DodgeStaminaCost = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BlockStaminaDrainRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BlockDamageReduction = 0.7f;

	/** Armor durability lost per absorbed hit, scaled by raw incoming damage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Armor")
	float ArmorWearPerHit = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float StaggerDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float UnarmedDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float UnarmedRange = 175.0f;

	/** Sphere radius of the melee sweep — wide enough to connect in crowded corridors */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MeleeSweepRadius = 48.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ComboWindow = 0.5f;

	// --- Melee tuning ---

	/** Damage multiplier per light combo step (combo length = array length, defaults 1.0/1.1/1.3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	TArray<float> ComboDamageMultipliers;

	/** Windup before a light swing's active frame (scaled by weapon attack speed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float LightAttackWindup = 0.15f;

	/** Windup before a heavy swing's active frame (scaled by weapon attack speed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float HeavyAttackWindup = 0.5f;

	/** Unarmed heavy damage multiplier (equipped weapons use their own) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float HeavyAttackMultiplier = 2.0f;

	/** Forward lunge impulse at the start of a light swing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float LightLungeImpulse = 280.0f;

	/** Forward lunge impulse at the start of a heavy swing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float HeavyLungeImpulse = 380.0f;

	/** Unarmed light attack stamina cost (equipped weapons use StaminaCostLight) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float LightStaminaCost = 8.0f;

	/** Unarmed heavy attack stamina cost (equipped weapons use StaminaCostHeavy) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float HeavyStaminaCost = 20.0f;

	/** Extra recovery time after the final hit of a light combo */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Melee")
	float ComboFinisherExtraRecovery = 0.2f;

	// --- Feedback ---

	/** Knockback impulse applied to victims of light hits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	float LightHitKnockback = 180.0f;

	/** Knockback impulse applied to victims of heavy hits */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	float HeavyHitKnockback = 520.0f;

	/** Hit-stop length in real seconds when a hit connects */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	float HitStopDuration = 0.05f;

	/** Global time dilation during hit-stop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Feedback")
	float HitStopTimeDilation = 0.3f;

	// --- Dodge ---

	/** Cooldown between dodges */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	float DodgeCooldown = 0.8f;

	/** Transient FOV pulse on dodge for feedback */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Dodge")
	float DodgeFOVPulse = 6.0f;

	// --- Block & parry ---

	/** Total frontal arc (degrees) inside which block/parry are effective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	float BlockArcDegrees = 120.0f;

	/** Stagger duration when a heavy attack breaks the block */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	float BlockBreakStaggerDuration = 0.5f;

	/** Knockback applied to parried attackers without a compatible combat component */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Block")
	float ParryKnockbackImpulse = 450.0f;

	// --- Target assist (corridor-friendly soft lock) ---

	/** Max distance to soft-lock onto an enemy when light attacking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|TargetAssist")
	float TargetAssistRange = 300.0f;

	/** Half-angle (degrees) of the frontal acquisition cone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|TargetAssist")
	float TargetAssistConeHalfAngle = 60.0f;

	/** Max yaw correction (degrees) applied per swing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|TargetAssist")
	float TargetAssistMaxYawCorrection = 30.0f;

private:
	UPROPERTY()
	TObjectPtr<ASEEWeaponBase> EquippedWeapon;

	ESEECombatState CurrentState = ESEECombatState::Idle;
	bool bInCombat = false;
	float StateTimer = 0.0f;
	float ParryTimer = 0.0f;
	float CombatExitTimer = 0.0f;
	int32 ComboCount = 0;
	float ComboTimer = 0.0f;
	bool bDodgeIFramesActive = false;
	float DodgeTimer = 0.0f;
	float DodgeCooldownRemaining = 0.0f;

	// Timer-driven attack phases (no anim notifies — windup is pure code timing)
	FTimerHandle AttackWindupTimer;
	FTimerHandle HitStopTimer;
	bool bPendingHeavyAttack = false;
	float PendingDamageMultiplier = 1.0f;

	// Hit feedback (no animation assets — feel comes from sound, knockback, hitmarker)
	float LastHitLandedTime = -1000.0f;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> SwingSound;

	bool bFoleyLoaded = false;
	void EnsureFoleyLoaded();

	/** Apply a melee hit to one actor (damage pipeline + knockback + feedback). Returns true if it had a health component. */
	bool ApplyMeleeHitTo(AActor* HitActor, float FinalDamage);

	void BeginAttack(bool bHeavy);
	void OnAttackWindupComplete();
	void ApplyTargetAssist();
	void TriggerHitStop();
	void EndHitStop();
	ASEECharacter* GetOwnerSEECharacter() const;
};
