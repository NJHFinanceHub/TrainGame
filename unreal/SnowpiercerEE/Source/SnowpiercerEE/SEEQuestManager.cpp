#include "SEEQuestManager.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Exploration/CollectibleJournalSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "SEEFactionManager.h"
#include "SEEInventoryComponent.h"
#include "SEEStatsComponent.h"
#include "SEETypes.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogSEEQuest, Log, All);

namespace
{
	constexpr float GQuestTickInterval = 1.0f;

	// Train layout: cars run along +X, 13000 units long, tail car centred near
	// X = 6500 (car = floor((X + 6500) / 13000)).
	constexpr float GCarLength = 13000.0f;
	constexpr float GCarOriginOffset = 6500.0f;

	const TCHAR* GQuestTablePath = TEXT("/Game/DataTables/DT_Quests.DT_Quests");
}

// ---------------------------------------------------------------------------
// Subsystem lifecycle
// ---------------------------------------------------------------------------

void USEEQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Dialogue and collectible systems live on the same GameInstance — make
	// sure they exist before we bind (duplicate binds guarded below).
	USEEDialogueManager* Dialogue =
		Cast<USEEDialogueManager>(Collection.InitializeDependency(USEEDialogueManager::StaticClass()));
	if (Dialogue)
	{
		if (!Dialogue->OnDialogueNodeChanged.IsAlreadyBound(this, &USEEQuestManager::HandleDialogueNodeChanged))
		{
			Dialogue->OnDialogueNodeChanged.AddDynamic(this, &USEEQuestManager::HandleDialogueNodeChanged);
		}
		if (!Dialogue->OnDialogueEnded.IsAlreadyBound(this, &USEEQuestManager::HandleDialogueEnded))
		{
			Dialogue->OnDialogueEnded.AddDynamic(this, &USEEQuestManager::HandleDialogueEnded);
		}
	}

	UCollectibleJournalSubsystem* Journal = Cast<UCollectibleJournalSubsystem>(
		Collection.InitializeDependency(UCollectibleJournalSubsystem::StaticClass()));
	if (Journal &&
		!Journal->OnCollectibleRegistered.IsAlreadyBound(this, &USEEQuestManager::HandleCollectibleRegistered))
	{
		Journal->OnCollectibleRegistered.AddDynamic(this, &USEEQuestManager::HandleCollectibleRegistered);
	}

	// 1s heartbeat: lazy table load, player-inventory (re)binding once a pawn
	// exists, GoTo car polling, flag catch-up and main-quest auto-accept.
	if (!TickerHandle.IsValid())
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &USEEQuestManager::HandleTicker), GQuestTickInterval);
	}
}

void USEEQuestManager::Deinitialize()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

	if (USEEInventoryComponent* Inventory = BoundPlayerInventory.Get())
	{
		Inventory->OnItemAdded.RemoveDynamic(this, &USEEQuestManager::HandleItemAdded);
	}
	BoundPlayerInventory.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEDialogueManager* Dialogue = GI->GetSubsystem<USEEDialogueManager>())
		{
			Dialogue->OnDialogueNodeChanged.RemoveDynamic(this, &USEEQuestManager::HandleDialogueNodeChanged);
			Dialogue->OnDialogueEnded.RemoveDynamic(this, &USEEQuestManager::HandleDialogueEnded);
		}
		if (UCollectibleJournalSubsystem* Journal = GI->GetSubsystem<UCollectibleJournalSubsystem>())
		{
			Journal->OnCollectibleRegistered.RemoveDynamic(this, &USEEQuestManager::HandleCollectibleRegistered);
		}
	}

	Super::Deinitialize();
}

bool USEEQuestManager::HandleTicker(float /*DeltaTime*/)
{
	EnsureQuestsLoaded();
	BindPlayerInventoryIfNeeded();
	TryAutoAcceptQuests();
	CatchUpActiveObjectives();
	PollGoToObjectives();
	return true; // keep ticking
}

// ---------------------------------------------------------------------------
// Quest table loading
// ---------------------------------------------------------------------------

void USEEQuestManager::LoadQuestsFromTable()
{
	bQuestsLoaded = true; // attempt once per call path; rows merge idempotently

	UDataTable* Table = LoadObject<UDataTable>(nullptr, GQuestTablePath);
	if (!Table)
	{
		if (!bTableLoadLogged)
		{
			bTableLoadLogged = true;
			UE_LOG(LogSEEQuest, Warning,
				TEXT("Quest DataTable not found at %s — journal will be empty until quests are registered."),
				GQuestTablePath);
		}
		bQuestsLoaded = false; // retry on the next heartbeat (table may import later)
		return;
	}

	int32 NumAdded = 0;
	Table->ForeachRow<FSEEQuest>(TEXT("LoadQuestsFromTable"),
		[this, &NumAdded](const FName& RowName, const FSEEQuest& Row)
		{
			FSEEQuest Quest = Row;
			if (Quest.QuestID.IsNone())
			{
				Quest.QuestID = RowName;
			}

			// Never clobber live progress on a reload.
			if (!Quests.Contains(Quest.QuestID))
			{
				Quests.Add(Quest.QuestID, Quest);
				++NumAdded;
			}
		});

	if (NumAdded > 0)
	{
		UE_LOG(LogSEEQuest, Log, TEXT("Loaded %d quests from %s."), NumAdded, GQuestTablePath);
		TryAutoAcceptQuests();
		OnQuestJournalChanged.Broadcast();
	}
}

void USEEQuestManager::EnsureQuestsLoaded()
{
	if (!bQuestsLoaded)
	{
		LoadQuestsFromTable();
	}
}

void USEEQuestManager::RegisterQuest(const FSEEQuest& Quest)
{
	if (Quest.QuestID.IsNone()) return;
	Quests.Add(Quest.QuestID, Quest);
	TryAutoAcceptQuests();
	OnQuestJournalChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// Lifecycle API
// ---------------------------------------------------------------------------

bool USEEQuestManager::AcceptQuest(FName QuestID)
{
	EnsureQuestsLoaded();

	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest) return false;
	if (Quest->State != ESEEQuestState::Available) return false;
	if (!ArePrerequisitesMet(*Quest)) return false;

	Quest->State = ESEEQuestState::Active;
	OnQuestStarted.Broadcast(QuestID);
	BroadcastStateChange(*Quest);

	// Catch up: the player may already hold the items / have set the flags /
	// be standing in the right car before accepting.
	AdvanceQuest(QuestID);
	return true;
}

bool USEEQuestManager::StartQuest(FName QuestID)
{
	return AcceptQuest(QuestID);
}

bool USEEQuestManager::AbandonQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return false;
	if (Quest->bIsMainQuest) return false; // the main arc cannot be dropped

	for (FSEEQuestObjective& Obj : Quest->Objectives)
	{
		Obj.CurrentCount = 0;
		Obj.bCompleted = false;
	}
	Quest->State = ESEEQuestState::Available;

	if (TrackedQuestID == QuestID)
	{
		TrackedQuestID = NAME_None;
	}

	BroadcastStateChange(*Quest);
	return true;
}

void USEEQuestManager::UpdateObjective(FName QuestID, FName ObjectiveID, int32 Count)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	for (int32 i = 0; i < Quest->Objectives.Num(); ++i)
	{
		if (Quest->Objectives[i].ObjectiveID == ObjectiveID)
		{
			if (IsObjectiveEligible(*Quest, i))
			{
				ApplyProgress(*Quest, i, Count);
				AdvanceQuest(QuestID);
			}
			return;
		}
	}
}

void USEEQuestManager::CompleteQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	Quest->State = ESEEQuestState::Completed;

	if (TrackedQuestID == QuestID)
	{
		TrackedQuestID = NAME_None;
	}

	OnQuestCompleted.Broadcast(QuestID);
	BroadcastStateChange(*Quest);

	// Rewards from a copy: granting items re-enters the progress engine and
	// may mutate other quests in the map.
	const FSEEQuest QuestCopy = *Quest;
	GrantRewards(QuestCopy);

	// Completing a prerequisite can unlock the next main-arc quest.
	TryAutoAcceptQuests();
}

void USEEQuestManager::FailQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	Quest->State = ESEEQuestState::Failed;
	if (TrackedQuestID == QuestID)
	{
		TrackedQuestID = NAME_None;
	}

	OnQuestFailed.Broadcast(QuestID);
	BroadcastStateChange(*Quest);
}

void USEEQuestManager::TryAutoAcceptQuests()
{
	// Main quests flow automatically once their prerequisites complete.
	// (Iterate over a snapshot of ids: AcceptQuest mutates quest values.)
	TArray<FName> Candidates;
	for (const auto& Pair : Quests)
	{
		if (Pair.Value.bIsMainQuest && Pair.Value.State == ESEEQuestState::Available &&
			ArePrerequisitesMet(Pair.Value))
		{
			Candidates.Add(Pair.Key);
		}
	}

	for (const FName& QuestID : Candidates)
	{
		if (AcceptQuest(QuestID))
		{
			UE_LOG(LogSEEQuest, Log, TEXT("Auto-accepted main quest %s."), *QuestID.ToString());
		}
	}
}

// ---------------------------------------------------------------------------
// Progress engine — signal handlers
// ---------------------------------------------------------------------------

void USEEQuestManager::HandleDialogueNodeChanged(const FSEEDialogueNode& CurrentNode)
{
	if (!CurrentNode.NodeID.IsNone())
	{
		VisitedDialogueNodes.Add(CurrentNode.NodeID);

		// Dialogue objectives keyed on a visited node id (entry nodes: "Pike_01").
		ProgressByTarget({ ESEEObjectiveType::Dialogue }, CurrentNode.NodeID, 1);
	}

	// SetFlag nodes broadcast before the manager stores the flag — treat the
	// node itself as the flag-set signal.
	if (CurrentNode.NodeType == ESEEDialogueNodeType::SetFlag &&
		!CurrentNode.FlagToSet.IsNone() && CurrentNode.FlagValue)
	{
		HandleDialogueFlagSet(CurrentNode.FlagToSet);
	}
}

void USEEQuestManager::HandleDialogueFlagSet(FName FlagName)
{
	// A true flag completes Dialogue objectives keyed on the flag name
	// ("Dealer_Persuade") and acts as the fallback for Interact / Escort /
	// Custom objectives so story beats can't dead-end.
	ProgressByTarget({ ESEEObjectiveType::Dialogue, ESEEObjectiveType::Interact,
		ESEEObjectiveType::Escort, ESEEObjectiveType::Custom, ESEEObjectiveType::GoTo },
		FlagName, 1);

	// Side-quest acceptance convention: "Quest_<Fragment>_Started" flags from
	// dialogue accept the Available quest whose QuestID contains <Fragment>
	// (e.g. "Quest_Workshop_Started" -> "Quest_TheWorkshop").
	FString FlagStr = FlagName.ToString();
	if (FlagStr.StartsWith(TEXT("Quest_")) && FlagStr.EndsWith(TEXT("_Started")))
	{
		const FString Fragment = FlagStr.Mid(6, FlagStr.Len() - 6 - 8);
		if (!Fragment.IsEmpty())
		{
			TArray<FName> ToAccept;
			for (const auto& Pair : Quests)
			{
				if (Pair.Value.State == ESEEQuestState::Available &&
					Pair.Key.ToString().Contains(Fragment, ESearchCase::IgnoreCase))
				{
					ToAccept.Add(Pair.Key);
				}
			}
			for (const FName& QuestID : ToAccept)
			{
				AcceptQuest(QuestID);
			}
		}
	}
}

void USEEQuestManager::HandleDialogueEnded()
{
	// Safety net: flags set mid-conversation may unlock or satisfy objectives
	// that only became active as the conversation closed.
	TryAutoAcceptQuests();
	CatchUpActiveObjectives();
}

void USEEQuestManager::HandleItemAdded(FName ItemID, int32 Quantity)
{
	if (ItemID.IsNone() || Quantity <= 0) return;
	ProgressByTarget({ ESEEObjectiveType::Collect }, ItemID, Quantity);
}

void USEEQuestManager::HandleCollectibleRegistered(FName CollectibleID)
{
	if (CollectibleID.IsNone()) return;
	ProgressByTarget({ ESEEObjectiveType::Collect }, CollectibleID, 1);
}

void USEEQuestManager::NotifyInteract(FName TargetID)
{
	if (TargetID.IsNone()) return;
	ProgressByTarget({ ESEEObjectiveType::Interact, ESEEObjectiveType::Escort,
		ESEEObjectiveType::Custom, ESEEObjectiveType::GoTo }, TargetID, 1);
}

void USEEQuestManager::NotifyNPCKilled(AActor* DeadNPC)
{
	if (!DeadNPC) return;

	TArray<FName> Affected;
	for (auto& Pair : Quests)
	{
		FSEEQuest& Quest = Pair.Value;
		if (Quest.State != ESEEQuestState::Active) continue;

		for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
		{
			const FSEEQuestObjective& Obj = Quest.Objectives[i];
			if (Obj.Type != ESEEObjectiveType::Kill) continue;
			if (!IsObjectiveEligible(Quest, i)) continue;
			if (!MatchesNPCTarget(Obj.TargetID, DeadNPC)) continue;

			ApplyProgress(Quest, i, 1);
			Affected.AddUnique(Quest.QuestID);
		}
	}

	for (const FName& QuestID : Affected)
	{
		AdvanceQuest(QuestID);
	}
}

void USEEQuestManager::ReportNPCDeath(AActor* DeadNPC)
{
	if (!DeadNPC) return;
	const UWorld* World = DeadNPC->GetWorld();
	UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (USEEQuestManager* QuestManager = GI ? GI->GetSubsystem<USEEQuestManager>() : nullptr)
	{
		QuestManager->NotifyNPCKilled(DeadNPC);
	}
}

// ---------------------------------------------------------------------------
// Progress engine — core
// ---------------------------------------------------------------------------

bool USEEQuestManager::ProgressByTarget(std::initializer_list<ESEEObjectiveType> Types,
	FName TargetID, int32 Count)
{
	if (TargetID.IsNone() || Count <= 0) return false;

	bool bAnyProgress = false;
	TArray<FName> Affected;

	for (auto& Pair : Quests)
	{
		FSEEQuest& Quest = Pair.Value;
		if (Quest.State != ESEEQuestState::Active) continue;

		for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
		{
			const FSEEQuestObjective& Obj = Quest.Objectives[i];

			bool bTypeMatches = false;
			for (ESEEObjectiveType Type : Types)
			{
				if (Obj.Type == Type) { bTypeMatches = true; break; }
			}
			if (!bTypeMatches) continue;
			if (Obj.TargetID != TargetID) continue;
			if (!IsObjectiveEligible(Quest, i)) continue;

			ApplyProgress(Quest, i, Count);
			bAnyProgress = true;
			Affected.AddUnique(Quest.QuestID);
		}
	}

	for (const FName& QuestID : Affected)
	{
		AdvanceQuest(QuestID);
	}

	return bAnyProgress;
}

bool USEEQuestManager::ApplyProgress(FSEEQuest& Quest, int32 ObjectiveIndex, int32 Count)
{
	if (!Quest.Objectives.IsValidIndex(ObjectiveIndex)) return false;

	FSEEQuestObjective& Obj = Quest.Objectives[ObjectiveIndex];
	if (Obj.bCompleted) return true;

	Obj.CurrentCount = FMath::Clamp(Obj.CurrentCount + Count, 0, FMath::Max(1, Obj.RequiredCount));
	if (Obj.CurrentCount >= Obj.RequiredCount)
	{
		Obj.bCompleted = true;
	}

	OnObjectiveUpdated.Broadcast(Quest.QuestID, Obj.ObjectiveID);
	OnQuestJournalChanged.Broadcast();
	return Obj.bCompleted;
}

void USEEQuestManager::AdvanceQuest(FName QuestID)
{
	// Cascade: each newly active objective may already be satisfied by held
	// items, set flags, visited nodes or the player's current car. Bounded by
	// the objective count to be safe.
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest) return;

	int32 Guard = Quest->Objectives.Num() + 1;
	while (Quest && Quest->State == ESEEQuestState::Active && Guard-- > 0)
	{
		const int32 ActiveIndex = GetActiveObjectiveIndex(*Quest);
		if (ActiveIndex == INDEX_NONE || !CatchUpObjective(*Quest, ActiveIndex))
		{
			break;
		}
		Quest = Quests.Find(QuestID); // re-find in case rewards re-entered
	}

	CheckQuestCompletion(QuestID);
}

bool USEEQuestManager::CatchUpObjective(FSEEQuest& Quest, int32 ObjectiveIndex)
{
	if (!Quest.Objectives.IsValidIndex(ObjectiveIndex)) return false;
	FSEEQuestObjective& Obj = Quest.Objectives[ObjectiveIndex];
	if (Obj.bCompleted) return false;

	switch (Obj.Type)
	{
	case ESEEObjectiveType::Dialogue:
	case ESEEObjectiveType::Interact:
	case ESEEObjectiveType::Escort:
	case ESEEObjectiveType::Custom:
	{
		const USEEDialogueManager* Dialogue = GetDialogueManager();
		const bool bFlagSet = Dialogue && Dialogue->GetFlag(Obj.TargetID);
		const bool bNodeVisited = Obj.Type == ESEEObjectiveType::Dialogue &&
			VisitedDialogueNodes.Contains(Obj.TargetID);
		if (bFlagSet || bNodeVisited)
		{
			return ApplyProgress(Quest, ObjectiveIndex, Obj.RequiredCount);
		}
		break;
	}

	case ESEEObjectiveType::Collect:
	{
		int32 Have = 0;
		if (const USEEInventoryComponent* Inventory = GetPlayerInventory())
		{
			Have = Inventory->GetItemCount(Obj.TargetID);
		}
		if (Have <= 0)
		{
			if (const UGameInstance* GI = GetGameInstance())
			{
				const UCollectibleJournalSubsystem* Journal = GI->GetSubsystem<UCollectibleJournalSubsystem>();
				if (Journal && Journal->IsCollected(Obj.TargetID))
				{
					Have = 1;
				}
			}
		}
		if (Have > Obj.CurrentCount)
		{
			return ApplyProgress(Quest, ObjectiveIndex, Have - Obj.CurrentCount);
		}
		break;
	}

	case ESEEObjectiveType::GoTo:
	{
		int32 TargetCar = INDEX_NONE;
		if (ParseCarIndexFromTarget(Obj.TargetID, TargetCar) && LastPolledCarIndex == TargetCar)
		{
			return ApplyProgress(Quest, ObjectiveIndex, Obj.RequiredCount);
		}
		break;
	}

	case ESEEObjectiveType::Kill:
	default:
		break; // no retroactive signal
	}

	return false;
}

void USEEQuestManager::CatchUpActiveObjectives()
{
	TArray<FName> ActiveIDs;
	for (const auto& Pair : Quests)
	{
		if (Pair.Value.State == ESEEQuestState::Active)
		{
			ActiveIDs.Add(Pair.Key);
		}
	}
	for (const FName& QuestID : ActiveIDs)
	{
		AdvanceQuest(QuestID);
	}
}

void USEEQuestManager::PollGoToObjectives()
{
	const APawn* Player = GetPlayerPawn();
	if (!Player) return;

	const int32 CurrentCar = FMath::FloorToInt32(
		(Player->GetActorLocation().X + GCarOriginOffset) / GCarLength);
	const int32 PreviousCar = LastPolledCarIndex;
	LastPolledCarIndex = CurrentCar;

	TArray<FName> Affected;
	for (auto& Pair : Quests)
	{
		FSEEQuest& Quest = Pair.Value;
		if (Quest.State != ESEEQuestState::Active) continue;

		for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
		{
			const FSEEQuestObjective& Obj = Quest.Objectives[i];
			if (Obj.Type != ESEEObjectiveType::GoTo || Obj.bCompleted) continue;
			if (!IsObjectiveEligible(Quest, i)) continue;

			int32 TargetCar = INDEX_NONE;
			if (!ParseCarIndexFromTarget(Obj.TargetID, TargetCar)) continue; // non-car GoTo: NotifyInteract/flag path

			// At the car, or crossed it since the last poll (covers fast
			// traversal in either direction between 1s samples).
			const bool bAtCar = CurrentCar == TargetCar;
			const bool bCrossed = PreviousCar != INDEX_NONE &&
				((PreviousCar < TargetCar && CurrentCar >= TargetCar) ||
				 (PreviousCar > TargetCar && CurrentCar <= TargetCar));

			if (bAtCar || bCrossed)
			{
				ApplyProgress(Quest, i, Obj.RequiredCount);
				Affected.AddUnique(Quest.QuestID);
			}
		}
	}

	for (const FName& QuestID : Affected)
	{
		AdvanceQuest(QuestID);
	}
}

// ---------------------------------------------------------------------------
// Sequential gating + completion
// ---------------------------------------------------------------------------

int32 USEEQuestManager::GetActiveObjectiveIndex(const FSEEQuest& Quest)
{
	if (Quest.State != ESEEQuestState::Active) return INDEX_NONE;

	for (int32 i = 0; i < Quest.Objectives.Num(); ++i)
	{
		if (!Quest.Objectives[i].bOptional && !Quest.Objectives[i].bCompleted)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool USEEQuestManager::IsObjectiveEligible(const FSEEQuest& Quest, int32 ObjectiveIndex) const
{
	if (Quest.State != ESEEQuestState::Active) return false;
	if (!Quest.Objectives.IsValidIndex(ObjectiveIndex)) return false;

	const FSEEQuestObjective& Obj = Quest.Objectives[ObjectiveIndex];
	if (Obj.bCompleted) return false;
	if (Obj.bOptional) return true; // optional objectives are live for the whole quest

	// Sequential gating: only the first incomplete required objective counts.
	return ObjectiveIndex == GetActiveObjectiveIndex(Quest);
}

bool USEEQuestManager::ArePrerequisitesMet(const FSEEQuest& Quest) const
{
	for (const FName& PrereqID : Quest.PrerequisiteQuests)
	{
		if (PrereqID.IsNone()) continue;
		if (GetQuestState(PrereqID) != ESEEQuestState::Completed)
		{
			return false;
		}
	}
	return true;
}

void USEEQuestManager::CheckQuestCompletion(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	for (const FSEEQuestObjective& Obj : Quest->Objectives)
	{
		if (!Obj.bOptional && !Obj.bCompleted)
		{
			return;
		}
	}

	CompleteQuest(QuestID);
}

void USEEQuestManager::GrantRewards(const FSEEQuest& Quest)
{
	APawn* Player = GetPlayerPawn();

	// XP through the player's stats component.
	if (Quest.XPReward > 0 && Player)
	{
		if (USEEStatsComponent* Stats = Player->FindComponentByClass<USEEStatsComponent>())
		{
			Stats->AddXP(Quest.XPReward);
		}
	}

	// Item rewards: one AddItem per listed entry (duplicates list multiple times).
	if (Quest.ItemRewards.Num() > 0 && Player)
	{
		if (USEEInventoryComponent* Inventory = Player->FindComponentByClass<USEEInventoryComponent>())
		{
			for (const FName& ItemID : Quest.ItemRewards)
			{
				if (!ItemID.IsNone())
				{
					Inventory->AddItem(ItemID, 1);
				}
			}
		}
	}

	// Faction reputation: keys are ESEEFaction enum names ("Tailies", ...).
	if (Quest.FactionRepRewards.Num() > 0)
	{
		UGameInstance* GI = GetGameInstance();
		USEEFactionManager* Factions = GI ? GI->GetSubsystem<USEEFactionManager>() : nullptr;
		const UEnum* FactionEnum = StaticEnum<ESEEFaction>();
		if (Factions && FactionEnum)
		{
			for (const TPair<FName, int32>& Pair : Quest.FactionRepRewards)
			{
				const int64 Value = FactionEnum->GetValueByName(Pair.Key);
				if (Value == INDEX_NONE)
				{
					UE_LOG(LogSEEQuest, Warning,
						TEXT("Quest %s rewards unknown faction '%s' — skipped."),
						*Quest.QuestID.ToString(), *Pair.Key.ToString());
					continue;
				}
				Factions->ModifyReputation(static_cast<ESEEFaction>(Value), Pair.Value);
			}
		}
	}

	UE_LOG(LogSEEQuest, Log, TEXT("Quest %s completed — rewards granted (XP %d, %d items, %d faction changes)."),
		*Quest.QuestID.ToString(), Quest.XPReward, Quest.ItemRewards.Num(), Quest.FactionRepRewards.Num());
}

void USEEQuestManager::BroadcastStateChange(const FSEEQuest& Quest)
{
	OnQuestStateChanged.Broadcast(Quest.QuestID, Quest.State);
	OnQuestJournalChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

ESEEQuestState USEEQuestManager::GetQuestState(FName QuestID) const
{
	const FSEEQuest* Quest = Quests.Find(QuestID);
	return Quest ? Quest->State : ESEEQuestState::Available;
}

FSEEQuest USEEQuestManager::GetQuest(FName QuestID) const
{
	const FSEEQuest* Quest = Quests.Find(QuestID);
	return Quest ? *Quest : FSEEQuest();
}

TArray<FSEEQuest> USEEQuestManager::GetAllQuests() const
{
	TArray<FSEEQuest> Result;
	Quests.GenerateValueArray(Result);
	return Result;
}

TArray<FSEEQuest> USEEQuestManager::GetActiveQuests() const
{
	TArray<FSEEQuest> Result;
	for (const auto& Pair : Quests)
	{
		if (Pair.Value.State == ESEEQuestState::Active)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FSEEQuest> USEEQuestManager::GetCompletedQuests() const
{
	TArray<FSEEQuest> Result;
	for (const auto& Pair : Quests)
	{
		if (Pair.Value.State == ESEEQuestState::Completed)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FSEEQuest> USEEQuestManager::GetFailedQuests() const
{
	TArray<FSEEQuest> Result;
	for (const auto& Pair : Quests)
	{
		if (Pair.Value.State == ESEEQuestState::Failed)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ---------------------------------------------------------------------------
// Save mirroring
// ---------------------------------------------------------------------------

TArray<FSEEQuestSaveState> USEEQuestManager::CaptureQuestSaveState() const
{
	TArray<FSEEQuestSaveState> Result;
	Result.Reserve(Quests.Num());

	for (const auto& Pair : Quests)
	{
		const FSEEQuest& Quest = Pair.Value;
		FSEEQuestSaveState& Entry = Result.AddDefaulted_GetRef();
		Entry.QuestID = Quest.QuestID;
		Entry.State = Quest.State;
		Entry.ObjectiveCounts.Reserve(Quest.Objectives.Num());
		Entry.ObjectiveCompleted.Reserve(Quest.Objectives.Num());
		for (const FSEEQuestObjective& Obj : Quest.Objectives)
		{
			Entry.ObjectiveCounts.Add(Obj.CurrentCount);
			Entry.ObjectiveCompleted.Add(Obj.bCompleted);
		}
	}
	return Result;
}

void USEEQuestManager::RestoreQuestSaveState(const TArray<FSEEQuestSaveState>& SavedStates)
{
	EnsureQuestsLoaded();

	for (const FSEEQuestSaveState& Entry : SavedStates)
	{
		FSEEQuest* Quest = Quests.Find(Entry.QuestID);
		if (!Quest) continue;

		Quest->State = Entry.State;
		for (int32 i = 0; i < Quest->Objectives.Num(); ++i)
		{
			if (Entry.ObjectiveCounts.IsValidIndex(i))
			{
				Quest->Objectives[i].CurrentCount = Entry.ObjectiveCounts[i];
			}
			if (Entry.ObjectiveCompleted.IsValidIndex(i))
			{
				Quest->Objectives[i].bCompleted = Entry.ObjectiveCompleted[i];
			}
		}
	}

	OnQuestJournalChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void USEEQuestManager::BindPlayerInventoryIfNeeded()
{
	USEEInventoryComponent* Inventory = GetPlayerInventory();
	if (Inventory == BoundPlayerInventory.Get()) return; // also true while no pawn exists yet

	if (USEEInventoryComponent* Old = BoundPlayerInventory.Get())
	{
		Old->OnItemAdded.RemoveDynamic(this, &USEEQuestManager::HandleItemAdded);
	}

	BoundPlayerInventory = Inventory;
	if (Inventory && !Inventory->OnItemAdded.IsAlreadyBound(this, &USEEQuestManager::HandleItemAdded))
	{
		Inventory->OnItemAdded.AddDynamic(this, &USEEQuestManager::HandleItemAdded);
	}
}

bool USEEQuestManager::ParseCarIndexFromTarget(FName TargetID, int32& OutCarIndex)
{
	const FString Str = TargetID.ToString();
	if (!Str.StartsWith(TEXT("Car"), ESearchCase::IgnoreCase)) return false;

	FString Digits;
	for (int32 i = 3; i < Str.Len() && FChar::IsDigit(Str[i]); ++i)
	{
		Digits.AppendChar(Str[i]);
	}
	if (Digits.IsEmpty()) return false;

	OutCarIndex = FCString::Atoi(*Digits);
	return true;
}

bool USEEQuestManager::MatchesNPCTarget(FName TargetID, const AActor* DeadNPC)
{
	if (TargetID.IsNone() || !DeadNPC) return false;

	FString Fragment = TargetID.ToString();
	Fragment.RemoveFromStart(TEXT("NPC_"));
	if (Fragment.IsEmpty()) return false;

	// "NPC_KronoleDealer" matches BP_NPC_KronoleDealer_C (class) or
	// BP_NPC_KronoleDealer_2 (instance), case-insensitive.
	return DeadNPC->GetClass()->GetName().Contains(Fragment, ESearchCase::IgnoreCase) ||
	       DeadNPC->GetName().Contains(Fragment, ESearchCase::IgnoreCase);
}

APawn* USEEQuestManager::GetPlayerPawn() const
{
	const UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	return World ? UGameplayStatics::GetPlayerPawn(World, 0) : nullptr;
}

USEEInventoryComponent* USEEQuestManager::GetPlayerInventory() const
{
	const APawn* Player = GetPlayerPawn();
	return Player ? Player->FindComponentByClass<USEEInventoryComponent>() : nullptr;
}

USEEDialogueManager* USEEQuestManager::GetDialogueManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USEEDialogueManager>() : nullptr;
}
