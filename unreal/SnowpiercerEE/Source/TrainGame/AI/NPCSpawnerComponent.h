// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGameAITypes.h"
#include "NPCSpawnerComponent.generated.h"

class ANPCAIController;
class UDataTable;

// ============================================================================
// UNPCSpawnerComponent
//
// Data-driven NPC spawner attached to train car actors. Reads spawn
// definitions from a DataTable (FNPCSpawnRow) and spawns NPCs at
// tagged locations or random nav mesh positions within the car.
// Handles initial schedule/patrol setup for spawned NPCs.
// ============================================================================

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCSpawnerComponent();

	virtual void BeginPlay() override;

	// --- Spawning ---

	/** Spawn all NPCs defined for this car */
	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	void SpawnNPCs();

	/** Despawn all NPCs managed by this spawner */
	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	void DespawnAll();

	/** Respawn a specific NPC that was killed or removed */
	UFUNCTION(BlueprintCallable, Category = "NPC Spawner")
	void RespawnNPC(int32 SpawnIndex);

	/** Get all currently alive NPCs spawned by this component */
	UFUNCTION(BlueprintPure, Category = "NPC Spawner")
	TArray<APawn*> GetSpawnedNPCs() const { return SpawnedNPCs; }

	/** Get the number of active NPCs */
	UFUNCTION(BlueprintPure, Category = "NPC Spawner")
	int32 GetActiveNPCCount() const;

protected:
	/** DataTable with NPC spawn definitions (rows of FNPCSpawnRow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Spawner")
	UDataTable* SpawnDataTable;

	/** Tag that identifies this car (matched against FNPCSpawnRow::CarTag) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Spawner")
	FName CarTag = NAME_None;

	/** Whether to spawn NPCs on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Spawner")
	bool bSpawnOnBeginPlay = true;

	/** Maximum NPCs this spawner will manage (performance cap) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Spawner")
	int32 MaxNPCs = 20;

	/** Respawn delay after NPC death (seconds, 0 = no respawn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Spawner")
	float RespawnDelay = 0.f;

private:
	/** Spawn a single NPC from a row definition */
	APawn* SpawnSingleNPC(const FNPCSpawnRow& Row, int32 SpawnPointIndex);

	/** Find a spawn location for the NPC */
	FVector GetSpawnLocation(const FNPCSpawnRow& Row, int32 SpawnPointIndex) const;

	/** Find actors with a specific tag within the owning car */
	TArray<AActor*> FindTaggedActors(FName Tag) const;

	/** Configure the spawned NPC's AI controller */
	void ConfigureNPCAI(APawn* SpawnedPawn, const FNPCSpawnRow& Row);

	UPROPERTY()
	TArray<APawn*> SpawnedNPCs;

	/** Cached spawn rows for this car */
	TArray<FNPCSpawnRow> CachedSpawnRows;
};
