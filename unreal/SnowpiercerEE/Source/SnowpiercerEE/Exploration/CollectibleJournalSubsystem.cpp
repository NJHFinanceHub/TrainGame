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
    if (!CollectibleDataTable) return 0.0f;

    int32 Total = 0;
    int32 Collected = 0;
    TArray<FName> RowNames = CollectibleDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        const FCollectibleData* Data = CollectibleDataTable->FindRow<FCollectibleData>(RowName, TEXT(""));
        if (Data && Data->Type == Type)
        {
            Total++;
            if (CollectibleStates.Contains(Data->CollectibleID))
            {
                Collected++;
            }
        }
    }
    return Total > 0 ? static_cast<float>(Collected) / static_cast<float>(Total) : 0.0f;
}

float UCollectibleJournalSubsystem::GetOverallCompletion() const
{
    if (!CollectibleDataTable) return 0.0f;

    int32 Total = CollectibleDataTable->GetRowNames().Num();
    int32 Collected = CollectibleStates.Num();
    return Total > 0 ? static_cast<float>(Collected) / static_cast<float>(Total) : 0.0f;
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
    int32 Count = 0;
    if (!CollectibleDataTable) return 0;

    for (const auto& Pair : CollectibleStates)
    {
        const FCollectibleData* Data = CollectibleDataTable->FindRow<FCollectibleData>(Pair.Key, TEXT(""));
        if (Data && Data->Type == ECollectibleType::ManifestPage)
        {
            Count++;
        }
    }
    return Count;
}

bool UCollectibleJournalSubsystem::IsManifestTruthUnlocked() const
{
    return GetManifestPagesCollected() >= 25;
}

TArray<FName> UCollectibleJournalSubsystem::GetManifestCrossReferences() const
{
    TArray<FName> CrossRefs;
    if (!CollectibleDataTable) return CrossRefs;

    for (const auto& Pair : CollectibleStates)
    {
        const FCollectibleData* Data = CollectibleDataTable->FindRow<FCollectibleData>(Pair.Key, TEXT(""));
        if (Data && Data->Type == ECollectibleType::ManifestPage)
        {
            for (const FName& Linked : Data->LinkedCollectibles)
            {
                CrossRefs.AddUnique(Linked);
            }
        }
    }
    return CrossRefs;
}

TArray<FName> UCollectibleJournalSubsystem::GetCodexEntries() const
{
    TArray<FName> Entries;
    if (!CollectibleDataTable) return Entries;

    for (const auto& Pair : CollectibleStates)
    {
        const FCollectibleData* Data = CollectibleDataTable->FindRow<FCollectibleData>(Pair.Key, TEXT(""));
        if (Data)
        {
            for (const FName& Entry : Data->CodexEntries)
            {
                Entries.AddUnique(Entry);
            }
        }
    }
    return Entries;
}

bool UCollectibleJournalSubsystem::IsCodexEntryComplete(FName EntryID) const
{
    if (!CollectibleDataTable) return false;

    // A codex entry is complete when all collectibles referencing it are collected
    TArray<FName> RowNames = CollectibleDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        const FCollectibleData* Data = CollectibleDataTable->FindRow<FCollectibleData>(RowName, TEXT(""));
        if (Data && Data->CodexEntries.Contains(EntryID))
        {
            if (!CollectibleStates.Contains(Data->CollectibleID))
            {
                return false;
            }
        }
    }
    return true;
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
    if (Data.Type != ECollectibleType::FactionIntel || Data.FactionID.IsNone()) return;

    FFactionIntelProgress& Progress = FactionProgress.FindOrAdd(Data.FactionID);
    Progress.FactionID = Data.FactionID;
    Progress.IntelCollected++;

    if (Progress.IntelCollected >= 5)
    {
        Progress.bSpecialDialogueUnlocked = true;
    }
    if (Progress.IntelCollected >= 10)
    {
        Progress.bCriticalWeaknessRevealed = true;
    }
}

void UCollectibleJournalSubsystem::UpdateCodex(const FCollectibleData& Data)
{
    for (const FName& EntryID : Data.CodexEntries)
    {
        bool bComplete = IsCodexEntryComplete(EntryID);
        OnCodexEntryUpdated.Broadcast(EntryID, bComplete);
    }
}

void UCollectibleJournalSubsystem::CheckMilestones(ECollectibleType Type)
{
    float Completion = GetCompletionPercentage(Type);

    static const float MilestoneThresholds[] = { 0.25f, 0.5f, 0.75f, 1.0f };
    for (float Threshold : MilestoneThresholds)
    {
        if (FMath::IsNearlyEqual(Completion, Threshold, 0.01f))
        {
            OnCompletionMilestone.Broadcast(Type, Completion);
            break;
        }
    }
}

void UCollectibleJournalSubsystem::ProcessManifestPage(const FCollectibleData& Data)
{
    if (Data.Type != ECollectibleType::ManifestPage) return;

    // Check for cross-references with other manifest pages
    for (const FName& LinkedID : Data.LinkedCollectibles)
    {
        if (CollectibleStates.Contains(LinkedID))
        {
            OnManifestCrossReference.Broadcast(LinkedID);
        }
    }
}
