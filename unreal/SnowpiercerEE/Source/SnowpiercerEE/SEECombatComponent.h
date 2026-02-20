#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEECombatComponent.generated.h"

class ASEEWeaponBase;
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
	float DodgeStaminaCost = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BlockStaminaDrainRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float BlockDamageReduction = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float StaggerDuration = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float UnarmedDamage = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float UnarmedRange = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float ComboWindow = 0.5f;

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
};
