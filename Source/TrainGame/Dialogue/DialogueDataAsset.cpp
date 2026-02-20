// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DialogueDataAsset.h"

FDialogueNode UDialogueDataAsset::GetNodeByID(FName NodeID) const
{
	for (const FDialogueNode& Node : Nodes)
	{
		if (Node.NodeID == NodeID)
		{
			return Node;
		}
	}

	// Return empty node if not found
	FDialogueNode Empty;
	return Empty;
}

FDialogueNode UDialogueDataAsset::GetHubNode() const
{
	return GetNodeByID(HubNodeID);
}

TArray<FDialogueNode> UDialogueDataAsset::GetNodesByType(EDialogueNodeType Type) const
{
	TArray<FDialogueNode> Result;
	for (const FDialogueNode& Node : Nodes)
	{
		if (Node.NodeType == Type)
		{
			Result.Add(Node);
		}
	}
	return Result;
}
