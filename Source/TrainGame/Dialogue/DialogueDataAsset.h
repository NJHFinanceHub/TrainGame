// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueTypes.h"
#include "DialogueDataAsset.generated.h"

/**
 * UDialogueDataAsset
 *
 * Contains the full dialogue graph for a single NPC. Authored in editor or
 * imported from external dialogue tools. Each NPC references one of these.
 */
UCLASS(BlueprintType)
class TRAINGAME_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** NPC identifier this dialogue belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName NPCID;

	/** Display name for the NPC speaker */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FText NPCDisplayName;

	/** The hub node ID (entry point) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FName HubNodeID;

	/** All dialogue nodes in this graph */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TArray<FDialogueNode> Nodes;

	/** NPC archetype for social resistance lookup */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	ENPCArchetype Archetype = ENPCArchetype::Guard;

	/** Base social resistance values */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	FSocialResistance SocialResistance;

	/** Starting disposition toward player */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	int32 InitialDisposition = 0;

	// --- Accessors ---

	/** Find a node by ID */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FDialogueNode GetNodeByID(FName NodeID) const;

	/** Get the hub (entry) node */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FDialogueNode GetHubNode() const;

	/** Get all nodes of a specific type */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	TArray<FDialogueNode> GetNodesByType(EDialogueNodeType Type) const;
};
