#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SEECombatComponent.h"
#include "SEECharacterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ESEELocomotionState : uint8
{
	Idle,
	Walking,
	Running,
	Sprinting,
	Crouching,
	Jumping,
	Falling
};

UENUM(BlueprintType)
enum class ESEEHitReactionType : uint8
{
	None,
	StaggerLight,
	StaggerHeavy,
	KnockbackFront,
	KnockbackBack,
	DeathRagdoll
};

/**
 * Base anim instance for all SEE characters.
 * Provides locomotion, combat, and hit reaction state for Animation Blueprints.
 */
UCLASS()
class SNOWPIERCEREE_API USEECharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- Locomotion ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float Direction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bIsInAir = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	bool bIsCrouching = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	ESEELocomotionState LocomotionState = ESEELocomotionState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float MovementInputForward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
	float MovementInputRight = 0.0f;

	// --- Combat ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ESEECombatState CombatState = ESEECombatState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsBlocking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasWeaponEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsDodging = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 ComboIndex = 0;

	// --- Hit Reactions ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReaction")
	ESEEHitReactionType ActiveHitReaction = ESEEHitReactionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReaction")
	bool bIsDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReaction")
	bool bIsStaggered = false;

	// --- Health ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float HealthPercent = 1.0f;

	// --- Play rate modifiers ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float AttackPlayRate = 1.0f;

	/** Call from Blueprint or C++ to trigger a hit reaction */
	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void PlayHitReaction(ESEEHitReactionType ReactionType);

	/** Call to clear the current hit reaction (typically from anim notify) */
	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ClearHitReaction();

protected:
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	void UpdateLocomotion(float DeltaSeconds);
	void UpdateCombat();
	void UpdateHealth();
};
