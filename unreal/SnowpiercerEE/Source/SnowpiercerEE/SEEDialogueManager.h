#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SEEDialogueManager.generated.h"

UENUM(BlueprintType)
enum class ESEEDialogueNodeType : uint8
{
	NPCLine		UMETA(DisplayName = "NPC Line"),
	PlayerChoice UMETA(DisplayName = "Player Choice"),
	SkillCheck	UMETA(DisplayName = "Skill Check"),
	Branch		UMETA(DisplayName = "Branch"),
	SetFlag		UMETA(DisplayName = "Set Flag"),
	End			UMETA(DisplayName = "End")
};

USTRUCT(BlueprintType)
struct FSEEDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NextNodeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RequiredStat;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredStatValue = 0;
};

USTRUCT(BlueprintType)
struct FSEEDialogueNode : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEDialogueNodeType NodeType = ESEEDialogueNodeType::NPCLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSEEDialogueChoice> Choices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NextNodeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName FlagToSet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool FlagValue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BranchFlag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BranchTrueNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BranchFalseNode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimedResponseDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> VoiceAudio;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, FName, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueNodeChanged, const FSEEDialogueNode&, CurrentNode);

UCLASS()
class SNOWPIERCEREE_API USEEDialogueManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartConversation(FName ConversationID, UDataTable* DialogueTable);

	/**
	 * Start a conversation directly at a node row in the Zone 1 dialogue table
	 * (lazily loaded from /Game/DataTables/DT_Dialogue_Zone1). Used by NPC
	 * interaction, where each NPC stores its entry row (Pike_01, Dealer_01, ...).
	 * Returns true if the conversation is running after processing the node.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	bool StartConversationAtNode(FName EntryNodeID);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndConversation();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInConversation() const { return bInConversation; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FSEEDialogueNode GetCurrentNode() const { return CurrentNode; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	TArray<FSEEDialogueChoice> GetAvailableChoices() const;

	/**
	 * If the current node is an NPC line that flows straight into a PlayerChoice
	 * node (the standard "greeting then responses" pattern in DT_Dialogue_Zone1),
	 * return that follow-on node's available choices so the UI can present the
	 * NPC line and its responses together on first display. Returns empty when the
	 * current node has no such follow-on (the player still presses to continue).
	 */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	TArray<FSEEDialogueChoice> GetUpcomingChoices() const;

	/** True when GetUpcomingChoices() would yield a folded PlayerChoice node. */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasUpcomingChoices() const;

	/**
	 * Select a choice that was presented "folded" onto the preceding NPC line:
	 * advance from the current NPC line to its PlayerChoice node, then pick the
	 * choice. Keeps node broadcasts (and quest tracking) intact for both nodes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceAndSelectChoice(int32 ChoiceIndex);

	// Flag system (NPC memory)
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetFlag(FName FlagName, bool Value);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool GetFlag(FName FlagName) const;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueNodeChanged OnDialogueNodeChanged;

private:
	void ProcessNode(FName NodeID);
	const FSEEDialogueNode* FindNode(FName NodeID) const;

	/** Filter a node's choices by their RequiredFlag against the flag store. */
	TArray<FSEEDialogueChoice> FilterAvailableChoices(const FSEEDialogueNode& Node) const;

	/** The PlayerChoice node the current NPC line flows into, or null. */
	const FSEEDialogueNode* GetFollowOnChoiceNode() const;

	/** Add a one-shot dialogue-gift item (Item_<X>) to the player's inventory. */
	void GrantRewardItem(FName ItemID);

	/** Lazy-loads the Zone 1 dialogue DataTable asset. */
	UDataTable* GetZone1DialogueTable();

	UPROPERTY()
	TObjectPtr<UDataTable> ActiveDialogueTable;

	UPROPERTY()
	TObjectPtr<UDataTable> Zone1DialogueTable;

	TMap<FName, bool> ConversationFlags;
	FSEEDialogueNode CurrentNode;
	FName ActiveConversationID;
	bool bInConversation = false;
};
