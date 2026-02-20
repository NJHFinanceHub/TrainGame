// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacterBase.h"
#include "TrainGame/Core/CombatTypes.h"
#include "BossCharacterBase.generated.h"

class UBossFightComponent;

// ============================================================================
// ABossCharacterBase
//
// Base class for zone bosses. Extends enemy with boss fight phases,
// special attacks, and unique mechanics. Each zone has one boss:
//
// Zone 1 — The Pit Champion: Raw brawler, crowd control
// Zone 2 — Chef Pierrot: Kitchen weapons, environmental fire
// Zone 3 — Jackboot Commander: Squad tactics, shield formations
// Zone 4 — Kronole Lord: Time manipulation, unpredictable
// Zone 5 — First Class Duelist: Fencing precision, riposte
// Zone 6 — Order High Priest: Zealot adds, ritual damage
// Zone 7 — Mr. Wilford: Engine room, final confrontation
// ============================================================================

UCLASS()
class TRAINGAME_API ABossCharacterBase : public AEnemyCharacterBase
{
	GENERATED_BODY()

public:
	ABossCharacterBase();

	UFUNCTION(BlueprintPure, Category = "Boss")
	UBossFightComponent* GetBossFightComponent() const { return BossFightComp; }

	/** Initialize the boss with a specific identity */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void SetupBoss(EBossIdentity Identity);

	/** Start the boss encounter */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void BeginEncounter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
	UBossFightComponent* BossFightComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	EBossIdentity BossIdentity = EBossIdentity::ThePitChampion;

	/** Arena size — boss fights happen in specific car areas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float ArenaRadius = 2000.f;

private:
	UFUNCTION()
	void OnPhaseChanged(EBossPhase NewPhase, EBossPhase OldPhase);

	UFUNCTION()
	void OnDefeated();
};
