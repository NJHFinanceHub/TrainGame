// SEECodexWidget.cpp
#include "SEECodexWidget.h"
#include "Exploration/CollectibleJournalSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void USEECodexWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshCodex();
}

UCollectibleJournalSubsystem* USEECodexWidget::GetJournal() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UCollectibleJournalSubsystem>() : nullptr;
}

void USEECodexWidget::RefreshCodex()
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	if (!Journal) return;

	TArray<FCollectibleState> Entries = GetCurrentTabEntries();

	if (TabNameText)
	{
		TabNameText->SetText(GetTabDisplayName(ActiveTab));
	}

	float Completion = GetTabCompletion(ActiveTab);

	if (TabCompletionBar)
	{
		TabCompletionBar->SetPercent(Completion);
	}

	if (CompletionText)
	{
		CompletionText->SetText(FText::FromString(
			FString::Printf(TEXT("%.0f%%"), Completion * 100.0f)));
	}

	float Overall = GetOverallCompletion();
	if (OverallCompletionText)
	{
		OverallCompletionText->SetText(FText::FromString(
			FString::Printf(TEXT("Overall: %.0f%%"), Overall * 100.0f)));
	}

	if (TitleText)
	{
		TitleText->SetText(GetExplorationTitle());
	}

	int32 Unviewed = GetUnviewedCount();
	if (UnviewedBadgeText)
	{
		if (Unviewed > 0)
		{
			UnviewedBadgeText->SetText(FText::FromString(
				FString::Printf(TEXT("%d NEW"), Unviewed)));
			UnviewedBadgeText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			UnviewedBadgeText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	OnCodexRefreshed(ActiveTab, Entries);
}

void USEECodexWidget::SetActiveTab(ESEECodexTab Tab)
{
	ActiveTab = Tab;
	SelectedEntryID = NAME_None;
	RefreshCodex();
}

void USEECodexWidget::SelectEntry(FName EntryID)
{
	SelectedEntryID = EntryID;
	MarkEntryViewed(EntryID);
	OnEntrySelected(EntryID);
}

void USEECodexWidget::MarkEntryViewed(FName EntryID)
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	if (Journal)
	{
		Journal->MarkViewed(EntryID);
	}
}

TArray<FCollectibleState> USEECodexWidget::GetCurrentTabEntries() const
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	if (!Journal) return {};

	if (ActiveTab == ESEECodexTab::CodexEntries)
	{
		// Return codex entries as collectible states
		TArray<FCollectibleState> Result;
		TArray<FName> CodexIDs = Journal->GetCodexEntries();
		for (const FName& ID : CodexIDs)
		{
			FCollectibleState State;
			State.CollectibleID = ID;
			State.bCollected = true;
			State.bViewed = Journal->IsCodexEntryComplete(ID);
			Result.Add(State);
		}
		return Result;
	}

	return Journal->GetCollectedByType(TabToCollectibleType(ActiveTab));
}

float USEECodexWidget::GetTabCompletion(ESEECodexTab Tab) const
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	if (!Journal) return 0.0f;

	if (Tab == ESEECodexTab::CodexEntries) return 0.0f;

	return Journal->GetCompletionPercentage(TabToCollectibleType(Tab));
}

float USEECodexWidget::GetOverallCompletion() const
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	return Journal ? Journal->GetOverallCompletion() : 0.0f;
}

int32 USEECodexWidget::GetUnviewedCount() const
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	return Journal ? Journal->GetUnviewedCount() : 0;
}

FText USEECodexWidget::GetExplorationTitle() const
{
	UCollectibleJournalSubsystem* Journal = GetJournal();
	return Journal ? Journal->GetCurrentExplorationTitle() : NSLOCTEXT("Codex", "NoTitle", "Explorer");
}

ECollectibleType USEECodexWidget::TabToCollectibleType(ESEECodexTab Tab) const
{
	switch (Tab)
	{
	case ESEECodexTab::TrainLogs: return ECollectibleType::TrainLog;
	case ESEECodexTab::AudioRecordings: return ECollectibleType::AudioRecording;
	case ESEECodexTab::Artifacts: return ECollectibleType::Artifact;
	case ESEECodexTab::Blueprints: return ECollectibleType::Blueprint;
	case ESEECodexTab::FactionIntel: return ECollectibleType::FactionIntel;
	case ESEECodexTab::ManifestPages: return ECollectibleType::ManifestPage;
	default: return ECollectibleType::TrainLog;
	}
}

FText USEECodexWidget::GetTabDisplayName(ESEECodexTab Tab) const
{
	switch (Tab)
	{
	case ESEECodexTab::TrainLogs: return NSLOCTEXT("Codex", "Logs", "Train Logs");
	case ESEECodexTab::AudioRecordings: return NSLOCTEXT("Codex", "Audio", "Audio Recordings");
	case ESEECodexTab::Artifacts: return NSLOCTEXT("Codex", "Artifacts", "Artifacts");
	case ESEECodexTab::Blueprints: return NSLOCTEXT("Codex", "BP", "Blueprints");
	case ESEECodexTab::FactionIntel: return NSLOCTEXT("Codex", "Intel", "Faction Intel");
	case ESEECodexTab::ManifestPages: return NSLOCTEXT("Codex", "Manifest", "Manifest Pages");
	case ESEECodexTab::CodexEntries: return NSLOCTEXT("Codex", "Codex", "Codex");
	default: return FText::GetEmpty();
	}
}
