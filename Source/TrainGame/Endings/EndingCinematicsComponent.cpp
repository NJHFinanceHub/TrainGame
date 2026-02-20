// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EndingCinematicsComponent.h"
#include "EndingCalculatorSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UEndingCinematicsComponent::UEndingCinematicsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UEndingCinematicsComponent::BeginPlay()
{
	Super::BeginPlay();
}

// ---------------------------------------------------------------------------
// Cutscene Playback
// ---------------------------------------------------------------------------

void UEndingCinematicsComponent::PlayEndingSequence()
{
	UEndingCalculatorSubsystem* Calculator = GetEndingCalculator();
	if (!Calculator || !Calculator->HasEndingBeenTriggered()) return;

	const FEndingCinematicData CinematicData = Calculator->GetEndingCinematicData();

	bCutscenePlaying = true;
	OnCutsceneStarted.Broadcast(Calculator->GetTriggeredEnding());

	// Load the ending level sequence if set.
	// The actual playback is handled by Blueprint — this component fires the
	// delegate and provides the data. A Blueprint event handler should:
	// 1. Load CinematicData.CutsceneSequence
	// 2. Create a LevelSequencePlayer
	// 3. Play it
	// 4. Call OnCutsceneTimerExpired when done (or use the timer as fallback)

	// Set fallback timer for auto-transition to credits
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CutsceneTimerHandle,
			this,
			&UEndingCinematicsComponent::OnCutsceneTimerExpired,
			CinematicData.CutsceneDuration,
			false
		);
	}
}

void UEndingCinematicsComponent::SkipToCredits()
{
	if (!bCutscenePlaying) return;

	// Cancel the cutscene timer
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CutsceneTimerHandle);
	}

	OnCutsceneTimerExpired();
}

void UEndingCinematicsComponent::OnCutsceneTimerExpired()
{
	const UEndingCalculatorSubsystem* Calculator = GetEndingCalculator();
	const EEndingType Ending = Calculator ? Calculator->GetTriggeredEnding() : EEndingType::None;

	bCutscenePlaying = false;
	OnCutsceneFinished.Broadcast(Ending);

	StartCredits();
}

// ---------------------------------------------------------------------------
// Epilogue
// ---------------------------------------------------------------------------

TArray<FText> UEndingCinematicsComponent::BuildEpilogueSequence() const
{
	TArray<FText> Epilogue;

	const UEndingCalculatorSubsystem* Calculator = GetEndingCalculator();
	if (!Calculator) return Epilogue;

	const FEndingCinematicData CinematicData = Calculator->GetEndingCinematicData();

	// Main ending epilogue
	if (!CinematicData.EndingEpilogue.IsEmpty())
	{
		Epilogue.Add(CinematicData.EndingEpilogue);
	}

	// Companion fate epilogues
	const TArray<FCompanionEndingFate> Fates = Calculator->EvaluateCompanionFates();
	for (const FCompanionEndingFate& Fate : Fates)
	{
		// Look up companion-specific epilogue from cinematic data
		const FText* CompanionEpilogue = CinematicData.CompanionEpilogues.Find(Fate.CompanionID);
		if (CompanionEpilogue && !CompanionEpilogue->IsEmpty())
		{
			Epilogue.Add(*CompanionEpilogue);
		}
		else if (!Fate.EpilogueText.IsEmpty())
		{
			Epilogue.Add(Fate.EpilogueText);
		}
	}

	return Epilogue;
}

// ---------------------------------------------------------------------------
// Credits
// ---------------------------------------------------------------------------

ECreditsMusic UEndingCinematicsComponent::GetCreditsMusic() const
{
	const UEndingCalculatorSubsystem* Calculator = GetEndingCalculator();
	if (!Calculator) return ECreditsMusic::Default;

	return Calculator->GetEndingCinematicData().CreditsMusic;
}

void UEndingCinematicsComponent::StartCredits()
{
	if (bCreditsStarted) return;

	bCreditsStarted = true;
	OnCreditsStarted.Broadcast(GetCreditsMusic());

	// Blueprint handles the actual credits UI.
	// The OnCreditsStarted delegate tells the UI widget which music to play
	// and triggers the credits roll widget to appear.
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

UEndingCalculatorSubsystem* UEndingCinematicsComponent::GetEndingCalculator() const
{
	const UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	if (!GI) return nullptr;
	return GI->GetSubsystem<UEndingCalculatorSubsystem>();
}
