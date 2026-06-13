// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCDialogueComponent.h"
#include "Net/UnrealNetwork.h"

USEENPCDialogueComponent::USEENPCDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Replicated to all clients so any guest's local dialogue path can resolve this
	// NPC's conversation (the entry id is not owner-private — every client may walk
	// up and talk, so this is a plain DOREPLIFETIME, not COND_OwnerOnly).
	SetIsReplicatedByDefault(true);
}

void USEENPCDialogueComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USEENPCDialogueComponent, ReplicatedDialogueEntryId);
	DOREPLIFETIME(USEENPCDialogueComponent, bReplicatedCanStartDialogue);
}

void USEENPCDialogueComponent::SetDialogueState(FName EntryId, bool bCanStart)
{
	// Authority-only mirror update; the values then replicate to every client.
	ReplicatedDialogueEntryId = EntryId;
	bReplicatedCanStartDialogue = bCanStart;
}
