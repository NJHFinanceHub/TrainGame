#include "SkillTreeWidget.h"
#include "SkillTreeComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/SizeBox.h"

// ===== USkillNodeWidget =====

void USkillNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (NodeButton)
	{
		NodeButton->OnClicked.AddDynamic(this, &USkillNodeWidget::HandleNodeClicked);
	}
}

void USkillNodeWidget::InitNode(const FSEESkillNode& InNodeData, bool bInUnlocked, bool bInCanUnlock)
{
	NodeData = InNodeData;
	UpdateState(bInUnlocked, bInCanUnlock);

	if (NodeNameText)
	{
		NodeNameText->SetText(NodeData.DisplayName);
	}

	if (CostText)
	{
		CostText->SetText(FText::AsNumber(NodeData.PointCost));
	}
}

void USkillNodeWidget::UpdateState(bool bInUnlocked, bool bInCanUnlock)
{
	bIsUnlocked = bInUnlocked;
	bCanUnlock = bInCanUnlock;

	if (NodeButton)
	{
		if (bIsUnlocked)
		{
			NodeButton->SetBackgroundColor(FLinearColor(0.2f, 0.8f, 0.3f, 1.0f)); // Green - unlocked
		}
		else if (bCanUnlock)
		{
			NodeButton->SetBackgroundColor(FLinearColor(0.9f, 0.8f, 0.2f, 1.0f)); // Yellow - available
		}
		else
		{
			NodeButton->SetBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f, 0.6f)); // Grey - locked
		}
	}
}

void USkillNodeWidget::HandleNodeClicked()
{
	OnNodeClicked.Broadcast(NodeData.NodeID);
}

// ===== USkillTreeWidget =====

void USkillTreeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CombatTabButton) CombatTabButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnCombatTabClicked);
	if (StealthTabButton) StealthTabButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnStealthTabClicked);
	if (SurvivalTabButton) SurvivalTabButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnSurvivalTabClicked);
	if (LeadershipTabButton) LeadershipTabButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnLeadershipTabClicked);
	if (EngineerTabButton) EngineerTabButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnEngineerTabClicked);
	if (UnlockButton) UnlockButton->OnClicked.AddDynamic(this, &USkillTreeWidget::OnUnlockClicked);
}

void USkillTreeWidget::InitSkillTree(USEESkillTreeComponent* InSkillTreeComp)
{
	SkillTreeComp = InSkillTreeComp;

	if (SkillTreeComp.IsValid())
	{
		SkillTreeComp->OnSkillNodeUnlocked.AddDynamic(this, &USkillTreeWidget::OnSkillNodeUnlocked);
		SkillTreeComp->OnTreePointsChanged.AddDynamic(this, &USkillTreeWidget::OnTreePointsChanged);
	}

	ShowTree(ESEESkillTree::Combat);
}

void USkillTreeWidget::ShowTree(ESEESkillTree Tree)
{
	CurrentTree = Tree;
	SelectedNodeID = NAME_None;

	if (TreeTitleText)
	{
		TreeTitleText->SetText(FText::FromString(GetTreeDisplayName(Tree)));
	}

	RefreshDisplay();
}

void USkillTreeWidget::RefreshDisplay()
{
	ClearTree();
	PopulateTree();
	UpdateInfoPanel();
}

void USkillTreeWidget::ClearTree()
{
	for (USkillNodeWidget* Widget : SpawnedNodeWidgets)
	{
		if (Widget)
		{
			Widget->RemoveFromParent();
		}
	}
	SpawnedNodeWidgets.Empty();
}

void USkillTreeWidget::PopulateTree()
{
	if (!SkillTreeComp.IsValid() || !TreeCanvas || !NodeWidgetClass)
	{
		return;
	}

	TArray<FSEESkillNode> Nodes = SkillTreeComp->GetNodesForTree(CurrentTree);

	// Get canvas size for positioning
	FVector2D CanvasSize(800.0f, 600.0f);
	if (TreeCanvas->GetCachedGeometry().GetLocalSize().X > 0)
	{
		CanvasSize = TreeCanvas->GetCachedGeometry().GetLocalSize();
	}

	for (const FSEESkillNode& Node : Nodes)
	{
		USkillNodeWidget* NodeWidget = CreateWidget<USkillNodeWidget>(this, NodeWidgetClass);
		if (!NodeWidget)
		{
			continue;
		}

		bool bUnlocked = SkillTreeComp->IsNodeUnlocked(Node.NodeID);
		bool bCanUnlock = SkillTreeComp->CanUnlockNode(Node.NodeID);
		NodeWidget->InitNode(Node, bUnlocked, bCanUnlock);
		NodeWidget->OnNodeClicked.AddDynamic(this, &USkillTreeWidget::OnNodeClicked);

		// Position in canvas based on TreePosition (0-1 normalized coords)
		UCanvasPanelSlot* CanvasSlot = TreeCanvas->AddChildToCanvas(NodeWidget);
		if (CanvasSlot)
		{
			float NodeWidth = 120.0f;
			float NodeHeight = 60.0f;
			float X = Node.TreePosition.X * CanvasSize.X - NodeWidth * 0.5f;
			float Y = Node.TreePosition.Y * CanvasSize.Y;
			CanvasSlot->SetPosition(FVector2D(X, Y));
			CanvasSlot->SetSize(FVector2D(NodeWidth, NodeHeight));
		}

		SpawnedNodeWidgets.Add(NodeWidget);
	}
}

void USkillTreeWidget::UpdateInfoPanel()
{
	if (!SkillTreeComp.IsValid())
	{
		return;
	}

	// Update available points
	if (AvailablePointsText)
	{
		AvailablePointsText->SetText(FText::Format(
			FText::FromString("Available Points: {0}"),
			FText::AsNumber(SkillTreeComp->GetAvailableTreePoints())));
	}

	// Update selected node info
	if (SelectedNodeID != NAME_None)
	{
		TArray<FSEESkillNode> Nodes = SkillTreeComp->GetNodesForTree(CurrentTree);
		for (const FSEESkillNode& Node : Nodes)
		{
			if (Node.NodeID == SelectedNodeID)
			{
				if (SelectedNodeName) SelectedNodeName->SetText(Node.DisplayName);
				if (SelectedNodeDesc) SelectedNodeDesc->SetText(Node.Description);

				if (SelectedNodeRequirements)
				{
					FString ReqText = FString::Printf(TEXT("Level %d | Cost: %d points"), Node.RequiredLevel, Node.PointCost);
					if (Node.Prerequisites.Num() > 0)
					{
						ReqText += TEXT("\nRequires: ");
						for (int32 i = 0; i < Node.Prerequisites.Num(); i++)
						{
							if (i > 0) ReqText += TEXT(", ");
							ReqText += Node.Prerequisites[i].ToString();
						}
					}
					SelectedNodeRequirements->SetText(FText::FromString(ReqText));
				}

				if (UnlockButton)
				{
					bool bUnlocked = SkillTreeComp->IsNodeUnlocked(SelectedNodeID);
					bool bCanUnlock = SkillTreeComp->CanUnlockNode(SelectedNodeID);
					UnlockButton->SetIsEnabled(bCanUnlock);

					if (UnlockButtonText)
					{
						if (bUnlocked)
							UnlockButtonText->SetText(FText::FromString("Unlocked"));
						else if (bCanUnlock)
							UnlockButtonText->SetText(FText::FromString("Unlock"));
						else
							UnlockButtonText->SetText(FText::FromString("Locked"));
					}
				}
				break;
			}
		}
	}
	else
	{
		if (SelectedNodeName) SelectedNodeName->SetText(FText::FromString("Select a node"));
		if (SelectedNodeDesc) SelectedNodeDesc->SetText(FText::GetEmpty());
		if (SelectedNodeRequirements) SelectedNodeRequirements->SetText(FText::GetEmpty());
		if (UnlockButton) UnlockButton->SetIsEnabled(false);
	}

	// Update active perks display
	if (ActivePerksText)
	{
		TArray<FSEEPerk> ActivePerks = SkillTreeComp->GetActivePerks();
		int32 MaxPerks = SkillTreeComp->GetMaxActivePerks();
		FString PerkText = FString::Printf(TEXT("Active Perks (%d/%d):"), ActivePerks.Num(), MaxPerks);
		for (const FSEEPerk& Perk : ActivePerks)
		{
			PerkText += FString::Printf(TEXT("\n  - %s"), *Perk.DisplayName.ToString());
		}
		ActivePerksText->SetText(FText::FromString(PerkText));
	}
}

void USkillTreeWidget::SelectNode(FName NodeID)
{
	SelectedNodeID = NodeID;
	UpdateInfoPanel();
}

void USkillTreeWidget::OnNodeClicked(FName NodeID)
{
	SelectNode(NodeID);
}

void USkillTreeWidget::OnUnlockClicked()
{
	if (SkillTreeComp.IsValid() && SelectedNodeID != NAME_None)
	{
		SkillTreeComp->UnlockNode(SelectedNodeID);
		// RefreshDisplay is called via OnSkillNodeUnlocked delegate
	}
}

void USkillTreeWidget::OnCombatTabClicked()		{ ShowTree(ESEESkillTree::Combat); }
void USkillTreeWidget::OnStealthTabClicked()	{ ShowTree(ESEESkillTree::Stealth); }
void USkillTreeWidget::OnSurvivalTabClicked()	{ ShowTree(ESEESkillTree::Survival); }
void USkillTreeWidget::OnLeadershipTabClicked()	{ ShowTree(ESEESkillTree::Leadership); }
void USkillTreeWidget::OnEngineerTabClicked()	{ ShowTree(ESEESkillTree::Engineer); }

void USkillTreeWidget::OnSkillNodeUnlocked(ESEESkillTree Tree, FName NodeID)
{
	if (Tree == CurrentTree)
	{
		RefreshDisplay();
	}
}

void USkillTreeWidget::OnTreePointsChanged(int32 AvailablePoints)
{
	UpdateInfoPanel();
}

FString USkillTreeWidget::GetTreeDisplayName(ESEESkillTree Tree) const
{
	switch (Tree)
	{
	case ESEESkillTree::Combat:		return TEXT("Combat");
	case ESEESkillTree::Stealth:	return TEXT("Stealth");
	case ESEESkillTree::Survival:	return TEXT("Survival");
	case ESEESkillTree::Leadership:	return TEXT("Leadership");
	case ESEESkillTree::Engineer:	return TEXT("Engineer");
	default:						return TEXT("Unknown");
	}
}
