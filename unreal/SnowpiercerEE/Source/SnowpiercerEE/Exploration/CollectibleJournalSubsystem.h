// CollectibleJournalSubsystem.h
// Snowpiercer: Eternal Engine - Journal/Codex management subsystem

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CollectibleTypes.h"
#include "CollectibleJournalSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollectibleRegistered, FName, CollectibleID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCodexEntryUpdated, FName, EntryID, bool, bNewEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCompletionMilestone, ECollectibleType, Type, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManifestCrossReference, FName, ManifestEntryID);

/**
 * Manages all collected items, journal state, codex entries, Web of Power,
 * and manifest reconstruction. Persists across level transitions.
 */
UCLASS()
class SNOWPIERCEREE_API UCollectibleJournalSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Collection Management ---

	/** Register a newly collected item. Triggers all downstream effects. */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void RegisterCollectible(FName CollectibleID);

	/** Mark a collectible as viewed in the journal (clears "new" indicator) */
	UFUNCTION(BlueprintCallable, Category = "Journal")
	void MarkViewed(FName CollectibleID);

	/** Check if a specific collectible has been collected */
	UFUNCTION(BlueprintPure, Category = "Journal")
	bool IsCollected(FName CollectibleID) const;

	// --- Query ---

	/** Get all collected items of a given type */
	UFUNCTION(BlueprintPure, Category = "Journal")
	TArray<FCollectibleState> GetCollectedByType(ECollectibleType Type) const;

	/** Get all collected items in a given zone */
	UFUNCTION(BlueprintPure, Category = "Journal")
	TArray<FCollectibleState> GetCollectedByZone(ECollectibleZone Zone) const;

	/** Get collection percentage for a given type (0.0-1.0) */
	UFUNCTION(BlueprintPure, Category = "Journal")
	float GetCompletionPercentage(ECollectibleType Type) const;

	/** Get overall collection percentage (0.0-1.0) */
	UFUNCTION(BlueprintPure, Category = "Journal")
	float GetOverallCompletion() const;

	/** Get count of unviewed (new) collectibles */
	UFUNCTION(BlueprintPure, Category = "Journal")
	int32 GetUnviewedCount() const;

	// --- Web of Power ---

	/** Get faction intel progress for all factions */
	UFUNCTION(BlueprintPure, Category = "Journal|WebOfPower")
	TArray<FFactionIntelProgress> GetAllFactionProgress() const;

	/** Get intel progress for a specific faction */
	UFUNCTION(BlueprintPure, Category = "Journal|WebOfPower")
	FFactionIntelProgress GetFactionProgress(FName FactionID) const;

	/** Check if special dialogue is unlocked for a faction (5+ intel) */
	UFUNCTION(BlueprintPure, Category = "Journal|WebOfPower")
	bool IsFactionDialogueUnlocked(FName FactionID) const;

	// --- Manifest ---

	/** Get number of manifest pages collected */
	UFUNCTION(BlueprintPure, Category = "Journal|Manifest")
	int32 GetManifestPagesCollected() const;

	/** Check if the Manifest Truth quest is triggered (25+ pages) */
	UFUNCTION(BlueprintPure, Category = "Journal|Manifest")
	bool IsManifestTruthUnlocked() const;

	/** Get active cross-references from the manifest */
	UFUNCTION(BlueprintPure, Category = "Journal|Manifest")
	TArray<FName> GetManifestCrossReferences() const;

	// --- Codex ---

	/** Get all codex entry IDs that have been created */
	UFUNCTION(BlueprintPure, Category = "Journal|Codex")
	TArray<FName> GetCodexEntries() const;

	/** Check if a codex entry has been completed (all sources found) */
	UFUNCTION(BlueprintPure, Category = "Journal|Codex")
	bool IsCodexEntryComplete(FName EntryID) const;

	// --- Milestone Rewards ---

	/** Check if a completion milestone reward has been granted */
	UFUNCTION(BlueprintPure, Category = "Journal|Rewards")
	bool IsMilestoneReached(float Percentage) const;

	/** Get the player's current exploration title */
	UFUNCTION(BlueprintPure, Category = "Journal|Rewards")
	FText GetCurrentExplorationTitle() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Journal|Events")
	FOnCollectibleRegistered OnCollectibleRegistered;

	UPROPERTY(BlueprintAssignable, Category = "Journal|Events")
	FOnCodexEntryUpdated OnCodexEntryUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Journal|Events")
	FOnCompletionMilestone OnCompletionMilestone;

	UPROPERTY(BlueprintAssignable, Category = "Journal|Events")
	FOnManifestCrossReference OnManifestCrossReference;

protected:
	/** Data table containing all collectible definitions */
	UPROPERTY()
	TObjectPtr<UDataTable> CollectibleDataTable;

	/** Runtime state of all collectibles */
	UPROPERTY()
	TMap<FName, FCollectibleState> CollectibleStates;

	/** Faction intel tracking */
	UPROPERTY()
	TMap<FName, FFactionIntelProgress> FactionProgress;

private:
	void UpdateFactionProgress(const FCollectibleData& Data);
	void UpdateCodex(const FCollectibleData& Data);
	void CheckMilestones(ECollectibleType Type);
	void ProcessManifestPage(const FCollectibleData& Data);
};
