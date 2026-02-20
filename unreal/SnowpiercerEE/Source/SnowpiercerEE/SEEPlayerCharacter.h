#pragma once

#include "CoreMinimal.h"
#include "SEECharacter.h"
#include "SEECharacterAnimInstance.h"
#include "SEEPlayerCharacter.generated.h"

/**
 * Player character with skeletal mesh, weapon socket, and hit reaction support.
 * Create a Blueprint extending this class and assign the UE5 Mannequin
 * (or custom skeletal mesh) plus an Animation Blueprint in the editor.
 *
 * Expected skeletal mesh setup:
 *   - Socket "weapon_r" on the right hand bone for weapon attachment
 *   - Animation Blueprint using USEECharacterAnimInstance
 */
UCLASS()
class SNOWPIERCEREE_API ASEEPlayerCharacter : public ASEECharacter
{
	GENERATED_BODY()

public:
	ASEEPlayerCharacter();

	virtual void BeginPlay() override;

	// --- Hit Reactions ---

	/** Apply a hit reaction (stagger, knockback, or death ragdoll) */
	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection = FVector::ZeroVector);

	/** Activate ragdoll physics for death */
	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ActivateDeathRagdoll();

	/** Check if player is in a hit reaction state */
	UFUNCTION(BlueprintPure, Category = "HitReaction")
	bool IsInHitReaction() const { return bInHitReaction; }

	// --- Weapon Socket ---

	/** Name of the weapon attach socket on the skeletal mesh */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("weapon_r");

	/** Attach an actor to the weapon socket */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToWeaponSocket(AActor* ActorToAttach);

	/** Detach the current weapon socket occupant */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DetachFromWeaponSocket();

protected:
	void OnDamageTaken(float Damage, ESEEDamageType DamageType, AActor* Instigator);

	/** Knockback impulse strength */
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	float KnockbackImpulse = 600.0f;

	/** Stagger duration before recovery */
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	float StaggerRecoveryTime = 0.6f;

	/** Heavy stagger threshold (damage percent to trigger heavy stagger vs light) */
	UPROPERTY(EditDefaultsOnly, Category = "HitReaction")
	float HeavyStaggerThreshold = 25.0f;

private:
	bool bInHitReaction = false;
	FTimerHandle StaggerRecoveryTimer;

	void EndStagger();
};
