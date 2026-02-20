// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEEndingCinematicsSubsystem.h"
#include "SEEEndingCalculator.h"

void USEEEndingCinematicsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USEEEndingCalculator>();
}

void USEEEndingCinematicsSubsystem::BeginEndingSequence(ESEEEnding Ending)
{
	USEEEndingCalculator* Calculator = GetGameInstance()->GetSubsystem<USEEEndingCalculator>();
	if (!Calculator)
	{
		return;
	}

	ActiveEnding = Ending;
	ActiveVariation = Calculator->CalculateEndingVariation(Ending);
	ActiveCinematicData = Calculator->GetEndingCinematicData(Ending);
	CachedStats = Calculator->BuildEndgameStats();

	bCinematicActive = true;
	CurrentPhaseIndex = 0;
	CurrentPhase = GetPhaseSequence()[0];

	OnEndingPhaseStarted.Broadcast(CurrentPhase);
}

void USEEEndingCinematicsSubsystem::AdvancePhase()
{
	if (!bCinematicActive)
	{
		return;
	}

	// Complete current phase
	OnEndingPhaseCompleted.Broadcast(CurrentPhase);

	CurrentPhaseIndex++;
	const TArray<ESEEEndingCinematicPhase>& Sequence = GetPhaseSequence();

	if (CurrentPhaseIndex >= Sequence.Num())
	{
		// All phases complete
		bCinematicActive = false;
		OnEndingCinematicComplete.Broadcast();
		return;
	}

	CurrentPhase = Sequence[CurrentPhaseIndex];
	OnEndingPhaseStarted.Broadcast(CurrentPhase);

	// Special handling for Credits phase
	if (CurrentPhase == ESEEEndingCinematicPhase::Credits)
	{
		OnCreditsStarted.Broadcast(ActiveEnding);
		OnEndgameStatsReady.Broadcast(CachedStats);
	}
}

FText USEEEndingCinematicsSubsystem::GetWilfordDialogue() const
{
	const FText* Found = ActiveCinematicData.WilfordDialogue.Find(ActiveVariation);
	if (Found)
	{
		return *Found;
	}

	// Fallback to Bittersweet if exact variation not available
	Found = ActiveCinematicData.WilfordDialogue.Find(ESEEEndingVariation::Bittersweet);
	return Found ? *Found : FText::GetEmpty();
}

FText USEEEndingCinematicsSubsystem::GetEpilogueNarration() const
{
	const FText* Found = ActiveCinematicData.EpilogueNarration.Find(ActiveVariation);
	if (Found)
	{
		return *Found;
	}

	// Fallback to first available narration
	for (const auto& Pair : ActiveCinematicData.EpilogueNarration)
	{
		return Pair.Value;
	}
	return FText::GetEmpty();
}

FText USEEEndingCinematicsSubsystem::GetEndingTitle() const
{
	return ActiveCinematicData.EndingTitle;
}

FText USEEEndingCinematicsSubsystem::GetEndingSubtitle() const
{
	return ActiveCinematicData.EndingSubtitle;
}

TSoftObjectPtr<USoundBase> USEEEndingCinematicsSubsystem::GetCreditsMusic() const
{
	return ActiveCinematicData.CreditsMusic;
}

TArray<FName> USEEEndingCinematicsSubsystem::GetEndingCompanions() const
{
	return ActiveCinematicData.SurvivingCompanionsInEnding;
}

const TArray<ESEEEndingCinematicPhase>& USEEEndingCinematicsSubsystem::GetPhaseSequence()
{
	static const TArray<ESEEEndingCinematicPhase> Sequence = {
		ESEEEndingCinematicPhase::Confrontation,
		ESEEEndingCinematicPhase::Decision,
		ESEEEndingCinematicPhase::Consequence,
		ESEEEndingCinematicPhase::Epilogue,
		ESEEEndingCinematicPhase::Credits
	};
	return Sequence;
}
