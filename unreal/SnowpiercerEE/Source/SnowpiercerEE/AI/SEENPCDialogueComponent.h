// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEENPCDialogueComponent.generated.h"

/**
 * USEENPCDialogueComponent
 *
 * CO-OP DIALOGUE BRIDGE.
 *
 * The per-NPC dialogue entry row (DialogueEntryNode) and the "can I talk right
 * now" check (CanStartDialogue) live on ASEENPCAIController, which is SERVER-ONLY
 * — AI controllers don't exist on clients. So a client's
 * Cast<ASEENPCAIController>(NPCPawn->GetController()) is null, and the old
 * dialogue path early-returned: guests literally could not talk to NPCs.
 *
 * This tiny replicated component is attached by the brain to its possessed pawn
 * (the same way it attaches the health / anim-driver components to adopted
 * plain-ACharacter NPCs). The server mirrors the controller's resolved entry id
 * and talkable flag onto it; because it lives on the PAWN — a replicated actor —
 * both fields replicate to every client. The client dialogue path then reads the
 * entry id from the pawn's component instead of the missing controller, and opens
 * the LOCAL dialogue UI with that entry (the dialogue DataTable exists on every
 * machine, so the conversation renders locally).
 *
 * Host/standalone read the SAME replicated field (set inline on authority), so
 * there is one code path everywhere — single-player dialogue is unchanged.
 */
UCLASS(ClassGroup=(Custom))
class SNOWPIERCEREE_API USEENPCDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEENPCDialogueComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** The DT_Dialogue_Zone1 entry row for this NPC (mirrors the controller's
	    DialogueEntryNode). NAME_None = this NPC has no conversation. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Dialogue")
	FName ReplicatedDialogueEntryId;

	/** Whether the player may currently open dialogue with this NPC (mirrors the
	    controller's CanStartDialogue(): alive, has an entry, not mid-fight, standing
	    not too low). Recomputed server-side and replicated. */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Dialogue")
	bool bReplicatedCanStartDialogue = false;

	/** Authority-side setter the brain calls to keep the replicated mirror current. */
	void SetDialogueState(FName EntryId, bool bCanStart);
};
