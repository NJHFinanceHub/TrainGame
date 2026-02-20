// SEEQuestLogWidget.cpp
#include "SEEQuestLogWidget.h"
#include "SEEQuestManager.h"
#include "Components/TextBlock.h"

void USEEQuestLogWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshQuests();
}

USEEQuestManager* USEEQuestLogWidget::GetQuestManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USEEQuestManager>() : nullptr;
}

void USEEQuestLogWidget::RefreshQuests()
{
	TArray<FSEEQuest> Quests = GetFilteredQuests();

	if (FilterText)
	{
		switch (CurrentFilter)
		{
		case ESEEQuestFilter::Active: FilterText->SetText(NSLOCTEXT("Quest", "Active", "Active Quests")); break;
		case ESEEQuestFilter::Completed: FilterText->SetText(NSLOCTEXT("Quest", "Completed", "Completed")); break;
		case ESEEQuestFilter::Failed: FilterText->SetText(NSLOCTEXT("Quest", "Failed", "Failed")); break;
		case ESEEQuestFilter::All: FilterText->SetText(NSLOCTEXT("Quest", "All", "All Quests")); break;
		}
	}

	OnQuestsRefreshed(Quests);

	if (!SelectedQuestID.IsNone())
	{
		UpdateQuestDetails();
	}
}

void USEEQuestLogWidget::SetFilter(ESEEQuestFilter NewFilter)
{
	CurrentFilter = NewFilter;
	RefreshQuests();
}

TArray<FSEEQuest> USEEQuestLogWidget::GetFilteredQuests() const
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM) return {};

	TArray<FSEEQuest> Result;

	if (CurrentFilter == ESEEQuestFilter::Active || CurrentFilter == ESEEQuestFilter::All)
	{
		Result.Append(QM->GetActiveQuests());
	}
	if (CurrentFilter == ESEEQuestFilter::Completed || CurrentFilter == ESEEQuestFilter::All)
	{
		Result.Append(QM->GetCompletedQuests());
	}

	// Sort: main quests first, then by name
	Result.Sort([](const FSEEQuest& A, const FSEEQuest& B)
	{
		if (A.bIsMainQuest != B.bIsMainQuest) return A.bIsMainQuest;
		return A.QuestName.CompareTo(B.QuestName) < 0;
	});

	return Result;
}

void USEEQuestLogWidget::SelectQuest(FName QuestID)
{
	SelectedQuestID = QuestID;
	UpdateQuestDetails();
}

void USEEQuestLogWidget::UpdateQuestDetails()
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM || SelectedQuestID.IsNone()) return;

	FSEEQuest Quest = QM->GetQuest(SelectedQuestID);

	if (QuestTitleText)
	{
		FString Title = Quest.QuestName.ToString();
		if (Quest.bIsMainQuest) Title = TEXT("[MAIN] ") + Title;
		QuestTitleText->SetText(FText::FromString(Title));
	}

	if (QuestDescText)
	{
		QuestDescText->SetText(Quest.Description);
	}

	if (ObjectivesText)
	{
		FString ObjStr;
		for (const FSEEQuestObjective& Obj : Quest.Objectives)
		{
			FString Prefix = Obj.bCompleted ? TEXT("[X] ") : TEXT("[ ] ");
			if (Obj.bOptional) Prefix += TEXT("(Optional) ");

			ObjStr += Prefix + Obj.Description.ToString();

			if (Obj.RequiredCount > 1)
			{
				ObjStr += FString::Printf(TEXT(" (%d/%d)"), Obj.CurrentCount, Obj.RequiredCount);
			}
			ObjStr += TEXT("\n");
		}
		ObjectivesText->SetText(FText::FromString(ObjStr));
	}

	if (RewardsText)
	{
		FString Rewards;
		if (Quest.XPReward > 0)
		{
			Rewards += FString::Printf(TEXT("XP: %d\n"), Quest.XPReward);
		}
		for (const FName& ItemID : Quest.ItemRewards)
		{
			Rewards += FString::Printf(TEXT("Item: %s\n"), *ItemID.ToString());
		}
		RewardsText->SetText(FText::FromString(Rewards));
	}

	if (TrackingText)
	{
		bool bIsTracked = IsTracked(SelectedQuestID);
		TrackingText->SetText(bIsTracked
			? NSLOCTEXT("Quest", "Tracked", "[TRACKING]")
			: NSLOCTEXT("Quest", "NotTracked", "Press T to Track"));
	}

	OnQuestSelected(Quest);
}

void USEEQuestLogWidget::ToggleTracking(FName QuestID)
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM) return;

	if (QM->GetTrackedQuestID() == QuestID)
	{
		QM->SetTrackedQuest(NAME_None);
	}
	else
	{
		QM->SetTrackedQuest(QuestID);
	}

	UpdateQuestDetails();
}

bool USEEQuestLogWidget::IsTracked(FName QuestID) const
{
	USEEQuestManager* QM = GetQuestManager();
	return QM && QM->GetTrackedQuestID() == QuestID;
}
