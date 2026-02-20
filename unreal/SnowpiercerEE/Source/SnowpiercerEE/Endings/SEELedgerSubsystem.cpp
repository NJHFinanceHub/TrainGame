// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEELedgerSubsystem.h"

void USEELedgerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// All axes start at zero — no moral leaning until the player acts
	LedgerScores = FSEELedgerSnapshot();
}

void USEELedgerSubsystem::RecordChoice(const FSEELedgerEntry& Entry)
{
	// Apply all axis deltas from this choice
	for (const auto& Pair : Entry.AxisDeltas)
	{
		int32 OldScore = LedgerScores.GetAxisScore(Pair.Key);
		LedgerScores.ApplyDelta(Pair.Key, Pair.Value);
		int32 NewScore = LedgerScores.GetAxisScore(Pair.Key);

		if (OldScore != NewScore)
		{
			OnLedgerAxisChanged.Broadcast(Pair.Key, NewScore);
		}
	}

	// Log to history
	ChoiceHistory.Add(Entry);

	OnMoralChoiceRecorded.Broadcast(Entry);
}

void USEELedgerSubsystem::ModifyAxis(ESEELedgerAxis Axis, int32 Delta)
{
	if (Delta == 0 || Axis == ESEELedgerAxis::COUNT)
	{
		return;
	}

	int32 OldScore = LedgerScores.GetAxisScore(Axis);
	LedgerScores.ApplyDelta(Axis, Delta);
	int32 NewScore = LedgerScores.GetAxisScore(Axis);

	if (OldScore != NewScore)
	{
		OnLedgerAxisChanged.Broadcast(Axis, NewScore);
	}
}

int32 USEELedgerSubsystem::GetAxisScore(ESEELedgerAxis Axis) const
{
	return LedgerScores.GetAxisScore(Axis);
}

void USEELedgerSubsystem::SetGlobalFlag(FName FlagName, bool Value)
{
	bool* Existing = GlobalFlags.Find(FlagName);
	if (Existing && *Existing == Value)
	{
		return;
	}

	GlobalFlags.Add(FlagName, Value);
	OnGlobalFlagChanged.Broadcast(FlagName, Value);
}

bool USEELedgerSubsystem::GetGlobalFlag(FName FlagName) const
{
	const bool* Found = GlobalFlags.Find(FlagName);
	return Found ? *Found : false;
}

void USEELedgerSubsystem::SetGlobalIntFlag(FName FlagName, int32 Value)
{
	GlobalIntFlags.Add(FlagName, Value);
}

int32 USEELedgerSubsystem::GetGlobalIntFlag(FName FlagName) const
{
	const int32* Found = GlobalIntFlags.Find(FlagName);
	return Found ? *Found : 0;
}

bool USEELedgerSubsystem::HasChoiceBeenMade(FName ChoiceID) const
{
	for (const FSEELedgerEntry& Entry : ChoiceHistory)
	{
		if (Entry.ChoiceID == ChoiceID)
		{
			return true;
		}
	}
	return false;
}
