// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CompanionTypes.h"
#include "CompanionDataAsset.generated.h"

// ============================================================================
// UCompanionDataAsset
//
// Data asset defining a companion's static configuration: identity, stats,
// dialogue references, quest definitions, loyalty event tables, and
// visual/audio references. One per companion.
// ============================================================================

/** Loyalty event trigger definition */
USTRUCT(BlueprintType)
struct FLoyaltyEventTrigger
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TriggerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELoyaltyEventType EventType = ELoyaltyEventType::DialogueApproved;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LoyaltyChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
};

/** Companion synergy pair definition */
USTRUCT(BlueprintType)
struct FCompanionSynergy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PartnerCompanionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SynergyDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageBonusPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpecialBonusValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpecialBonusDescription;
};

UCLASS(BlueprintType)
class TRAINGAME_API UCompanionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FName CompanionID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity", meta = (MultiLine = true))
	FText Backstory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	ECompanionRole Role = ECompanionRole::Support;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	ERecruitmentZone RecruitmentZone = ERecruitmentZone::Zone1_Tail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	int32 RecruitmentCar = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Identity")
	bool bRomanceable = false;

	// --- Stats ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	FCompanionStats BaseStats;

	// --- Combat ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ECompanionBehavior DefaultBehavior = ECompanionBehavior::Defensive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FText WeaponDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FText UniqueAbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (MultiLine = true))
	FText UniqueAbilityDescription;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FText UniqueAbilityBondedUpgrade;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 BaseAbilityUsesPerEncounter = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 BondedAbilityUsesPerEncounter = 1;

	// --- Loyalty ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loyalty")
	int32 StartingLoyalty = 15;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loyalty")
	TArray<FLoyaltyEventTrigger> ApprovalTriggers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loyalty")
	TArray<FLoyaltyEventTrigger> DisapprovalTriggers;

	// --- Personal Quests ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quests")
	TArray<FCompanionQuestStep> QuestChain;

	// --- Synergies ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Synergy")
	TArray<FCompanionSynergy> Synergies;

	// --- Conflicts ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conflict")
	TArray<FCompanionConflict> PotentialConflicts;

	// --- Visual References (asset paths, populated in editor) ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<USkeletalMesh> CharacterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Visuals")
	TSoftClassPtr<AActor> CompanionBlueprintClass;

	// --- Audio References ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> RecruitmentDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TSoftObjectPtr<USoundBase> MemorialCutsceneAudio;
};
