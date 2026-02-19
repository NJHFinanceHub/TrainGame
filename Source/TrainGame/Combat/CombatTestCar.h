// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrainGame/Core/CombatTypes.h"
#include "CombatTestCar.generated.h"

class UEnvironmentalHazardComponent;

// ============================================================================
// ACombatTestCar
//
// A self-contained test car for prototyping the combat system.
// Spawns a tight corridor environment with environmental hazards,
// enemy spawn points, and weapon pickups. Represents a single
// Snowpiercer car (~50m long, standard gauge width).
//
// This is "The Pit" (Car 3) - where Tailies fight for extra rations.
// ============================================================================

UCLASS()
class TRAINGAME_API ACombatTestCar : public AActor
{
	GENERATED_BODY()

public:
	ACombatTestCar();

	/** Spawn all enemies and reset the car for combat */
	UFUNCTION(BlueprintCallable, Category = "TestCar")
	void InitializeCombatEncounter();

	/** Reset the car to initial state */
	UFUNCTION(BlueprintCallable, Category = "TestCar")
	void ResetCar();

	/** Get count of remaining alive enemies */
	UFUNCTION(BlueprintPure, Category = "TestCar")
	int32 GetRemainingEnemyCount() const;

protected:
	virtual void BeginPlay() override;

	// --- Car Dimensions (Snowpiercer standard) ---

	/** Car length in cm (~50 meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Dimensions")
	float CarLength = 5000.f;

	/** Car width in cm (~3 meters for standard gauge, tight corridors) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Dimensions")
	float CarWidth = 300.f;

	/** Car height in cm (~2.8 meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Dimensions")
	float CarHeight = 280.f;

	// --- Enemy Spawning ---

	/** Class to spawn for enemies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Enemies")
	TSubclassOf<APawn> EnemyClass;

	/** Number of enemies in this encounter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Enemies")
	int32 EnemyCount = 6;

	/** Spawn points for enemies (if empty, auto-distribute along car length) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Enemies")
	TArray<FVector> EnemySpawnOffsets;

	// --- Environmental Hazards ---

	/** Steam vent positions (local offsets from car center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Hazards")
	TArray<FVector> SteamVentPositions;

	/** Electrical panel positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Hazards")
	TArray<FVector> ElectricalPanelPositions;

	/** Window breach positions (along the car walls) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Hazards")
	TArray<FVector> WindowBreachPositions;

	// --- Weapon Pickups ---

	/** Weapons available in this test car (for prototype testing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Weapons")
	TArray<FWeaponStats> AvailableWeapons;

	/** Positions to place weapon pickups */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TestCar|Weapons")
	TArray<FVector> WeaponSpawnPositions;

private:
	void SetupDefaultLayout();
	void SpawnHazards();
	void SpawnEnemies();
	void SpawnWeaponPickups();

	UFUNCTION()
	void OnEnemyDeath(AActor* Killer);

	UPROPERTY()
	TArray<AActor*> SpawnedEnemies;

	UPROPERTY()
	TArray<AActor*> SpawnedHazards;

	int32 EnemiesRemaining = 0;
};
