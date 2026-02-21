// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrainGame/Combat/BossCharacter.h"
#include "SEEBossGrey.generated.h"

class AEnemyCharacter;

// ============================================================================
// ASEEBossGrey
//
// Commander Grey — Zone 1 boss. Jackboot Commander who guards the Threshold
// (Car 15). 4-phase fight: Gauntlet → Arena → 1v1 Melee → Door Sequence.
// Intel from Cars 8/13 reveals his weak left guard.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGreyDefeated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorSequenceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGuardSquadChanged, int32, GuardsRemaining);

UCLASS()
class SNOWPIERCEREE_API ASEEBossGrey : public ABossCharacter
{
	GENERATED_BODY()

public:
	ASEEBossGrey();

	// --- Intel / Weakness ---

	/** Set by quest system when player gathers intel from Cars 8/13 */
	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void SetLeftWeaknessRevealed(bool bRevealed) { bLeftWeaknessRevealed = bRevealed; }

	UFUNCTION(BlueprintPure, Category = "Boss|Grey")
	bool IsLeftWeaknessRevealed() const { return bLeftWeaknessRevealed; }

	// --- Guard Squad ---

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void SpawnGuardSquad();

	UFUNCTION(BlueprintPure, Category = "Boss|Grey")
	int32 GetGuardsRemaining() const { return GuardsRemaining; }

	UFUNCTION(BlueprintPure, Category = "Boss|Grey")
	bool AreAllGuardsDead() const { return GuardsRemaining <= 0; }

	// --- Environment ---

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void ActivateSpotlight();

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void DeactivateSpotlight();

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void TriggerSteamConduit();

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void DestroyCommEquipment();

	UFUNCTION(BlueprintPure, Category = "Boss|Grey")
	bool IsCommEquipmentDestroyed() const { return bCommEquipmentDestroyed; }

	// --- Door Sequence (Phase 4) ---

	UFUNCTION(BlueprintCallable, Category = "Boss|Grey")
	void BeginDoorSequence();

	UFUNCTION(BlueprintPure, Category = "Boss|Grey")
	bool IsDoorSequenceActive() const { return bDoorSequenceActive; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Boss|Grey")
	FOnGreyDefeated OnGreyDefeated;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Grey")
	FOnDoorSequenceStarted OnDoorSequenceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Boss|Grey")
	FOnGuardSquadChanged OnGuardSquadChanged;

	// --- Guard spawn configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	TArray<FTransform> EliteSpawnTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	TArray<FTransform> ShieldSpawnTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	FTransform CrossbowSpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	TSubclassOf<AEnemyCharacter> JackbootEliteClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	TSubclassOf<AEnemyCharacter> JackbootShieldClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey|Spawns")
	TSubclassOf<AEnemyCharacter> JackbootCrossbowClass;

protected:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey")
	bool bLeftWeaknessRevealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey")
	bool bCommEquipmentDestroyed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey")
	float LeftSideDamageMultiplier = 1.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Grey")
	int32 GuardsRemaining = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Grey")
	TArray<AEnemyCharacter*> SpawnedGuards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey")
	float DoorSequenceTimer = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Grey")
	float DoorSequenceTimeLimit = 30.f;

private:
	void SetupCommanderGrey();
	void OnGuardKilled(AEnemyCharacter* Guard);
	bool IsAttackFromLeft(AActor* DamageCauser) const;

	bool bDoorSequenceActive = false;
	bool bSpotlightActive = false;
};
