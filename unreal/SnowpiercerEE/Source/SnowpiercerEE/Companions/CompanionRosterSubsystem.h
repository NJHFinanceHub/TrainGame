// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CompanionTypes.h"
#include "CompanionRosterSubsystem.generated.h"

class UCompanionComponent;
class UCompanionDataAsset;

// ============================================================================
// UCompanionRosterSubsystem
//
// Game instance subsystem that manages the companion roster, active party,
// companion conflicts, synergy bonuses, and permadeath tracking.
// Persists across level transitions within a game session.
// ============================================================================

UCLASS()
class SNOWPIERCEREE_API UCompanionRosterSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- Party Management ---

	/** Maximum companions in active party (default: 2) */
	static constexpr int32 MAX_ACTIVE_PARTY_SIZE = 2;

	/** Maximum roster size (one per companion in the game) */
	static constexpr int32 MAX_ROSTER_SIZE = 12;

	/** Add companion to roster when recruited */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	bool AddToRoster(UCompanionComponent* Companion);

	/** Move companion from roster to active party */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	bool AddToActiveParty(UCompanionComponent* Companion);

	/** Remove companion from active party (back to roster) */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	void RemoveFromActiveParty(UCompanionComponent* Companion);

	/** Get all companions in the active party */
	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	TArray<UCompanionComponent*> GetActiveParty() const { return ActiveParty; }

	/** Get all recruited companions (active + roster) */
	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	TArray<UCompanionComponent*> GetFullRoster() const { return Roster; }

	/** Get companions waiting at rest points */
	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	TArray<UCompanionComponent*> GetBenchedCompanions() const;

	/** Check if party swap is available (must be at rest point) */
	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	bool CanSwapPartyMembers() const { return bAtRestPoint; }

	/** Notify subsystem that player is at a rest point */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	void SetAtRestPoint(bool bIsAtRestPoint) { bAtRestPoint = bIsAtRestPoint; }

	// --- Permadeath ---

	/** Get all dead companions and their causes of death */
	UFUNCTION(BlueprintPure, Category = "Companion|Death")
	TArray<UCompanionComponent*> GetDeadCompanions() const;

	/** Check if a specific companion is dead */
	UFUNCTION(BlueprintPure, Category = "Companion|Death")
	bool IsCompanionDead(FName CompanionID) const;

	// --- Companion Conflicts ---

	/** Trigger a companion conflict between two active party members */
	UFUNCTION(BlueprintCallable, Category = "Companion|Conflict")
	void TriggerConflict(const FCompanionConflict& Conflict);

	/** Resolve a conflict in favor of one companion */
	UFUNCTION(BlueprintCallable, Category = "Companion|Conflict")
	void ResolveConflict(FName WinnerID, bool bCompromise);

	// --- Synergy ---

	/** Get the synergy bonus for a specific pair of companions */
	UFUNCTION(BlueprintPure, Category = "Companion|Synergy")
	float GetPairSynergyBonus(FName CompanionA, FName CompanionB) const;

	/** Check if a synergy pair is currently in the active party */
	UFUNCTION(BlueprintPure, Category = "Companion|Synergy")
	bool IsSynergyPairActive(FName CompanionA, FName CompanionB) const;

	// --- Romance ---

	/** Get the currently romanced companion (if any) */
	UFUNCTION(BlueprintPure, Category = "Companion|Romance")
	UCompanionComponent* GetRomancedCompanion() const;

	/** Check if attempting to romance a second companion while one is active */
	UFUNCTION(BlueprintPure, Category = "Companion|Romance")
	bool HasActiveRomance() const;

	// --- Memorial ---

	/** Trigger the memorial cutscene for a recently deceased companion */
	UFUNCTION(BlueprintCallable, Category = "Companion|Death")
	void TriggerMemorial(FName DeceasedCompanionID);

	// --- Oathkeeper Perk ---

	/** Set whether the Oathkeeper perk is active (prevents abandonment) */
	UFUNCTION(BlueprintCallable, Category = "Companion|Perks")
	void SetOathkeeperActive(bool bActive) { bOathkeeperActive = bActive; }

	UFUNCTION(BlueprintPure, Category = "Companion|Perks")
	bool IsOathkeeperActive() const { return bOathkeeperActive; }

	/** Set Natural Leader perk multipliers on all companions */
	UFUNCTION(BlueprintCallable, Category = "Companion|Perks")
	void SetNaturalLeaderActive(bool bActive);

private:
	UPROPERTY()
	TArray<UCompanionComponent*> Roster;

	UPROPERTY()
	TArray<UCompanionComponent*> ActiveParty;

	bool bAtRestPoint = false;
	bool bOathkeeperActive = false;

	FCompanionConflict CurrentConflict;
	bool bConflictActive = false;
};
