// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "TrainGame/Core/CombatTypes.h"
#include "BossCharacter.generated.h"

class UBossFightComponent;

// ============================================================================
// ABossCharacter
//
// Zone boss character for Snowpiercer. Each of the 7 zones has a unique
// boss with distinct mechanics, special attacks, and phase-based behavior.
//
// Zone bosses:
// 1. Tail Warden — brute enforcer, area slams, summons Tail scrappers
// 2. Third Class Overseer — controls machinery, environmental attacks
// 3. Agricultural Guardian — uses greenhouse traps, poison gas
// 4. Industrial Foreman — forge-based attacks, fire damage, heavy armor
// 5. Hospitality Director — psychological warfare, minion waves
// 6. First Class Marshal — elite combat, ranged barrages, unblockable
// 7. Engine Guardian — controls train systems, cold/electrical attacks
// ============================================================================

/** Which zone this boss belongs to */
UENUM(BlueprintType)
enum class EBossZone : uint8
{
	Zone1_Tail			UMETA(DisplayName = "Tail - Warden"),
	Zone2_ThirdClass	UMETA(DisplayName = "Third Class - Overseer"),
	Zone3_Agriculture	UMETA(DisplayName = "Agriculture - Guardian"),
	Zone4_Industrial	UMETA(DisplayName = "Industrial - Foreman"),
	Zone5_Hospitality	UMETA(DisplayName = "Hospitality - Director"),
	Zone6_FirstClass	UMETA(DisplayName = "First Class - Marshal"),
	Zone7_Engine		UMETA(DisplayName = "Engine - Guardian")
};

UCLASS()
class TRAINGAME_API ABossCharacter : public AEnemyCharacter
{
	GENERATED_BODY()

public:
	ABossCharacter();

	/** Initialize boss for a specific zone */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void InitializeBoss(EBossZone Zone);

	/** Get the boss fight component */
	UFUNCTION(BlueprintPure, Category = "Boss")
	UBossFightComponent* GetBossFightComponent() const { return BossComp; }

	/** Get which zone this boss belongs to */
	UFUNCTION(BlueprintPure, Category = "Boss")
	EBossZone GetBossZone() const { return BossZone; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	UBossFightComponent* BossComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	EBossZone BossZone = EBossZone::Zone1_Tail;

	void AddPhaseConfig(EBossPhase Phase, float HealthThreshold, float DamageMult, float SpeedMult, float Resistance);
	void AddSpecialAttack(FName Name, EBossAttackType Type, float Damage, float Range,
		float Cooldown, float WindUp, bool bBlockable, bool bDodgeable,
		TArray<EBossPhase> Phases);

private:
	void SetupTailWarden();
	void SetupThirdClassOverseer();
	void SetupAgriculturalGuardian();
	void SetupIndustrialForeman();
	void SetupHospitalityDirector();
	void SetupFirstClassMarshal();
	void SetupEngineGuardian();
};
