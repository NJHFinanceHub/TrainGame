// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowpiercerEE/SEEHealthComponent.h"
#include "SEENPCHealthComponent.generated.h"

/**
 * USEENPCHealthComponent
 *
 * Thin USEEHealthComponent subclass for runtime-attached NPC health.
 *
 * The placed BP_NPC_* blueprints derive from plain ACharacter (their intended
 * C++ parent did not exist when they were authored), so they carry no health
 * component. USEENPCBrainSubsystem / ASEENPCAIController attach this component
 * at possession time. The subclass exists solely to expose an initializer for
 * the protected MaxHealth/CurrentHealth members so jackboots, civilians and
 * the boss can have different durability without touching the base class.
 *
 * Player melee finds it through FindComponentByClass<USEEHealthComponent>,
 * so the existing damage pipeline works unchanged.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEENPCHealthComponent : public USEEHealthComponent
{
	GENERATED_BODY()

public:
	USEENPCHealthComponent();

	virtual void BeginPlay() override;

	/** Set max health and refill. Call once right after attaching, before damage flows. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitHealth(float InMaxHealth);

private:
	/**
	 * CO-OP DEATH REPLICATION (client-side ragdoll).
	 *
	 * The ragdoll/death visuals normally run inside ASEENPCAIController::
	 * HandlePawnDeath, but that controller exists ONLY on the server — joined
	 * clients never spawn it, so without this they would see a dead NPC frozen
	 * upright in its last replicated pose. This component replicates (base sets
	 * SetIsReplicatedByDefault + DOREPLIFETIME(bIsDead)), so its inherited
	 * OnRep_IsDead fires OnDeath on every client. We bind this handler to OnDeath
	 * and, on non-authority only, collapse the owner pawn's mesh into ragdoll so
	 * clients see the NPC die the same way the server does. On authority this is a
	 * no-op (the AI controller already handled the full death sequence there).
	 */
	UFUNCTION()
	void HandleClientDeathRagdoll();
};
