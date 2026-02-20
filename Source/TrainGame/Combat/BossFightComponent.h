// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "BossFightComponent.generated.h"

class UCombatComponent;

// ============================================================================
// UBossFightComponent
//
// Boss fight framework with health phases, special attacks, and add spawning.
// Each zone boss has unique mechanics:
//
// Zone 1 (Tail):    The Pit Champion — brute force, crowd fighting
// Zone 2 (3rd):     Chef Pierrot — kitchen weapons, fire hazards
// Zone 3 (Security):Jackboot Commander — calls squads, formation tactics
// Zone 4 (Night):   Kronole Lord — time dilation manipulation
// Zone 5 (1st):     First Class Duelist — fencing, precise mechanics
// Zone 6 (Shrine):  Order High Priest — zealot adds, ritual attacks
// Zone 7 (Engine):  Mr. Wilford — engine room hazards, final confrontation
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBossPhaseChanged, EBossPhase, NewPhase, EBossPhase, OldPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossSpecialAttack, FName, AttackName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBossDefeated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossAddsSpawned, int32, Count);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBossFightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBossFightComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Initialize the boss fight (call when encounter begins) */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void InitBossFight();

	/** Apply preset mechanics for a boss identity */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ApplyBossPreset(EBossIdentity Identity);

	/** Trigger a special attack by name */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void PerformSpecialAttack(FName AttackName);

	/** Get current boss phase */
	UFUNCTION(BlueprintPure, Category = "Boss")
	EBossPhase GetCurrentPhase() const { return CurrentPhase; }

	/** Get boss identity */
	UFUNCTION(BlueprintPure, Category = "Boss")
	EBossIdentity GetBossIdentity() const { return BossIdentity; }

	/** Check if boss is in enraged state */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsEnraged() const { return CurrentPhase == EBossPhase::Enraged; }

	/** Check if boss fight is active */
	UFUNCTION(BlueprintPure, Category = "Boss")
	bool IsFightActive() const { return bFightActive; }

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossSpecialAttack OnBossSpecialAttack;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossDefeated OnBossDefeated;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossAddsSpawned OnBossAddsSpawned;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	EBossIdentity BossIdentity = EBossIdentity::ThePitChampion;

	/** Maximum health for this boss */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float BossMaxHealth = 500.f;

	/** Phase configurations (in order of health threshold) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TArray<FBossPhaseConfig> PhaseConfigs;

	/** Special attacks available (by name) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TArray<FName> SpecialAttackNames;

	/** Cooldown between special attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float SpecialAttackCooldown = 8.f;

	/** Class to spawn as adds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	TSubclassOf<APawn> AddClass;

	/** Max adds alive at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	int32 MaxAdds = 4;

	/** Damage reduction during phase transitions (brief invulnerability) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss")
	float PhaseTransitionInvulnDuration = 2.f;

private:
	void CheckPhaseTransition();
	void TransitionToPhase(EBossPhase NewPhase);
	void SpawnAdds(int32 Count);
	void UpdateSpecialAttackCooldown(float DeltaTime);
	void ChooseSpecialAttack();

	UFUNCTION()
	void OnBossDeath(AActor* Killer);

	EBossPhase CurrentPhase = EBossPhase::Phase1;
	bool bFightActive = false;
	float SpecialAttackTimer = 0.f;
	float PhaseTransitionTimer = 0.f;
	bool bInPhaseTransition = false;
	int32 CurrentAddsAlive = 0;

	UPROPERTY()
	UCombatComponent* OwnerCombat = nullptr;
};
