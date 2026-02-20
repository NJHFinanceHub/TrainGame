// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueTypes.h"
#include "DialogueComponent.generated.h"

class UDialogueDataAsset;
class UNPCMemoryComponent;

/**
 * UDialogueComponent
 *
 * Attached to NPCs to manage dialogue state. Handles the hub-and-spoke graph,
 * skill check resolution, timed decisions, and interruption/resume logic.
 */
UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDialogueComponent();

	// --- Dialogue Flow ---

	/** Start dialogue with this NPC. Returns the hub node. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FDialogueNode StartDialogue();

	/** Select a dialogue option by index. Returns the resulting node. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FDialogueNode SelectOption(int32 OptionIndex);

	/** End the current dialogue session */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	/** Return to the hub node from any spoke */
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	FDialogueNode ReturnToHub();

	/** Check if dialogue is currently active */
	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return bIsInDialogue; }

	// --- Skill Checks ---

	/** Resolve a skill check against the player's stats. Returns true on success. */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|SkillCheck")
	bool ResolveSkillCheck(const FDialogueSkillCheck& Check) const;

	/** Get the difficulty color for a skill check display */
	UFUNCTION(BlueprintPure, Category = "Dialogue|SkillCheck")
	ESkillCheckDifficulty GetCheckDifficulty(const FDialogueSkillCheck& Check) const;

	/** Check if a passive check is passed (used to filter visible options) */
	UFUNCTION(BlueprintPure, Category = "Dialogue|SkillCheck")
	bool PassesPassiveCheck(const FDialogueSkillCheck& Check) const;

	// --- Timed Decisions ---

	/** Get remaining time on current timed decision (0 if no timer) */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Timer")
	float GetRemainingTime() const;

	/** Get the urgency level of the current node */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Timer")
	EDialogueUrgency GetCurrentUrgency() const;

	// --- Social Combat ---

	/** Attempt a social approach (persuasion/intimidation/deception) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Social")
	bool AttemptSocialApproach(ESocialApproach Approach, int32 PlayerStatValue);

	/** Get the social resistance modifiers for this NPC */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Social")
	FSocialResistance GetSocialResistance() const { return SocialResistance; }

	// --- Interruption ---

	/** Interrupt current dialogue from an external source */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Interrupt")
	void InterruptDialogue(EDialogueInterruptSource Source);

	/** Resume dialogue after interruption (returns to hub with new context) */
	UFUNCTION(BlueprintCallable, Category = "Dialogue|Interrupt")
	FDialogueNode ResumeAfterInterrupt();

	/** Check if dialogue was interrupted and can be resumed */
	UFUNCTION(BlueprintPure, Category = "Dialogue|Interrupt")
	bool CanResume() const { return bWasInterrupted; }

	// --- Delegates ---

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, FName, NPCID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueEnded, FName, NPCID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillCheckResolved, EDialogueStat, Stat, bool, bSuccess);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueInterrupted, EDialogueInterruptSource, Source);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimerExpired);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDispositionChanged, int32, OldValue, int32, NewValue);

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueEnded OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnSkillCheckResolved OnSkillCheckResolved;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDialogueInterrupted OnDialogueInterrupted;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnTimerExpired OnTimerExpired;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue|Events")
	FOnDispositionChanged OnDispositionChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Tick the dialogue timer if active */
	void TickTimer(float DeltaTime);

	/** Filter options based on passive checks and condition tags */
	TArray<FDialogueOption> FilterAvailableOptions(const FDialogueNode& Node) const;

	/** Record a failed skill check to prevent re-attempts */
	void RecordFailedCheck(FName NodeID, EDialogueStat Stat);

	/** Check if a skill check was already failed in this conversation */
	bool WasCheckAlreadyFailed(FName NodeID, EDialogueStat Stat) const;

private:
	/** The dialogue data asset containing all nodes */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	UDialogueDataAsset* DialogueData;

	/** NPC archetype for social resistance */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	ENPCArchetype Archetype = ENPCArchetype::Guard;

	/** Social resistance modifiers */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FSocialResistance SocialResistance;

	/** Current disposition toward the player */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	int32 Disposition = 0;

	/** NPC identifier */
	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FName NPCID = NAME_None;

	// --- Runtime State ---

	bool bIsInDialogue = false;
	bool bWasInterrupted = false;
	EDialogueInterruptSource LastInterruptSource = EDialogueInterruptSource::Combat;
	FName CurrentNodeID = NAME_None;
	FName InterruptedNodeID = NAME_None;
	float CurrentTimerRemaining = 0.f;

	/** Failed checks this conversation: maps NodeID to set of stats */
	TMap<FName, TArray<EDialogueStat>> FailedChecks;

	/** One-shot options already selected */
	TSet<FName> UsedOneShotOptions;
};
