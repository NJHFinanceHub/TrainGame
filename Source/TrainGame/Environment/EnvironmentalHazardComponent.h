// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "EnvironmentalHazardComponent.generated.h"

// ============================================================================
// UEnvironmentalHazardComponent
//
// Attach to environmental objects in train cars that can be triggered
// during combat for environmental kills. Steam vents scald, electrical
// panels shock, window breaches suck enemies into the frozen void.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHazardTriggered, EEnvironmentalHazard, HazardType, AActor*, Victim);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UEnvironmentalHazardComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEnvironmentalHazardComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Trigger the hazard, affecting all actors in the hazard zone */
	UFUNCTION(BlueprintCallable, Category = "Environment|Hazard")
	void TriggerHazard(AActor* Instigator);

	/** Check if a specific actor is within the hazard's effect zone */
	UFUNCTION(BlueprintPure, Category = "Environment|Hazard")
	bool IsActorInHazardZone(AActor* Actor) const;

	/** Can the hazard be triggered right now? */
	UFUNCTION(BlueprintPure, Category = "Environment|Hazard")
	bool CanTrigger() const;

	UPROPERTY(BlueprintAssignable, Category = "Environment|Hazard")
	FOnHazardTriggered OnHazardTriggered;

protected:
	virtual void BeginPlay() override;

	/** What type of hazard this is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	EEnvironmentalHazard HazardType = EEnvironmentalHazard::SteamVent;

	/** Base damage dealt by this hazard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float BaseDamage = 50.f;

	/** Radius of the hazard effect zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float HazardRadius = 200.f;

	/** Duration the hazard remains active after triggering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float ActiveDuration = 3.f;

	/** Cooldown before hazard can be triggered again (0 = one-shot) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	float Cooldown = 10.f;

	/** Can this hazard only be triggered once? (window breach, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bOneShot = false;

	/** Does this hazard apply a knockback force? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bAppliesKnockback = false;

	/** Knockback force magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard", meta = (EditCondition = "bAppliesKnockback"))
	float KnockbackForce = 1000.f;

	/** Direction of knockback (local space - Z = up, X = forward from hazard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard", meta = (EditCondition = "bAppliesKnockback"))
	FVector KnockbackDirection = FVector(1.f, 0.f, 0.f);

	/** Does this hazard deal damage over time while in the zone? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bDamageOverTime = false;

	/** DOT damage per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard", meta = (EditCondition = "bDamageOverTime"))
	float DamagePerSecond = 20.f;

	/** Is this a lethal hazard? (window breach = instant kill if sucked out) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bInstantKill = false;

	/** Can the player trigger this hazard? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bPlayerCanTrigger = true;

	/** Can enemies trigger this hazard (accidentally or by being pushed into it)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hazard")
	bool bEnemiesCanTrigger = true;

private:
	void ApplyHazardEffect(AActor* Victim, AActor* Instigator);
	void ApplyKnockback(AActor* Victim) const;
	EDamageType GetDamageTypeForHazard() const;
	TArray<AActor*> GetActorsInHazardZone() const;

	bool bIsActive = false;
	float ActiveTimer = 0.f;
	float CooldownTimer = 0.f;
	bool bHasBeenTriggered = false;
};
