// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrainGameCombatCharacter.h"
#include "TrainGame/Core/CombatTypes.h"
#include "EnemyCharacterBase.generated.h"

// ============================================================================
// AEnemyCharacterBase
//
// Base class for all enemy types in Snowpiercer. Each enemy type has
// distinct behaviors, stats, and visual identity:
//
// - Jackboot Grunt: Standard soldier, disciplined, mid-tier
// - Jackboot Captain: Rallies troops, stronger, calls reinforcements
// - Order Zealot: Fanatical, aggressive, hard to stagger
// - First Class Guard: Elite, fast, precise attacks
// - Tail Fighter: Desperate, scrappy, improvised weapons
// - Kronole Addict: Unpredictable, can enter Kronole mode
// - Breachman: Heavy armor, cold resistant, slow
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAlerted, AEnemyCharacterBase*, Enemy, AActor*, AlertCause);

UCLASS()
class TRAINGAME_API AEnemyCharacterBase : public ATrainGameCombatCharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacterBase();

	/** Get this enemy's type */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyType GetEnemyType() const { return EnemyConfig.EnemyType; }

	/** Get this enemy's configuration */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	const FEnemyConfig& GetEnemyConfig() const { return EnemyConfig; }

	/** Apply configuration preset for an enemy type */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyTypePreset(EEnemyType Type);

	/** Check if this enemy can be stealth-killed */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool CanBeStealthKilled() const { return EnemyConfig.bCanBeStealthKilled; }

	/** Alert this enemy to a threat */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void AlertToThreat(AActor* Threat);

	/** Check if this enemy is alerted */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsAlerted() const { return bIsAlerted; }

	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FOnEnemyAlerted OnEnemyAlerted;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FEnemyConfig EnemyConfig;

	/** Weapon to equip on spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FWeaponStats StartingWeapon;

	/** Does this enemy call for backup when alerted? (Captains) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool bCallsBackup = false;

	/** Backup call radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (EditCondition = "bCallsBackup"))
	float BackupCallRadius = 2000.f;

	/** Can this enemy use Kronole mode? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	bool bCanUseKronole = false;

private:
	void SetupJackbootGrunt();
	void SetupJackbootCaptain();
	void SetupOrderZealot();
	void SetupFirstClassGuard();
	void SetupTailFighter();
	void SetupKronoleAddict();
	void SetupBreachman();

	void CallForBackup();

	bool bIsAlerted = false;
};
