// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TrainGame/Core/CombatTypes.h"
#include "EnemyCharacter.generated.h"

class UCombatComponent;
class UWeaponComponent;
class UBossFightComponent;

// ============================================================================
// AEnemyCharacter
//
// Configurable enemy character with type-specific stats, damage resistances,
// and weapon loadouts. Supports all enemy types from Tail fighters to
// zone bosses. Automatically applies FEnemyConfig on BeginPlay.
// ============================================================================

UCLASS()
class TRAINGAME_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UCombatComponent* GetCombatComponent() const { return CombatComp; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UWeaponComponent* GetWeaponComponent() const { return WeaponComp; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UBossFightComponent* GetBossFightComponent() const { return BossComp; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyType GetEnemyType() const { return EnemyConfig.EnemyType; }

	/** Apply an enemy configuration (can be called post-spawn) */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void ApplyConfig(const FEnemyConfig& Config);

	/** Initialize as a boss with boss data */
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void InitAsBoss(const FBossData& InBossData);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UBossFightComponent* BossComp;

	/** Enemy configuration — set in editor or applied at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	FEnemyConfig EnemyConfig;

private:
	void EquipDefaultWeapon();

	/** Create preset configs for each enemy type */
	static FEnemyConfig MakeTailFighterConfig();
	static FEnemyConfig MakeJackbootGruntConfig();
	static FEnemyConfig MakeJackbootCaptainConfig();
	static FEnemyConfig MakeOrderZealotConfig();
	static FEnemyConfig MakeFirstClassGuardConfig();
};
