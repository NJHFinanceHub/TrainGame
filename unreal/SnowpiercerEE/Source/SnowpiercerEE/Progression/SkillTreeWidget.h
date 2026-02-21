#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillTreeTypes.h"
#include "SkillTreeWidget.generated.h"

class USEESkillTreeComponent;
class UCanvasPanel;
class UButton;
class UTextBlock;
class UImage;
class UVerticalBox;
class USizeBox;

// Represents a single node button in the skill tree UI
UCLASS()
class SNOWPIERCEREE_API USkillNodeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "SkillTree|UI")
	void InitNode(const FSEESkillNode& InNodeData, bool bInUnlocked, bool bInCanUnlock);

	UFUNCTION(BlueprintCallable, Category = "SkillTree|UI")
	void UpdateState(bool bInUnlocked, bool bInCanUnlock);

	UPROPERTY(BlueprintReadOnly, Category = "SkillTree|UI")
	FSEESkillNode NodeData;

	UPROPERTY(BlueprintReadOnly, Category = "SkillTree|UI")
	bool bIsUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "SkillTree|UI")
	bool bCanUnlock = false;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNodeClicked, FName, NodeID);

	UPROPERTY(BlueprintAssignable, Category = "SkillTree|UI")
	FOnNodeClicked OnNodeClicked;

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> NodeButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> NodeNameText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UImage> NodeIcon;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> CostText;

	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleNodeClicked();
};

// Main skill tree UI widget
UCLASS()
class SNOWPIERCEREE_API USkillTreeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Initialize with the player's skill tree component
	UFUNCTION(BlueprintCallable, Category = "SkillTree|UI")
	void InitSkillTree(USEESkillTreeComponent* InSkillTreeComp);

	// Switch which tree is displayed
	UFUNCTION(BlueprintCallable, Category = "SkillTree|UI")
	void ShowTree(ESEESkillTree Tree);

	// Refresh the display (after unlock, etc.)
	UFUNCTION(BlueprintCallable, Category = "SkillTree|UI")
	void RefreshDisplay();

	// Get the currently displayed tree
	UFUNCTION(BlueprintPure, Category = "SkillTree|UI")
	ESEESkillTree GetCurrentTree() const { return CurrentTree; }

protected:
	virtual void NativeConstruct() override;

	// The canvas where nodes are placed
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UCanvasPanel> TreeCanvas;

	// Tree tab buttons
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> CombatTabButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> StealthTabButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> SurvivalTabButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> LeadershipTabButton;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UButton> EngineerTabButton;

	// Info panel
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TreeTitleText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> AvailablePointsText;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> SelectedNodeName;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> SelectedNodeDesc;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> SelectedNodeRequirements;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UButton> UnlockButton;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> UnlockButtonText;

	// Perk section
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UVerticalBox> PerkListBox;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	TObjectPtr<UTextBlock> ActivePerksText;

	// Node widget class (set in Blueprint)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SkillTree|UI")
	TSubclassOf<USkillNodeWidget> NodeWidgetClass;

private:
	UPROPERTY()
	TWeakObjectPtr<USEESkillTreeComponent> SkillTreeComp;

	ESEESkillTree CurrentTree = ESEESkillTree::Combat;
	FName SelectedNodeID;

	UPROPERTY()
	TArray<TObjectPtr<USkillNodeWidget>> SpawnedNodeWidgets;

	void ClearTree();
	void PopulateTree();
	void DrawConnections();
	void UpdateInfoPanel();
	void SelectNode(FName NodeID);

	UFUNCTION()
	void OnNodeClicked(FName NodeID);

	UFUNCTION()
	void OnUnlockClicked();

	UFUNCTION()
	void OnCombatTabClicked();

	UFUNCTION()
	void OnStealthTabClicked();

	UFUNCTION()
	void OnSurvivalTabClicked();

	UFUNCTION()
	void OnLeadershipTabClicked();

	UFUNCTION()
	void OnEngineerTabClicked();

	UFUNCTION()
	void OnSkillNodeUnlocked(ESEESkillTree Tree, FName NodeID);

	UFUNCTION()
	void OnTreePointsChanged(int32 AvailablePoints);

	FString GetTreeDisplayName(ESEESkillTree Tree) const;
};
