#include "SEEDialogueManager.h"

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
	TArray<FSEEDialogueChoice> Available;
	for (const FSEEDialogueChoice& Choice : CurrentNode.Choices)
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

void USEEDialogueManager::SetFlag(FName FlagName, bool Value)
{
	ConversationFlags.FindOrAdd(FlagName) = Value;
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
