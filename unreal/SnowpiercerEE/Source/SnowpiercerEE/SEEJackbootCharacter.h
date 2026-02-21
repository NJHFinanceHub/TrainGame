#pragma once

#include "CoreMinimal.h"
#include "AI/SEENPCCharacter.h"
#include "SEECharacterAnimInstance.h"
#include "SEEJackbootCharacter.generated.h"

/**
 * Jackboot guard NPC. Heavily armed train security.
 * Create a Blueprint extending this class and assign a Jackboot skeletal mesh
 * plus NPC Animation Blueprint in the editor.
 *
 * Defaults: higher health, damage, and detection capabilities than civilians.
 */
UCLASS()
class SNOWPIERCEREE_API ASEEJackbootCharacter : public ASEENPCCharacter
{
	GENERATED_BODY()

public:
	ASEEJackbootCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Hit Reactions ---

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ActivateDeathRagdoll();

	// --- Weapon Socket ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("weapon_r");

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToWeaponSocket(AActor* ActorToAttach);

	// --- Jackboot-Specific ---

	/** Whether this Jackboot calls for reinforcements when entering combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot")
	bool bCanCallReinforcements = true;

	/** Radius to alert nearby Jackboots when entering combat */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot")
	float ReinforcementAlertRadius = 1500.0f;

protected:
	void AlertNearbyJackboots();

private:
	bool bHasAlertedReinforcements = false;
};
