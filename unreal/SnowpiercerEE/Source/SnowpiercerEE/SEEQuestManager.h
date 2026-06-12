#pragma once

#include <initializer_list>

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEEDialogueManager.h" // FSEEDialogueNode (dialogue-driven objective progress)
#include "SEEQuestManager.generated.h"

class USEEInventoryComponent;

UENUM(BlueprintType)
enum class ESEEQuestState : uint8
{
	Available	UMETA(DisplayName = "Available"),
	Active		UMETA(DisplayName = "Active"),
	Completed	UMETA(DisplayName = "Completed"),
	Failed		UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class ESEEObjectiveType : uint8
{
	GoTo		UMETA(DisplayName = "Go To"),
	Interact	UMETA(DisplayName = "Interact"),
	Kill		UMETA(DisplayName = "Kill"),
	Collect		UMETA(DisplayName = "Collect"),
	Escort		UMETA(DisplayName = "Escort"),
	Dialogue	UMETA(DisplayName = "Dialogue"),
	Custom		UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FSEEQuestObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEObjectiveType Type = ESEEObjectiveType::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TargetID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOptional = false;
};

USTRUCT(BlueprintType)
struct FSEEQuest : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEQuestState State = ESEEQuestState::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsMainQuest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSEEQuestObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 XPReward = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ItemRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> FactionRepRewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> PrerequisiteQuests;
};

/** Minimal per-quest snapshot for save games (see CaptureQuestSaveState). */
USTRUCT(BlueprintType)
struct FSEEQuestSaveState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	FName QuestID;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	ESEEQuestState State = ESEEQuestState::Available;

	/** Parallel to FSEEQuest::Objectives (by index). */
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	TArray<int32> ObjectiveCounts;

	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite)
	TArray<bool> ObjectiveCompleted;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, FName, QuestID, ESEEQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, FName, ObjectiveID);

// ============================================================================
// USEEQuestManager
//
// Data-driven quest lifecycle + objective progress engine.
//
// Quests load lazily from /Game/DataTables/DT_Quests (FSEEQuest rows). Main
// quests whose prerequisites are met auto-accept so the main arc always flows;
// side quests accept via AcceptQuest(), or automatically when dialogue sets a
// "Quest_<Fragment>_Started" flag whose fragment appears in a QuestID.
//
// Objective progress signals (TargetID conventions from DT_Quests):
//   Dialogue : completes when the conversation visits a node whose NodeID ==
//              TargetID (entry nodes like "Pike_01"), OR when a dialogue flag
//              named TargetID is set true (e.g. "Dealer_Persuade"). Both paths
//              are supported; flags also catch up retroactively via GetFlag.
//   Collect  : TargetID is an ItemID ("Item_GateKey") counted via the player
//              inventory's OnItemAdded, or a collectible ID
//              ("Collect_Intel_Z1_01") via UCollectibleJournalSubsystem.
//              On activation the objective catches up from current counts.
//   Kill     : NPC controllers report deaths through ReportNPCDeath().
//              TargetID ("NPC_KronoleDealer") matches when its fragment
//              (minus the "NPC_" prefix) appears in the dead pawn's class
//              name or actor name (case-insensitive).
//   GoTo     : 1s poll maps player X to a car index (car = floor((X+6500)/
//              13000)). TargetID begins "Car<NN>" ("Car14_UnionHall");
//              completes when the player is at/crosses that car. Non-car
//              targets ("TailGate") complete via NotifyInteract or a dialogue
//              flag of the same name.
//   Interact : NotifyInteract(TargetID) from any interact path ("NPC_Cook"),
//              with a dialogue-flag fallback (flag name == TargetID) so the
//              arc cannot dead-end before world interactables call in.
//   Escort / Custom : NotifyInteract(TargetID) or dialogue flag == TargetID
//              (e.g. "Craft_ReinforcedShiv" once crafting reports it).
//
// Objectives gate sequentially: only the first incomplete non-optional
// objective is active; optional objectives are live for the whole quest.
// Completing all non-optional objectives completes the quest and pays out
// XPReward (player USEEStatsComponent), ItemRewards (AddItem x1 each) and
// FactionRepRewards (USEEFactionManager::ModifyReputation), then newly
// unlocked main quests auto-accept.
// ============================================================================
UCLASS()
class SNOWPIERCEREE_API USEEQuestManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Lifecycle ---

	/** Loads /Game/DataTables/DT_Quests into the runtime quest map (idempotent). */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void LoadQuestsFromTable();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void RegisterQuest(const FSEEQuest& Quest);

	/** Accept an Available quest whose prerequisites are met. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(FName QuestID);

	/** Legacy alias for AcceptQuest. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool StartQuest(FName QuestID);

	/** Drop an active side quest back to Available and reset its progress. Main quests cannot be abandoned. */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AbandonQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateObjective(FName QuestID, FName ObjectiveID, int32 Count = 1);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest(FName QuestID);

	// --- Progress signals ---

	/**
	 * Generic world signal: completes the active Interact/Escort/Custom (and
	 * non-car GoTo) objective whose TargetID equals the given id. Safe to call
	 * from any interactable; unknown ids are ignored.
	 */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyInteract(FName TargetID);

	/** Kill-objective progress for a dead NPC pawn (called by the NPC AI death path). */
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void NotifyNPCKilled(AActor* DeadNPC);

	/** Null-safe static hook for AI code: routes to NotifyNPCKilled on the actor's game instance. */
	static void ReportNPCDeath(AActor* DeadNPC);

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Quest")
	ESEEQuestState GetQuestState(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	FSEEQuest GetQuest(FName QuestID) const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetAllQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetActiveQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetCompletedQuests() const;

	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FSEEQuest> GetFailedQuests() const;

	/** Index of the objective the player should do next (first incomplete non-optional), or INDEX_NONE. */
	UFUNCTION(BlueprintPure, Category = "Quest")
	static int32 GetActiveObjectiveIndex(const FSEEQuest& Quest);

	UFUNCTION(BlueprintPure, Category = "Quest")
	FName GetTrackedQuestID() const { return TrackedQuestID; }

	UFUNCTION(BlueprintCallable, Category = "Quest")
	void SetTrackedQuest(FName QuestID) { TrackedQuestID = QuestID; }

	// --- Save mirroring (SEESaveGameSubsystem does not yet persist quests;
	//     these snapshot/restore hooks are ready for it) ---

	UFUNCTION(BlueprintPure, Category = "Quest|Save")
	TArray<FSEEQuestSaveState> CaptureQuestSaveState() const;

	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void RestoreQuestSaveState(const TArray<FSEEQuestSaveState>& SavedStates);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStarted OnQuestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestFailed OnQuestFailed;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStateChanged OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnObjectiveUpdated OnObjectiveUpdated;

	/** Native catch-all for Slate widgets (fires on any quest/objective change). */
	FSimpleMulticastDelegate OnQuestJournalChanged;

private:
	// --- Signal handlers (dynamic delegate targets must be UFUNCTIONs) ---

	UFUNCTION()
	void HandleDialogueNodeChanged(const FSEEDialogueNode& CurrentNode);

	UFUNCTION()
	void HandleDialogueEnded();

	UFUNCTION()
	void HandleItemAdded(FName ItemID, int32 Quantity);

	UFUNCTION()
	void HandleCollectibleRegistered(FName CollectibleID);

	// --- Engine internals ---

	bool HandleTicker(float DeltaTime);
	void EnsureQuestsLoaded();
	void BindPlayerInventoryIfNeeded();
	void PollGoToObjectives();
	void HandleDialogueFlagSet(FName FlagName);
	void TryAutoAcceptQuests();
	void CatchUpActiveObjectives();

	/** Exact-TargetID progress across active quests for the given objective types. Returns true if anything advanced. */
	bool ProgressByTarget(std::initializer_list<ESEEObjectiveType> Types, FName TargetID, int32 Count);

	/** Apply count to one objective; fires delegates; returns true when it completed. */
	bool ApplyProgress(FSEEQuest& Quest, int32 ObjectiveIndex, int32 Count);

	/** Retro-complete the newly active objective from flags / inventory / position, cascading; then check quest completion. */
	void AdvanceQuest(FName QuestID);

	bool CatchUpObjective(FSEEQuest& Quest, int32 ObjectiveIndex);
	bool IsObjectiveEligible(const FSEEQuest& Quest, int32 ObjectiveIndex) const;
	bool ArePrerequisitesMet(const FSEEQuest& Quest) const;
	void CheckQuestCompletion(FName QuestID);
	void GrantRewards(const FSEEQuest& Quest);
	void BroadcastStateChange(const FSEEQuest& Quest);

	static bool ParseCarIndexFromTarget(FName TargetID, int32& OutCarIndex);
	static bool MatchesNPCTarget(FName TargetID, const AActor* DeadNPC);

	APawn* GetPlayerPawn() const;
	USEEInventoryComponent* GetPlayerInventory() const;
	USEEDialogueManager* GetDialogueManager() const;

	// --- Runtime state ---

	TMap<FName, FSEEQuest> Quests;
	FName TrackedQuestID;

	/** Every dialogue node id the player's conversations have passed through. */
	TSet<FName> VisitedDialogueNodes;

	TWeakObjectPtr<USEEInventoryComponent> BoundPlayerInventory;
	FTSTicker::FDelegateHandle TickerHandle;

	int32 LastPolledCarIndex = INDEX_NONE;
	bool bQuestsLoaded = false;
	bool bTableLoadLogged = false;
};
