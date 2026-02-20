#include "SEEChoiceTrackingSubsystem.h"

void USEEChoiceTrackingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	SaveData = FSEEChoiceTrackingSaveData();
}

void USEEChoiceTrackingSubsystem::RecordChoice(FName ChoiceID, FName OptionID, int32 CarIndex, const FString& Zone)
{
	FSEEChoiceRecord Record;
	Record.ChoiceID = ChoiceID;
	Record.SelectedOptionID = OptionID;
	Record.CarIndex = CarIndex;
	Record.Zone = Zone;

	if (const UWorld* World = GetWorld())
	{
		Record.GameTimeWhenChosen = World->GetTimeSeconds();
	}

	SaveData.ChoiceHistory.Add(Record);

	OnChoiceMade.Broadcast(ChoiceID, OptionID);
}

bool USEEChoiceTrackingSubsystem::HasMadeChoice(FName ChoiceID) const
{
	for (const FSEEChoiceRecord& Record : SaveData.ChoiceHistory)
	{
		if (Record.ChoiceID == ChoiceID)
		{
			return true;
		}
	}
	return false;
}

FName USEEChoiceTrackingSubsystem::GetChosenOption(FName ChoiceID) const
{
	for (const FSEEChoiceRecord& Record : SaveData.ChoiceHistory)
	{
		if (Record.ChoiceID == ChoiceID)
		{
			return Record.SelectedOptionID;
		}
	}
	return NAME_None;
}

void USEEChoiceTrackingSubsystem::ModifyLedger(ESEELedgerAxis Axis, int32 Delta)
{
	int32& Ref = GetLedgerRef(Axis);
	Ref = ClampLedger(Ref + Delta);
	OnLedgerChanged.Broadcast(Axis, Ref);
}

int32 USEEChoiceTrackingSubsystem::GetLedgerValue(ESEELedgerAxis Axis) const
{
	const FSEELedgerSnapshot& L = SaveData.Ledger;
	switch (Axis)
	{
	case ESEELedgerAxis::MercyVsPragmatism:		return L.MercyPragmatism;
	case ESEELedgerAxis::IndividualVsCollective:	return L.IndividualCollective;
	case ESEELedgerAxis::TruthVsStability:		return L.TruthStability;
	case ESEELedgerAxis::ForceVsCunning:			return L.ForceCunning;
	case ESEELedgerAxis::PresentVsFuture:			return L.PresentFuture;
	default: return 0;
	}
}

bool USEEChoiceTrackingSubsystem::IsEndingAvailable(FName EndingID) const
{
	const FSEELedgerSnapshot& L = SaveData.Ledger;

	if (EndingID == FName("Ending_Revolution"))
	{
		return true; // Always available
	}
	if (EndingID == FName("Ending_NewWilford"))
	{
		return true; // Always available
	}
	if (EndingID == FName("Ending_Exodus"))
	{
		// Requires Thaw evidence
		return IsFlagSet(FName("Flag_Thaw_Confirmed")) || IsFlagSet(FName("Flag_Nasser_Free"));
	}
	if (EndingID == FName("Ending_EternalLoop"))
	{
		return true; // Always available
	}
	if (EndingID == FName("Ending_Bridge"))
	{
		// Requires high Mercy, high Truth, high Future, Thaw evidence, and coalition
		return L.MercyPragmatism >= 30
			&& L.TruthStability >= 20
			&& L.PresentFuture <= -20 // Negative = Future-leaning
			&& (IsFlagSet(FName("Flag_Thaw_Confirmed")) || IsFlagSet(FName("Flag_Nasser_Free")))
			&& IsFlagSet(FName("Flag_Kazakov_Recruited"));
	}
	if (EndingID == FName("Ending_Derailment"))
	{
		// Requires consistently nihilistic choices
		return L.MercyPragmatism <= -40
			&& L.TruthStability <= -20;
	}

	return false;
}

void USEEChoiceTrackingSubsystem::SetFlag(FName FlagName)
{
	SaveData.ChoiceFlags.Add(FlagName, true);
	OnFlagSet.Broadcast(FlagName);
}

void USEEChoiceTrackingSubsystem::ClearFlag(FName FlagName)
{
	SaveData.ChoiceFlags.Remove(FlagName);
}

bool USEEChoiceTrackingSubsystem::IsFlagSet(FName FlagName) const
{
	const bool* Value = SaveData.ChoiceFlags.Find(FlagName);
	return Value != nullptr && *Value;
}

TArray<FName> USEEChoiceTrackingSubsystem::GetAllFlags() const
{
	TArray<FName> Flags;
	SaveData.ChoiceFlags.GetKeys(Flags);
	return Flags;
}

void USEEChoiceTrackingSubsystem::IncrementCounter(FName CounterName, int32 Delta)
{
	int32& Value = SaveData.CompoundingCounters.FindOrAdd(CounterName);
	Value += Delta;
}

int32 USEEChoiceTrackingSubsystem::GetCounter(FName CounterName) const
{
	const int32* Value = SaveData.CompoundingCounters.Find(CounterName);
	return Value ? *Value : 0;
}

void USEEChoiceTrackingSubsystem::RestoreFromSaveData(const FSEEChoiceTrackingSaveData& Data)
{
	SaveData = Data;
}

int32& USEEChoiceTrackingSubsystem::GetLedgerRef(ESEELedgerAxis Axis)
{
	switch (Axis)
	{
	case ESEELedgerAxis::MercyVsPragmatism:		return SaveData.Ledger.MercyPragmatism;
	case ESEELedgerAxis::IndividualVsCollective:	return SaveData.Ledger.IndividualCollective;
	case ESEELedgerAxis::TruthVsStability:		return SaveData.Ledger.TruthStability;
	case ESEELedgerAxis::ForceVsCunning:			return SaveData.Ledger.ForceCunning;
	case ESEELedgerAxis::PresentVsFuture:			return SaveData.Ledger.PresentFuture;
	default:
		// Fallback — shouldn't happen
		return SaveData.Ledger.MercyPragmatism;
	}
}
