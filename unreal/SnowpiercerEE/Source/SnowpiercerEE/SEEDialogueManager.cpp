#include "SEEDialogueManager.h"
#include "SEEInventoryComponent.h"
#include "SEECharacter.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void USEEDialogueManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USEEDialogueManager::StartConversation(FName ConversationID, UDataTable* DialogueTable)
{
	if (bInConversation || !DialogueTable) return;

	ActiveDialogueTable = DialogueTable;
	ActiveConversationID = ConversationID;
	bInConversation = true;

	OnDialogueStarted.Broadcast(ConversationID);

	// Start with first node (convention: ConversationID_Start)
	FName StartNode = FName(*(ConversationID.ToString() + TEXT("_Start")));
	ProcessNode(StartNode);
}

bool USEEDialogueManager::StartConversationAtNode(FName EntryNodeID)
{
	if (bInConversation || EntryNodeID.IsNone()) return false;

	UDataTable* Table = GetZone1DialogueTable();
	if (!Table) return false;
	if (!Table->FindRow<FSEEDialogueNode>(EntryNodeID, TEXT("StartConversationAtNode"), false)) return false;

	ActiveDialogueTable = Table;
	ActiveConversationID = EntryNodeID;
	bInConversation = true;

	OnDialogueStarted.Broadcast(EntryNodeID);
	ProcessNode(EntryNodeID);

	// SetFlag/Branch/End chains can finish the conversation immediately.
	return bInConversation;
}

void USEEDialogueManager::AdvanceDialogue()
{
	if (!bInConversation) return;

	if (CurrentNode.NodeType == ESEEDialogueNodeType::NPCLine)
	{
		if (!CurrentNode.NextNodeID.IsNone())
		{
			ProcessNode(CurrentNode.NextNodeID);
		}
		else
		{
			EndConversation();
		}
	}
}

void USEEDialogueManager::SelectChoice(int32 ChoiceIndex)
{
	if (!bInConversation) return;
	if (CurrentNode.NodeType != ESEEDialogueNodeType::PlayerChoice) return;

	TArray<FSEEDialogueChoice> Available = GetAvailableChoices();
	if (ChoiceIndex >= 0 && ChoiceIndex < Available.Num())
	{
		ProcessNode(Available[ChoiceIndex].NextNodeID);
	}
}

void USEEDialogueManager::EndConversation()
{
	bInConversation = false;
	ActiveDialogueTable = nullptr;
	ActiveConversationID = NAME_None;
	OnDialogueEnded.Broadcast();
}

TArray<FSEEDialogueChoice> USEEDialogueManager::GetAvailableChoices() const
{
	return FilterAvailableChoices(CurrentNode);
}

TArray<FSEEDialogueChoice> USEEDialogueManager::FilterAvailableChoices(const FSEEDialogueNode& Node) const
{
	TArray<FSEEDialogueChoice> Available;
	for (const FSEEDialogueChoice& Choice : Node.Choices)
	{
		// Check required flag
		if (!Choice.RequiredFlag.IsNone() && !GetFlag(Choice.RequiredFlag))
		{
			continue;
		}
		Available.Add(Choice);
	}
	return Available;
}

const FSEEDialogueNode* USEEDialogueManager::GetFollowOnChoiceNode() const
{
	// Only NPC lines fold their responses forward; choice nodes already carry them.
	if (!bInConversation || CurrentNode.NodeType != ESEEDialogueNodeType::NPCLine)
	{
		return nullptr;
	}
	if (CurrentNode.NextNodeID.IsNone())
	{
		return nullptr;
	}

	const FSEEDialogueNode* Next = FindNode(CurrentNode.NextNodeID);
	if (Next && Next->NodeType == ESEEDialogueNodeType::PlayerChoice)
	{
		return Next;
	}
	return nullptr;
}

TArray<FSEEDialogueChoice> USEEDialogueManager::GetUpcomingChoices() const
{
	if (const FSEEDialogueNode* ChoiceNode = GetFollowOnChoiceNode())
	{
		return FilterAvailableChoices(*ChoiceNode);
	}
	return TArray<FSEEDialogueChoice>();
}

bool USEEDialogueManager::HasUpcomingChoices() const
{
	return GetUpcomingChoices().Num() > 0;
}

void USEEDialogueManager::AdvanceAndSelectChoice(int32 ChoiceIndex)
{
	if (!bInConversation) return;

	// Move from the NPC line to its PlayerChoice node first. ProcessNode broadcasts
	// the choice node (so quest tracking sees it), then SelectChoice validates
	// against it and routes to the picked branch.
	const FSEEDialogueNode* ChoiceNode = GetFollowOnChoiceNode();
	if (!ChoiceNode)
	{
		// Current node already carries the choices (or none) — select directly.
		SelectChoice(ChoiceIndex);
		return;
	}

	ProcessNode(CurrentNode.NextNodeID);
	SelectChoice(ChoiceIndex);
}

void USEEDialogueManager::SetFlag(FName FlagName, bool Value)
{
	const bool bWasSet = GetFlag(FlagName);
	ConversationFlags.FindOrAdd(FlagName) = Value;

	// Dialogue-direct item gifts: a node that sets "Reward_<Item>" hands the
	// player Item_<Item> the first time the flag goes true. Quest-completion
	// rewards are handled separately by the quest manager, so anything a quest
	// already grants is excluded here to avoid a double-grant.
	if (Value && !bWasSet && FlagName.ToString().StartsWith(TEXT("Reward_")))
	{
		static const TSet<FName> QuestHandledRewards = { FName(TEXT("Reward_TinStar")) };
		if (!QuestHandledRewards.Contains(FlagName))
		{
			const FString ItemSuffix = FlagName.ToString().RightChop(7); // after "Reward_"
			GrantRewardItem(FName(*(FString(TEXT("Item_")) + ItemSuffix)));
		}
	}
}

void USEEDialogueManager::GrantRewardItem(FName ItemID)
{
	if (ItemID.IsNone()) return;
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	// Dialogue runs locally on each machine, so the local player controller is the
	// player who is talking (the host on the host, a guest on a guest).
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn) return;

	// CO-OP: the item gift is a WORLD/QUEST side-effect and must be server-
	// authoritative. Route it through the player pawn's authoritative grant path
	// (direct on host/standalone, Server RPC on a guest) instead of calling the
	// inventory directly, so a guest's reward isn't a client-only mutation that the
	// next inventory replication overwrites. Pure conversation navigation stays local.
	if (ASEECharacter* Character = Cast<ASEECharacter>(Pawn))
	{
		Character->GrantDialogueRewardItem(ItemID);
		UE_LOG(LogTemp, Log, TEXT("Dialogue reward granted: %s"), *ItemID.ToString());
		return;
	}

	// Fallback (non-ASEECharacter pawn): add directly — authority decides validity.
	if (USEEInventoryComponent* Inv = Pawn->FindComponentByClass<USEEInventoryComponent>())
	{
		Inv->AddItem(ItemID, 1);
		UE_LOG(LogTemp, Log, TEXT("Dialogue reward granted: %s"), *ItemID.ToString());
	}
}

bool USEEDialogueManager::GetFlag(FName FlagName) const
{
	const bool* Val = ConversationFlags.Find(FlagName);
	return Val ? *Val : false;
}

void USEEDialogueManager::ProcessNode(FName NodeID)
{
	const FSEEDialogueNode* Node = FindNode(NodeID);
	if (!Node)
	{
		EndConversation();
		return;
	}

	CurrentNode = *Node;
	OnDialogueNodeChanged.Broadcast(CurrentNode);

	switch (Node->NodeType)
	{
	case ESEEDialogueNodeType::SetFlag:
		SetFlag(Node->FlagToSet, Node->FlagValue);
		if (!Node->NextNodeID.IsNone())
		{
			ProcessNode(Node->NextNodeID);
		}
		break;

	case ESEEDialogueNodeType::Branch:
		if (GetFlag(Node->BranchFlag))
		{
			ProcessNode(Node->BranchTrueNode);
		}
		else
		{
			ProcessNode(Node->BranchFalseNode);
		}
		break;

	case ESEEDialogueNodeType::End:
		EndConversation();
		break;

	default:
		// NPCLine, PlayerChoice, SkillCheck — wait for player input
		break;
	}
}

const FSEEDialogueNode* USEEDialogueManager::FindNode(FName NodeID) const
{
	if (!ActiveDialogueTable) return nullptr;
	return ActiveDialogueTable->FindRow<FSEEDialogueNode>(NodeID, TEXT(""));
}

UDataTable* USEEDialogueManager::GetZone1DialogueTable()
{
	if (!Zone1DialogueTable)
	{
		Zone1DialogueTable = LoadObject<UDataTable>(nullptr,
			TEXT("/Game/DataTables/DT_Dialogue_Zone1.DT_Dialogue_Zone1"));
	}
	return Zone1DialogueTable;
}
