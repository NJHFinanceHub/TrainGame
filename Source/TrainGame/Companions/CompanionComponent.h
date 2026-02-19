// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CompanionTypes.h"
#include "CompanionComponent.generated.h"

class UCompanionDataAsset;
class UCombatComponent;

// ============================================================================
// UCompanionComponent
//
// Core component for companion NPCs. Manages loyalty, behavior mode,
// formation, personal quests, and unique abilities. Attach to any
// companion character actor alongside UCombatComponent.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoyaltyStateChanged, ELoyaltyState, OldState, ELoyaltyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviorChanged, ECompanionBehavior, NewBehavior);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCompanionDeath, EPermadeathCause, Cause);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStepCompleted, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCompanionConflict, FName, CompanionA, FName, CompanionB);

UCLASS(ClassGroup=(Companion), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UCompanionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCompanionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Behavior Mode ---

	/** Set companion AI behavior mode (Aggressive/Defensive/Passive/Hold) */
	UFUNCTION(BlueprintCallable, Category = "Companion|Behavior")
	void SetBehaviorMode(ECompanionBehavior NewBehavior);

	UFUNCTION(BlueprintPure, Category = "Companion|Behavior")
	ECompanionBehavior GetBehaviorMode() const { return CurrentBehavior; }

	// --- Loyalty System ---

	/** Apply a loyalty event. Loyalty value is hidden from player. */
	UFUNCTION(BlueprintCallable, Category = "Companion|Loyalty")
	void ApplyLoyaltyEvent(const FLoyaltyEvent& Event);

	UFUNCTION(BlueprintPure, Category = "Companion|Loyalty")
	ELoyaltyState GetLoyaltyState() const;

	/** Get raw loyalty for internal systems (NOT exposed to UI) */
	UFUNCTION(BlueprintPure, Category = "Companion|Loyalty")
	int32 GetLoyaltyScore() const { return LoyaltyScore; }

	/** Check if loyalty is high enough for a specific gate */
	UFUNCTION(BlueprintPure, Category = "Companion|Loyalty")
	bool MeetsLoyaltyThreshold(ELoyaltyState RequiredState) const;

	// --- Formation ---

	/** Set formation position for corridor navigation */
	UFUNCTION(BlueprintCallable, Category = "Companion|Formation")
	void SetFormationPosition(EFormationPosition Position);

	UFUNCTION(BlueprintPure, Category = "Companion|Formation")
	EFormationPosition GetFormationPosition() const { return CurrentFormation; }

	/** Request formation swap (front/rear rotation) */
	UFUNCTION(BlueprintCallable, Category = "Companion|Formation")
	void RequestFormationSwap();

	// --- Recruitment & Death ---

	/** Recruit this companion into the player's roster */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	void Recruit();

	/** Dismiss from active party (moves to roster, waits at rest point) */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	void DismissToRoster();

	/** Activate from roster into the party */
	UFUNCTION(BlueprintCallable, Category = "Companion|Roster")
	void ActivateFromRoster();

	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	bool IsRecruited() const { return bRecruited; }

	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	bool IsInActiveParty() const { return bInActiveParty; }

	UFUNCTION(BlueprintPure, Category = "Companion|Roster")
	bool IsAlive() const { return DeathCause == EPermadeathCause::None; }

	/** Trigger permadeath. Irreversible. */
	UFUNCTION(BlueprintCallable, Category = "Companion|Death")
	void TriggerPermadeath(EPermadeathCause Cause);

	// --- Unique Ability ---

	/** Attempt to use the companion's unique ability */
	UFUNCTION(BlueprintCallable, Category = "Companion|Ability")
	bool TryUseUniqueAbility();

	UFUNCTION(BlueprintPure, Category = "Companion|Ability")
	bool IsUniqueAbilityAvailable() const;

	// --- Personal Quests ---

	UFUNCTION(BlueprintPure, Category = "Companion|Quest")
	TArray<FCompanionQuestStep> GetQuestSteps() const { return PersonalQuestSteps; }

	UFUNCTION(BlueprintCallable, Category = "Companion|Quest")
	void CompleteQuestStep(FName QuestID);

	UFUNCTION(BlueprintPure, Category = "Companion|Quest")
	FCompanionQuestStep GetCurrentQuestStep() const;

	// --- Romance ---

	UFUNCTION(BlueprintPure, Category = "Companion|Romance")
	bool IsRomanceable() const { return bRomanceable; }

	UFUNCTION(BlueprintPure, Category = "Companion|Romance")
	bool IsRomanceActive() const { return bRomanceActive; }

	UFUNCTION(BlueprintCallable, Category = "Companion|Romance")
	void InitiateRomance();

	UFUNCTION(BlueprintCallable, Category = "Companion|Romance")
	void EndRomance();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Companion")
	FCompanionStats GetStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "Companion")
	ECompanionRole GetRole() const { return Role; }

	UFUNCTION(BlueprintPure, Category = "Companion")
	FName GetCompanionID() const { return CompanionID; }

	UFUNCTION(BlueprintPure, Category = "Companion")
	FText GetCompanionName() const { return CompanionName; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnLoyaltyStateChanged OnLoyaltyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnBehaviorChanged OnBehaviorChanged;

	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnCompanionDeath OnCompanionDeath;

	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnQuestStepCompleted OnQuestStepCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Companion")
	FOnCompanionConflict OnCompanionConflict;

protected:
	virtual void BeginPlay() override;

	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	FName CompanionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	FText CompanionName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	FText CompanionTitle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	ECompanionRole Role = ECompanionRole::Support;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	ERecruitmentZone RecruitmentZone = ERecruitmentZone::Zone1_Tail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Companion|Identity")
	bool bRomanceable = false;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Stats")
	FCompanionStats Stats;

	// --- Loyalty ---

	/** Loyalty score: -100 to +100. Hidden from player. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Companion|Loyalty")
	int32 LoyaltyScore = 15;

	/** Multiplier for positive loyalty events (affected by Natural Leader perk) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Loyalty")
	float PositiveLoyaltyMultiplier = 1.0f;

	/** Multiplier for negative loyalty events (affected by Natural Leader perk) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Loyalty")
	float NegativeLoyaltyMultiplier = 1.0f;

	// --- Combat Synergy ---

	/** Damage bonus when near player at Friendly+ loyalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Combat")
	float FriendlySynergyBonus = 0.10f;

	/** Damage bonus when near player at Devoted+ loyalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Combat")
	float DevotedSynergyBonus = 0.15f;

	/** Combat effectiveness penalty when Resentful */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Combat")
	float ResentfulPenalty = 0.20f;

	/** Combat effectiveness penalty when Bitter (Oathkeeper + -100 loyalty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Combat")
	float BitterPenalty = 0.30f;

	// --- Revive ---

	/** Time to revive when downed (default 10s, Marta overrides to 5s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Revive")
	float ReviveTime = 10.0f;

	/** HP percentage restored on revive */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Revive")
	float ReviveHPPercent = 0.10f;

	/** Time window for revive before permadeath triggers */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Revive")
	float PermadeathTimer = 30.0f;

	// --- Personal Quests ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Quest")
	TArray<FCompanionQuestStep> PersonalQuestSteps;

	// --- Unique Ability ---

	/** Cooldown for unique ability (per encounter or timed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Ability")
	int32 MaxAbilityUsesPerEncounter = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Ability")
	int32 BondedAbilityUsesPerEncounter = 1;

	// --- Data Asset ---

	/** Reference to companion data asset containing dialogue, visuals, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion|Data")
	UCompanionDataAsset* CompanionData = nullptr;

private:
	ELoyaltyState CalculateLoyaltyState(int32 Score) const;
	void ClampLoyalty();
	void CheckAbandonmentThreshold();
	void UpdateCombatSynergy();

	ECompanionBehavior CurrentBehavior = ECompanionBehavior::Defensive;
	EFormationPosition CurrentFormation = EFormationPosition::Middle;
	EPermadeathCause DeathCause = EPermadeathCause::None;

	bool bRecruited = false;
	bool bInActiveParty = false;
	bool bRomanceActive = false;

	int32 AbilityUsesThisEncounter = 0;
	float DownedTimer = 0.0f;
};
