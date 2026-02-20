// SEECombatAIComponent.h
// Snowpiercer: Eternal Engine - Enemy combat AI: attack patterns, blocking, positioning

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEDamageTypes.h"
#include "SEECombatAIComponent.generated.h"

class USEECombatComponent;

/** AI combat behavior profile */
UENUM(BlueprintType)
enum class ESEECombatAIBehavior : uint8
{
	Aggressive   UMETA(DisplayName = "Aggressive"),
	Defensive    UMETA(DisplayName = "Defensive"),
	Balanced     UMETA(DisplayName = "Balanced"),
	Berserker    UMETA(DisplayName = "Berserker"),
	Sniper       UMETA(DisplayName = "Sniper")
};

/** AI combat state */
UENUM(BlueprintType)
enum class ESEECombatAIState : uint8
{
	Idle         UMETA(DisplayName = "Idle"),
	Approaching  UMETA(DisplayName = "Approaching"),
	Attacking    UMETA(DisplayName = "Attacking"),
	Blocking     UMETA(DisplayName = "Blocking"),
	Retreating   UMETA(DisplayName = "Retreating"),
	Flanking     UMETA(DisplayName = "Flanking"),
	Staggered    UMETA(DisplayName = "Staggered")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatAIStateChanged, ESEECombatAIState, NewState);

/**
 * AI component for enemy combat behavior.
 * Drives attack timing, block reactions, positioning, and group tactics.
 * Requires a sibling USEECombatComponent on the same actor.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEECombatAIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEECombatAIComponent();

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	ESEECombatAIBehavior Behavior = ESEECombatAIBehavior::Balanced;

	/** Preferred engagement distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float PreferredRange = 200.0f;

	/** Time between attacks (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Timing")
	float AttackCooldown = 1.5f;

	/** Chance to block an incoming attack (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Timing")
	float BlockChance = 0.3f;

	/** Chance to dodge an incoming attack (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Timing")
	float DodgeChance = 0.15f;

	/** Health threshold to start retreating (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float RetreatHealthThreshold = 0.2f;

	/** Stagger duration after taking heavy damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI|Timing")
	float StaggerDuration = 0.8f;

	/** Damage threshold to trigger stagger */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float StaggerDamageThreshold = 25.0f;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "CombatAI")
	ESEECombatAIState CurrentState = ESEECombatAIState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "CombatAI")
	AActor* CurrentTarget = nullptr;

	// --- Interface ---

	/** Set the combat target */
	UFUNCTION(BlueprintCallable, Category = "CombatAI")
	void SetTarget(AActor* Target);

	/** Clear current target and return to idle */
	UFUNCTION(BlueprintCallable, Category = "CombatAI")
	void ClearTarget();

	/** Notify AI of incoming damage for reaction */
	UFUNCTION(BlueprintCallable, Category = "CombatAI")
	void NotifyDamageReceived(float DamageAmount);

	/** Get the desired movement direction toward/away from target */
	UFUNCTION(BlueprintPure, Category = "CombatAI")
	FVector GetDesiredMovementDirection() const;

	/** Check if AI wants to attack this frame */
	UFUNCTION(BlueprintPure, Category = "CombatAI")
	bool WantsToAttack() const;

	/** Get the attack direction the AI has chosen */
	UFUNCTION(BlueprintPure, Category = "CombatAI")
	ESEEAttackDirection GetChosenAttackDirection() const;

	/** Get distance to current target */
	UFUNCTION(BlueprintPure, Category = "CombatAI")
	float GetDistanceToTarget() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "CombatAI|Events")
	FOnCombatAIStateChanged OnStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateState(float DeltaTime);
	void SetState(ESEECombatAIState NewState);

	UPROPERTY()
	USEECombatComponent* CombatComponent = nullptr;

	float AttackCooldownRemaining = 0.0f;
	float StaggerTimeRemaining = 0.0f;
	bool bWantsToAttack = false;
	ESEEAttackDirection ChosenDirection = ESEEAttackDirection::Mid;
};
