#include "CollectibleJournalSubsystem.h"

void UCollectibleJournalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UCollectibleJournalSubsystem::RegisterCollectible(FName CollectibleID)
{
    if (CollectibleID.IsNone() || CollectibleStates.Contains(CollectibleID))
    {
        return;
    }

    FCollectibleState State;
    State.CollectibleID = CollectibleID;
    State.bCollected = true;
    CollectibleStates.Add(CollectibleID, State);
    OnCollectibleRegistered.Broadcast(CollectibleID);
}

void UCollectibleJournalSubsystem::MarkViewed(FName CollectibleID)
{
    if (FCollectibleState* State = CollectibleStates.Find(CollectibleID))
    {
        State->bViewed = true;
    }
}

bool UCollectibleJournalSubsystem::IsCollected(FName CollectibleID) const
{
    return CollectibleStates.Contains(CollectibleID);
}

TArray<FCollectibleState> UCollectibleJournalSubsystem::GetCollectedByType(ECollectibleType Type) const
{
    TArray<FCollectibleState> Result;
    for (const auto& Pair : CollectibleStates)
    {
        if (Pair.Value.bCollected)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FCollectibleState> UCollectibleJournalSubsystem::GetCollectedByZone(ETrainZone Zone) const
{
    TArray<FCollectibleState> Result;
    for (const auto& Pair : CollectibleStates)
    {
        if (Pair.Value.bCollected)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

float UCollectibleJournalSubsystem::GetCompletionPercentage(ECollectibleType Type) const
{
    return 0.0f;
}

float UCollectibleJournalSubsystem::GetOverallCompletion() const
{
    return 0.0f;
}

int32 UCollectibleJournalSubsystem::GetUnviewedCount() const
{
    int32 Count = 0;
    for (const auto& Pair : CollectibleStates)
    {
        if (!Pair.Value.bViewed)
        {
            Count++;
        }
    }
    return Count;
}

TArray<FFactionIntelProgress> UCollectibleJournalSubsystem::GetAllFactionProgress() const
{
    TArray<FFactionIntelProgress> Result;
    FactionProgress.GenerateValueArray(Result);
    return Result;
}

FFactionIntelProgress UCollectibleJournalSubsystem::GetFactionProgress(FName FactionID) const
{
    if (const FFactionIntelProgress* Found = FactionProgress.Find(FactionID))
    {
        return *Found;
    }
    return FFactionIntelProgress();
}

bool UCollectibleJournalSubsystem::IsFactionDialogueUnlocked(FName FactionID) const
{
    if (const FFactionIntelProgress* Found = FactionProgress.Find(FactionID))
    {
        return Found->IntelCollected >= 5;
    }
    return false;
}

int32 UCollectibleJournalSubsystem::GetManifestPagesCollected() const
{
    return 0;
}

bool UCollectibleJournalSubsystem::IsManifestTruthUnlocked() const
{
    return GetManifestPagesCollected() >= 25;
}

TArray<FName> UCollectibleJournalSubsystem::GetManifestCrossReferences() const
{
    return TArray<FName>();
}

TArray<FName> UCollectibleJournalSubsystem::GetCodexEntries() const
{
    return TArray<FName>();
}

bool UCollectibleJournalSubsystem::IsCodexEntryComplete(FName EntryID) const
{
    return false;
}

bool UCollectibleJournalSubsystem::IsMilestoneReached(float Percentage) const
{
    return GetOverallCompletion() >= Percentage;
}

FText UCollectibleJournalSubsystem::GetCurrentExplorationTitle() const
{
    return FText::FromString(TEXT("Explorer"));
}

void UCollectibleJournalSubsystem::UpdateFactionProgress(const FCollectibleData& Data)
{
}

void UCollectibleJournalSubsystem::UpdateCodex(const FCollectibleData& Data)
{
}

void UCollectibleJournalSubsystem::CheckMilestones(ECollectibleType Type)
{
}

void UCollectibleJournalSubsystem::ProcessManifestPage(const FCollectibleData& Data)
{
}
