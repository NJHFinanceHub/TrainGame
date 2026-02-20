// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "NPCSpawnerComponent.generated.h"

class UDataTable;

// ============================================================================
// UNPCSpawnerComponent
//
// Data-driven NPC spawner attached to train car actors. Reads spawn
// configuration from a DataTable, spawns NPCs at designated points,
// and manages their lifecycle (despawn when car is far from player,
// respawn when player approaches).
// ============================================================================

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCSpawnerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Spawning ---

	/** Spawn all NPCs defined in the spawn table */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawner")
	void SpawnAllNPCs();

	/** Despawn all managed NPCs */
	UFUNCTION(BlueprintCallable, Category = "AI|Spawner")
	void DespawnAllNPCs();

	/** Check if NPCs are currently spawned */
	UFUNCTION(BlueprintPure, Category = "AI|Spawner")
	bool AreNPCsSpawned() const { return SpawnedNPCs.Num() > 0; }

	/** Get all currently spawned NPCs */
	UFUNCTION(BlueprintPure, Category = "AI|Spawner")
	const TArray<AActor*>& GetSpawnedNPCs() const { return SpawnedNPCs; }

protected:
	virtual void BeginPlay() override;

	/** Data table with NPC spawn entries (rows of FNPCSpawnEntry) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	UDataTable* SpawnTable = nullptr;

	/** Spawn points within this car (set in editor or auto-detected) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	TArray<FVector> SpawnPoints;

	/** Distance from player at which to activate/deactivate NPCs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	float ActivationDistance = 5000.f;

	/** Whether to auto-spawn on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	bool bAutoSpawn = true;

	/** Whether to manage spawn/despawn based on player distance */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Spawner")
	bool bDistanceManaged = true;

private:
	void CheckPlayerDistance();
	AActor* SpawnSingleNPC(const FNPCSpawnEntry& Entry, FVector Location);

	UPROPERTY()
	TArray<AActor*> SpawnedNPCs;

	float DistanceCheckTimer = 0.f;
	bool bPlayerInRange = false;
};
