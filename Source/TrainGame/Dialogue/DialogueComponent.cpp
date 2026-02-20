// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DialogueComponent.h"
#include "NPCMemoryComponent.h"
#include "DialogueDataAsset.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInDialogue)
	{
		TickTimer(DeltaTime);
	}
}

// --- Dialogue Flow ---

FDialogueNode UDialogueComponent::StartDialogue()
{
	bIsInDialogue = true;
	bWasInterrupted = false;
	FailedChecks.Empty();
	PrimaryComponentTick.SetTickFunctionEnable(true);

	// TODO: Load hub node from DialogueData asset
	// CurrentNodeID = DialogueData->GetHubNodeID();

	OnDialogueStarted.Broadcast(NPCID);

	FDialogueNode HubNode;
	HubNode.NodeType = EDialogueNodeType::Hub;
	return HubNode;
}

FDialogueNode UDialogueComponent::SelectOption(int32 OptionIndex)
{
	// TODO: Validate index, resolve skill check if present, navigate graph
	// 1. Get current node from DialogueData
	// 2. Get option at OptionIndex
	// 3. If option has skill check, resolve it
	// 4. Navigate to success/failure/target node
	// 5. If option is one-shot, mark it used
	// 6. If option is social approach, apply disposition changes
	// 7. Return the new current node

	FDialogueNode ResultNode;
	return ResultNode;
}

void UDialogueComponent::EndDialogue()
{
	bIsInDialogue = false;
	CurrentNodeID = NAME_None;
	CurrentTimerRemaining = 0.f;
	PrimaryComponentTick.SetTickFunctionEnable(false);

	OnDialogueEnded.Broadcast(NPCID);
}

FDialogueNode UDialogueComponent::ReturnToHub()
{
	// TODO: Navigate back to the hub node
	// CurrentNodeID = DialogueData->GetHubNodeID();

	FDialogueNode HubNode;
	HubNode.NodeType = EDialogueNodeType::Hub;
	return HubNode;
}

// --- Skill Checks ---

bool UDialogueComponent::ResolveSkillCheck(const FDialogueSkillCheck& Check) const
{
	// TODO: Get player stat value from player character/stats component
	// int32 PlayerStat = GetPlayerStatValue(Check.Stat);
	int32 PlayerStat = 0; // Stub

	bool bSuccess = false;

	switch (Check.CheckType)
	{
	case ESkillCheckType::Passive:
		bSuccess = PlayerStat >= Check.Threshold;
		break;

	case ESkillCheckType::Active:
		bSuccess = PlayerStat >= Check.Threshold;
		break;

	case ESkillCheckType::Contested:
		bSuccess = PlayerStat > Check.NPCContestValue;
		break;
	}

	return bSuccess;
}

ESkillCheckDifficulty UDialogueComponent::GetCheckDifficulty(const FDialogueSkillCheck& Check) const
{
	// TODO: Get actual player stat
	int32 PlayerStat = 0; // Stub
	int32 Difference = PlayerStat - Check.Threshold;

	if (Difference >= 3)
	{
		return ESkillCheckDifficulty::Guaranteed;
	}
	if (Difference >= -2)
	{
		return ESkillCheckDifficulty::Uncertain;
	}
	if (Difference >= -7)
	{
		return ESkillCheckDifficulty::Unlikely;
	}
	return ESkillCheckDifficulty::Impossible;
}

bool UDialogueComponent::PassesPassiveCheck(const FDialogueSkillCheck& Check) const
{
	if (Check.CheckType != ESkillCheckType::Passive)
	{
		return true; // Non-passive checks are always visible
	}
	return ResolveSkillCheck(Check);
}

// --- Timed Decisions ---

float UDialogueComponent::GetRemainingTime() const
{
	return CurrentTimerRemaining;
}

EDialogueUrgency UDialogueComponent::GetCurrentUrgency() const
{
	// TODO: Get from current node
	return EDialogueUrgency::None;
}

void UDialogueComponent::TickTimer(float DeltaTime)
{
	if (CurrentTimerRemaining <= 0.f)
	{
		return;
	}

	CurrentTimerRemaining -= DeltaTime;

	if (CurrentTimerRemaining <= 0.f)
	{
		CurrentTimerRemaining = 0.f;
		OnTimerExpired.Broadcast();
		// TODO: Auto-select the timeout default option
	}
}

// --- Social Combat ---

bool UDialogueComponent::AttemptSocialApproach(ESocialApproach Approach, int32 PlayerStatValue)
{
	int32 Modifier = 0;

	switch (Approach)
	{
	case ESocialApproach::Persuasion:
		Modifier = SocialResistance.PersuasionModifier;
		break;
	case ESocialApproach::Intimidation:
		Modifier = SocialResistance.IntimidationModifier;
		break;
	case ESocialApproach::Deception:
		Modifier = SocialResistance.DeceptionModifier;
		break;
	}

	int32 EffectiveValue = PlayerStatValue + Modifier;

	// TODO: Compare against NPC's base resistance threshold
	// For now, threshold is 5 + zone difficulty
	int32 Threshold = 5;
	bool bSuccess = EffectiveValue >= Threshold;

	// Apply disposition changes
	int32 OldDisposition = Disposition;

	if (bSuccess)
	{
		switch (Approach)
		{
		case ESocialApproach::Persuasion:
			Disposition = FMath::Clamp(Disposition + 10, -100, 100);
			break;
		case ESocialApproach::Intimidation:
			Disposition = FMath::Clamp(Disposition - 15, -100, 100);
			break;
		case ESocialApproach::Deception:
			// No disposition change on successful deception (they don't know)
			break;
		}
	}
	else
	{
		switch (Approach)
		{
		case ESocialApproach::Persuasion:
			Disposition = FMath::Clamp(Disposition - 5, -100, 100);
			break;
		case ESocialApproach::Intimidation:
			Disposition = FMath::Clamp(Disposition - 15, -100, 100);
			break;
		case ESocialApproach::Deception:
			Disposition = FMath::Clamp(Disposition - 20, -100, 100);
			break;
		}
	}

	if (OldDisposition != Disposition)
	{
		OnDispositionChanged.Broadcast(OldDisposition, Disposition);
	}

	return bSuccess;
}

// --- Interruption ---

void UDialogueComponent::InterruptDialogue(EDialogueInterruptSource Source)
{
	if (!bIsInDialogue)
	{
		return;
	}

	bWasInterrupted = true;
	LastInterruptSource = Source;
	InterruptedNodeID = CurrentNodeID;

	// Disposition penalty for player walk-away
	if (Source == EDialogueInterruptSource::PlayerWalkAway)
	{
		int32 OldDisposition = Disposition;
		Disposition = FMath::Clamp(Disposition - 5, -100, 100);
		OnDispositionChanged.Broadcast(OldDisposition, Disposition);
	}

	bIsInDialogue = false;
	CurrentTimerRemaining = 0.f;

	OnDialogueInterrupted.Broadcast(Source);
}

FDialogueNode UDialogueComponent::ResumeAfterInterrupt()
{
	bIsInDialogue = true;
	bWasInterrupted = false;

	// Return to hub with context about the interruption
	// TODO: Add interrupt-specific spoke to hub

	FDialogueNode HubNode;
	HubNode.NodeType = EDialogueNodeType::Hub;
	return HubNode;
}

// --- Internal ---

TArray<FDialogueOption> UDialogueComponent::FilterAvailableOptions(const FDialogueNode& Node) const
{
	TArray<FDialogueOption> Available;

	for (const FDialogueOption& Option : Node.Options)
	{
		// Filter one-shot options already used
		if (Option.bOneShot && UsedOneShotOptions.Contains(Option.TargetNode))
		{
			continue;
		}

		// Filter passive checks that fail
		if (Option.bHasSkillCheck && !PassesPassiveCheck(Option.SkillCheck))
		{
			continue;
		}

		// TODO: Check condition tags against game state

		Available.Add(Option);
	}

	return Available;
}

void UDialogueComponent::RecordFailedCheck(FName NodeID, EDialogueStat Stat)
{
	TArray<EDialogueStat>& FailedStats = FailedChecks.FindOrAdd(NodeID);
	FailedStats.AddUnique(Stat);
}

bool UDialogueComponent::WasCheckAlreadyFailed(FName NodeID, EDialogueStat Stat) const
{
	const TArray<EDialogueStat>* FailedStats = FailedChecks.Find(NodeID);
	if (FailedStats)
	{
		return FailedStats->Contains(Stat);
	}
	return false;
}
