// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "NPCSpawnerComponent.generated.h"

class ASEENPCCharacter;

// ============================================================================
// UNPCSpawnerComponent
//
// Data-driven NPC spawner, attached to train car actors. Reads a DataTable
// of FNPCSpawnEntry rows to determine what NPCs to spawn, where, and with
// what schedules. Supports despawning NPCs when the car is far from the
// player and respawning when the player approaches.
// ============================================================================

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCSpawnerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCSpawnerComponent();

	virtual void BeginPlay() override;

	/** Spawn all NPCs defined in the spawn table */
	UFUNCTION(BlueprintCallable, Category = "NPC|Spawner")
	void SpawnAllNPCs();

	/** Despawn all managed NPCs (e.g., when car is unloaded) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Spawner")
	void DespawnAllNPCs();

	/** Respawn NPCs that were despawned */
	UFUNCTION(BlueprintCallable, Category = "NPC|Spawner")
	void RespawnNPCs();

	UFUNCTION(BlueprintPure, Category = "NPC|Spawner")
	int32 GetSpawnedNPCCount() const { return SpawnedNPCs.Num(); }

	UFUNCTION(BlueprintPure, Category = "NPC|Spawner")
	const TArray<ASEENPCCharacter*>& GetSpawnedNPCs() const { return SpawnedNPCs; }

protected:
	/** DataTable of FNPCSpawnEntry rows defining what to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawner")
	UDataTable* SpawnTable;

	/** Whether to auto-spawn on BeginPlay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawner")
	bool bAutoSpawn = true;

	/** Distance from player beyond which NPCs are despawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawner")
	float DespawnDistance = 10000.f;

	/** Distance from player within which NPCs are respawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Spawner")
	float RespawnDistance = 8000.f;

private:
	ASEENPCCharacter* SpawnNPC(const FNPCSpawnEntry& Entry, FVector SpawnLocation);
	FVector ResolveSpawnLocation(const FNPCSpawnEntry& Entry) const;

	UPROPERTY()
	TArray<ASEENPCCharacter*> SpawnedNPCs;

	bool bHasDespawned = false;
};
