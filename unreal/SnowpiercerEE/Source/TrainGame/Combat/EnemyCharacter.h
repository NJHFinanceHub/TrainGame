// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrainGameCombatCharacter.h"
#include "TrainGame/Core/CombatTypes.h"
#include "EnemyCharacter.generated.h"

class UStealthComponent;

// ============================================================================
// AEnemyCharacter
//
// Base enemy character with archetype-driven stats. Each enemy type on the
// train fights differently: Jackboot grunts are disciplined but predictable,
// captains coordinate squads, Order zealots are fanatical and fear-immune,
// First Class guards have superior equipment, Tail scrappers fight dirty.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyAlerted, AEnemyCharacter*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDowned, AEnemyCharacter*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReinforcementsCalled, AEnemyCharacter*, Caller, float, Delay);

UCLASS()
class TRAINGAME_API AEnemyCharacter : public ATrainGameCombatCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	/** Initialize this enemy with a specific archetype */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitializeArchetype(EEnemyType Type);

	/** Get the enemy's archetype */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyType GetEnemyType() const { return EnemyStats.EnemyType; }

	/** Get full enemy stats */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	const FEnemyStats& GetEnemyStats() const { return EnemyStats; }

	/** Is this enemy currently alerted? */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsAlerted() const { return bIsAlerted; }

	/** Alert this enemy (transitions to combat) */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void AlertEnemy(AActor* AlertSource);

	/** Call for reinforcements */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void CallReinforcements();

	/** Can this enemy be stealth-takedown'd with the given strength? */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool CanBeTakenDown(int32 AttackerStrength = 10) const;

	/** Apply a stealth takedown to this enemy */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ReceiveTakedown(bool bLethal, AActor* Attacker);

	/** Is this enemy unconscious (non-lethal takedown)? */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsUnconscious() const { return bIsUnconscious; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyAlerted OnEnemyAlerted;

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyDowned OnEnemyDowned;

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnReinforcementsCalled OnReinforcementsCalled;

protected:
	virtual void BeginPlay() override;

	/** Enemy stat configuration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FEnemyStats EnemyStats;

	/** Default weapon for this enemy type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FWeaponStats DefaultWeapon;

private:
	void ApplyArchetypeStats();
	void SetupJackbootGrunt();
	void SetupJackbootCaptain();
	void SetupOrderZealot();
	void SetupFirstClassGuard();
	void SetupTailScrapper();

	bool bIsAlerted = false;
	bool bIsUnconscious = false;
	bool bHasCalledReinforcements = false;
};
