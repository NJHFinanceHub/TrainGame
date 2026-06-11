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

	/** Set max health and refill. Call once right after attaching, before damage flows. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitHealth(float InMaxHealth);
};
