// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEENPCBrainSubsystem.generated.h"

class ACharacter;
class ASEENPCAIController;

// ============================================================================
// USEENPCBrainSubsystem
//
// Runtime adoption layer that gives the placed NPC blueprints a working brain.
//
// The level's BP_NPC_* blueprints were authored against a C++ parent that did
// not exist at creation time, so they fell back to plain ACharacter: no AI
// controller class, no AutoPossessAI, no health component. Reparenting the
// assets requires the editor, so instead this world subsystem scans the level
// shortly after BeginPlay (and periodically, to catch streamed-in cars),
// spawns an ASEENPCAIController for every un-possessed NPC pawn, configures
// it per NPC type (hostile jackboots/boss vs. friendly civilians/merchant,
// stats, dialogue entry node) and possesses the pawn.
//
// Identification uses the generated-class name (BP_NPC_Jackboot_C, ...), the
// C++ NPC base classes, and the actor label (NPC_* / Boss_* set by the level
// population script) for the named characters' dialogue trees.
// ============================================================================

UCLASS()
class SNOWPIERCEREE_API USEENPCBrainSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	/** Periodic pass: possess and configure any NPC pawn that lacks a brain. */
	void ScanForUnpossessedNPCs();

	/** Returns true if the pawn is an NPC this subsystem should drive. */
	static bool ShouldAdoptPawn(const ACharacter* PawnChar);

	/** Spawn, configure and possess a brain controller for the pawn. */
	void AdoptPawn(ACharacter* PawnChar);

	/** Per-NPC-type tuning: hostility, stats, wander and dialogue entry node. */
	static void ConfigureController(ASEENPCAIController* Controller, const ACharacter* PawnChar);

	/** Map a placed NPC (label + class) to its DT_Dialogue_Zone1 entry row. */
	static FName PickDialogueEntryNode(const FString& Identity, bool bMerchant, bool bBreachman);

	FTimerHandle ScanTimerHandle;
};
