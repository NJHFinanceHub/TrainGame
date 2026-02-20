#include "SEEQuestManager.h"

void USEEQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void USEEQuestManager::RegisterQuest(const FSEEQuest& Quest)
{
	Quests.Add(Quest.QuestID, Quest);
}

bool USEEQuestManager::StartQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest) return false;
	if (Quest->State != ESEEQuestState::Available) return false;
	if (!ArePrerequisitesMet(*Quest)) return false;

	Quest->State = ESEEQuestState::Active;
	OnQuestStarted.Broadcast(QuestID);
	return true;
}

void USEEQuestManager::UpdateObjective(FName QuestID, FName ObjectiveID, int32 Count)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	for (FSEEQuestObjective& Obj : Quest->Objectives)
	{
		if (Obj.ObjectiveID == ObjectiveID && !Obj.bCompleted)
		{
			Obj.CurrentCount = FMath::Min(Obj.CurrentCount + Count, Obj.RequiredCount);
			if (Obj.CurrentCount >= Obj.RequiredCount)
			{
				Obj.bCompleted = true;
			}
			OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID);
			break;
		}
	}

	CheckQuestCompletion(QuestID);
}

void USEEQuestManager::CompleteQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	Quest->State = ESEEQuestState::Completed;
	OnQuestCompleted.Broadcast(QuestID);
}

void USEEQuestManager::FailQuest(FName QuestID)
{
	FSEEQuest* Quest = Quests.Find(QuestID);
	if (!Quest || Quest->State != ESEEQuestState::Active) return;

	Quest->State = ESEEQuestState::Failed;
	OnQuestFailed.Broadcast(QuestID);
}

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

bool USEEQuestManager::ArePrerequisitesMet(const FSEEQuest& Quest) const
{
	for (const FName& PrereqID : Quest.PrerequisiteQuests)
	{
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

	bool bAllRequired = true;
	for (const FSEEQuestObjective& Obj : Quest->Objectives)
	{
		if (!Obj.bOptional && !Obj.bCompleted)
		{
			bAllRequired = false;
			break;
		}
	}

	if (bAllRequired)
	{
		CompleteQuest(QuestID);
	}
}
