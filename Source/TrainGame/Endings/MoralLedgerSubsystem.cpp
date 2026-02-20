// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "MoralLedgerSubsystem.h"

void UMoralLedgerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	for (int32 i = 0; i < static_cast<int32>(EMoralAxis::MAX); ++i)
	{
		AxisValues[i] = 0;
	}
}

// ---------------------------------------------------------------------------
// Moral Axis Operations
// ---------------------------------------------------------------------------

void UMoralLedgerSubsystem::ModifyAxis(EMoralAxis Axis, int32 Delta)
{
	if (Axis == EMoralAxis::MAX) return;

	const int32 Index = static_cast<int32>(Axis);
	const int32 OldValue = AxisValues[Index];
	AxisValues[Index] = FMath::Clamp(OldValue + Delta, -100, 100);

	if (AxisValues[Index] != OldValue)
	{
		OnMoralAxisChanged.Broadcast(Axis, AxisValues[Index]);
	}
}

int32 UMoralLedgerSubsystem::GetAxisValue(EMoralAxis Axis) const
{
	if (Axis == EMoralAxis::MAX) return 0;
	return AxisValues[static_cast<int32>(Axis)];
}

float UMoralLedgerSubsystem::GetAxisNormalized(EMoralAxis Axis) const
{
	return static_cast<float>(GetAxisValue(Axis)) / 100.0f;
}

// ---------------------------------------------------------------------------
// Choice Recording
// ---------------------------------------------------------------------------

void UMoralLedgerSubsystem::RecordChoice(FName ChoiceID, FName OptionChosen, int32 CarIndex, const TMap<uint8, int32>& AxisDeltas)
{
	FMoralChoiceRecord Record;
	Record.ChoiceID = ChoiceID;
	Record.OptionChosen = OptionChosen;
	Record.CarIndex = CarIndex;
	Record.AxisDeltas = AxisDeltas;

	if (const UWorld* World = GetWorld())
	{
		Record.GameTimeWhenChosen = World->GetTimeSeconds();
	}

	ChoiceHistory.Add(Record);

	// Apply axis deltas
	for (const auto& Pair : AxisDeltas)
	{
		const EMoralAxis Axis = static_cast<EMoralAxis>(Pair.Key);
		ModifyAxis(Axis, Pair.Value);
	}

	OnMoralChoiceRecorded.Broadcast(Record);
}

bool UMoralLedgerSubsystem::HasMadeChoice(FName ChoiceID) const
{
	return ChoiceHistory.ContainsByPredicate([ChoiceID](const FMoralChoiceRecord& R)
	{
		return R.ChoiceID == ChoiceID;
	});
}

FName UMoralLedgerSubsystem::GetChosenOption(FName ChoiceID) const
{
	const FMoralChoiceRecord* Found = ChoiceHistory.FindByPredicate([ChoiceID](const FMoralChoiceRecord& R)
	{
		return R.ChoiceID == ChoiceID;
	});
	return Found ? Found->OptionChosen : NAME_None;
}

// ---------------------------------------------------------------------------
// Story Flags
// ---------------------------------------------------------------------------

void UMoralLedgerSubsystem::SetStoryFlag(FName FlagName, bool bValue)
{
	StoryFlags.Add(FlagName, bValue);
	OnStoryFlagSet.Broadcast(FlagName, bValue);
}

bool UMoralLedgerSubsystem::GetStoryFlag(FName FlagName) const
{
	const bool* Found = StoryFlags.Find(FlagName);
	return Found ? *Found : false;
}

// ---------------------------------------------------------------------------
// Save/Load
// ---------------------------------------------------------------------------

FMoralLedgerSnapshot UMoralLedgerSubsystem::CreateSnapshot() const
{
	FMoralLedgerSnapshot Snapshot;

	Snapshot.AxisValues.SetNum(static_cast<int32>(EMoralAxis::MAX));
	for (int32 i = 0; i < static_cast<int32>(EMoralAxis::MAX); ++i)
	{
		Snapshot.AxisValues[i] = AxisValues[i];
	}

	Snapshot.ChoiceHistory = ChoiceHistory;
	Snapshot.StoryFlags = StoryFlags;

	return Snapshot;
}

void UMoralLedgerSubsystem::RestoreFromSnapshot(const FMoralLedgerSnapshot& Snapshot)
{
	for (int32 i = 0; i < static_cast<int32>(EMoralAxis::MAX) && i < Snapshot.AxisValues.Num(); ++i)
	{
		AxisValues[i] = FMath::Clamp(Snapshot.AxisValues[i], -100, 100);
	}

	ChoiceHistory = Snapshot.ChoiceHistory;
	StoryFlags = Snapshot.StoryFlags;
}
