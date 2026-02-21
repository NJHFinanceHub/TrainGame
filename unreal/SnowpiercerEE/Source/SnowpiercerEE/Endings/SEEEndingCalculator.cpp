// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEEndingCalculator.h"
#include "SEELedgerSubsystem.h"
#include "../SEEFactionManager.h"
#include "TrainGame/Companions/CompanionRosterSubsystem.h"
#include "TrainGame/Companions/CompanionComponent.h"
#include "../SEEQuestManager.h"

void USEEEndingCalculator::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Collection.InitializeDependency<USEELedgerSubsystem>();
	Collection.InitializeDependency<USEEFactionManager>();
	Collection.InitializeDependency<UCompanionRosterSubsystem>();

	InitializeEndingRequirements();
	InitializeCinematicData();
}

void USEEEndingCalculator::InitializeEndingRequirements()
{
	// =====================================================================
	// THE NEW WILFORD — Take the Engine, become the tyrant
	// High pragmatism, willingness to sacrifice, accumulation of power
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheNewWilford;
		Req.MinAxisScores.Add(ESEELedgerAxis::MercyVsPragmatism, -20); // Pragmatic side
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.DisplayPriority = 20;
		EndingRequirements.Add(Req);
	}

	// =====================================================================
	// THE REVOLUTION — Destroy class system, democracy on rails
	// Balanced mercy and force, popular support, truth-telling
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheRevolution;
		Req.MinAxisScores.Add(ESEELedgerAxis::IndividualVsCollective, -15); // Collective side
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.MinCompanionsSurvived = 3;
		Req.DisplayPriority = 10;
		EndingRequirements.Add(Req);
	}

	// =====================================================================
	// THE DERAILMENT — Destroy the Engine, face the cold
	// Desperation, loss of faith, nihilism
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheDerailment;
		Req.MaxAxisScores.Add(ESEELedgerAxis::MercyVsPragmatism, -10); // Deep pragmatism
		Req.MaxAxisScores.Add(ESEELedgerAxis::TruthVsStability, -10); // Low truth
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.MaxCompanionsSurvived = 5; // Many companions lost
		Req.DisplayPriority = 50;
		EndingRequirements.Add(Req);
	}

	// =====================================================================
	// THE EXODUS — Open the front door, lead survivors out
	// Hope, evidence of thawing world, trust in the outside
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheExodus;
		Req.MinAxisScores.Add(ESEELedgerAxis::PresentVsFuture, -20); // Future-oriented
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.RequiredFlags.Add(FName("Thaw_Evidence_Found"));
		Req.DisplayPriority = 30;
		EndingRequirements.Add(Req);
	}

	// =====================================================================
	// THE ETERNAL LOOP — Discover truth, choose silence or revelation
	// Pursuit of truth, discovery of circular route
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheEternalLoop;
		Req.MinAxisScores.Add(ESEELedgerAxis::TruthVsStability, 25); // High truth
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.RequiredFlags.Add(FName("Gilliam_Truth_Revealed"));
		Req.MinCollectiblesFound = 50;
		Req.DisplayPriority = 40;
		EndingRequirements.Add(Req);
	}

	// =====================================================================
	// THE BRIDGE (Secret) — Contact another train
	// Maximum exploration, all hidden intel, outside contact
	// =====================================================================
	{
		FSEEEndingRequirement Req;
		Req.Ending = ESEEEnding::TheBridge;
		Req.MinAxisScores.Add(ESEELedgerAxis::MercyVsPragmatism, 15);   // Merciful
		Req.MinAxisScores.Add(ESEELedgerAxis::TruthVsStability, 15);    // Truthful
		Req.MinAxisScores.Add(ESEELedgerAxis::PresentVsFuture, -15);    // Future-oriented
		Req.RequiredFlags.Add(FName("Reached_Engine"));
		Req.RequiredFlags.Add(FName("Thaw_Evidence_Found"));
		Req.RequiredFlags.Add(FName("Radio_Contact_Made"));
		Req.RequiredFlags.Add(FName("Nix_Radio_Used"));
		Req.RequiredFlags.Add(FName("Nix_Two_Comm_Used"));
		Req.RequiredFlags.Add(FName("All_Listening_Post_Intel"));
		Req.MinCompanionsSurvived = 6;
		Req.MinCollectiblesFound = 100;
		Req.DisplayPriority = 60;
		EndingRequirements.Add(Req);
	}
}

void USEEEndingCalculator::InitializeCinematicData()
{
	// THE NEW WILFORD
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheNewWilford;
		Data.EndingTitle = FText::FromString(TEXT("The New Wilford"));
		Data.EndingSubtitle = FText::FromString(TEXT("The train endures. Under new management."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("You understand now. The weight of it. The necessity. Welcome to the Engine.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("I'd say you'll do better, but I've heard that before.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("Look at you. You became me. Was it worth it?")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("You're worse than I ever was. At least I felt guilty.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("The train runs. The system persists, refined by someone who walked its length. Perhaps this time, the compromises are smaller. Perhaps.")));
		Data.EpilogueNarration.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("The train runs. The names change. The suffering does not.")));

		CinematicDataMap.Add(ESEEEnding::TheNewWilford, Data);
	}

	// THE REVOLUTION
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheRevolution;
		Data.EndingTitle = FText::FromString(TEXT("The Revolution"));
		Data.EndingSubtitle = FText::FromString(TEXT("The class system falls. The train is everyone's."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("You did what no one else could. I hope they're worth it.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("Democracy on a train. I give it three years.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("You burned it all down. Now what?")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("Congratulations. You've traded one tyranny for another. At least mine worked.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("The Council governs. Not perfectly — never perfectly — but openly. The Tail eats the same food as the Engine. Children play in cars they were never meant to see.")));
		Data.EpilogueNarration.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("The mob rules. What replaced the system is chaos wearing the mask of freedom.")));

		CinematicDataMap.Add(ESEEEnding::TheRevolution, Data);
	}

	// THE DERAILMENT
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheDerailment;
		Data.EndingTitle = FText::FromString(TEXT("The Derailment"));
		Data.EndingSubtitle = FText::FromString(TEXT("The Engine dies. The cold comes."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("...")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("You're killing everyone. You know that.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("None of us deserved this. Not the suffering. Not the survival. Not any of it.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("Then we die. Together, at least.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("The Engine goes silent. The train slows. What comes next is unclear — but the machine that demanded so much suffering is finally, mercifully, still.")));
		Data.EpilogueNarration.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("Silence. Cold. The experiment ends.")));

		CinematicDataMap.Add(ESEEEnding::TheDerailment, Data);
	}

	// THE EXODUS
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheExodus;
		Data.EndingTitle = FText::FromString(TEXT("The Exodus"));
		Data.EndingSubtitle = FText::FromString(TEXT("The doors open. The world is cold, but it is the world."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("I... I never checked. I never looked outside. What if you're right?")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("You're gambling with 2,800 lives on temperature readings and hope.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("Some will die out there. You're trading certain suffering for uncertain survival.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("Open the doors. Kill them faster than I ever did.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("They walk into the cold. Not all of them survive — but those who do see the sun for the first time in seventeen years. The snow is melting.")));
		Data.EpilogueNarration.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("The doors open. The cold rushes in. Some walk out. Not all of them come back.")));

		CinematicDataMap.Add(ESEEEnding::TheExodus, Data);
	}

	// THE ETERNAL LOOP
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheEternalLoop;
		Data.EndingTitle = FText::FromString(TEXT("The Eternal Loop"));
		Data.EndingSubtitle = FText::FromString(TEXT("The truth exists now. What comes next is up to them."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("Honesty. Finally. You don't know either. But at least you documented everything.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("You walked 103 cars and found... questions. How very human.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Bittersweet,
			FText::FromString(TEXT("So the train keeps running. Nothing changes. But everyone knows.")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Dark,
			FText::FromString(TEXT("You saw everything and did nothing. That's not wisdom. That's cowardice.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("The train continues. The truth is archived. Future generations will have what this one lacked: the complete record. Whether they do better is their choice to make.")));

		CinematicDataMap.Add(ESEEEnding::TheEternalLoop, Data);
	}

	// THE BRIDGE (Secret)
	{
		FSEEEndingCinematicData Data;
		Data.Ending = ESEEEnding::TheBridge;
		Data.EndingTitle = FText::FromString(TEXT("The Bridge"));
		Data.EndingSubtitle = FText::FromString(TEXT("We are not alone."));

		Data.WilfordDialogue.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("Another train. I... there's another train. All this time...")));
		Data.WilfordDialogue.Add(ESEEEndingVariation::Good,
			FText::FromString(TEXT("Connection. Not exile, not isolation. I never considered that.")));

		Data.EpilogueNarration.Add(ESEEEndingVariation::Ideal,
			FText::FromString(TEXT("The signal is answered. Across the frozen waste, another engine hums. The bridge between them — fragile, impossible, necessary — begins to form. Humanity is no longer a single train. It is a network.")));

		CinematicDataMap.Add(ESEEEnding::TheBridge, Data);
	}
}

TArray<FSEEEndingResult> USEEEndingCalculator::CalculateAvailableEndings()
{
	TArray<FSEEEndingResult> Results;

	for (const FSEEEndingRequirement& Req : EndingRequirements)
	{
		Results.Add(EvaluateEnding(Req));
	}

	// Sort by affinity (highest first), then priority
	Results.Sort([](const FSEEEndingResult& A, const FSEEEndingResult& B)
	{
		if (A.bUnlocked != B.bUnlocked) return A.bUnlocked;
		return A.Affinity > B.Affinity;
	});

	return Results;
}

FSEEEndingResult USEEEndingCalculator::GetRecommendedEnding()
{
	TArray<FSEEEndingResult> Available = CalculateAvailableEndings();

	for (const FSEEEndingResult& Result : Available)
	{
		if (Result.bUnlocked)
		{
			return Result;
		}
	}

	// Fallback: The Eternal Loop is always available if the player reached the Engine
	FSEEEndingResult Fallback;
	Fallback.Ending = ESEEEnding::TheEternalLoop;
	Fallback.Variation = ESEEEndingVariation::Bittersweet;
	Fallback.Affinity = 0.1f;
	Fallback.bUnlocked = true;
	return Fallback;
}

bool USEEEndingCalculator::IsEndingAvailable(ESEEEnding Ending)
{
	for (const FSEEEndingRequirement& Req : EndingRequirements)
	{
		if (Req.Ending == Ending)
		{
			FSEEEndingResult Result = EvaluateEnding(Req);
			return Result.bUnlocked;
		}
	}
	return false;
}

ESEEEndingVariation USEEEndingCalculator::CalculateEndingVariation(ESEEEnding Ending)
{
	USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
	if (!Ledger)
	{
		return ESEEEndingVariation::Bittersweet;
	}

	FSEELedgerSnapshot Snapshot = Ledger->GetLedgerSnapshot();
	float Affinity = CalculateAffinity(Ending, Snapshot);
	int32 CompanionsSurvived = GetSurvivingCompanionCount();

	// Variation is determined by affinity + companion survival
	float VariationScore = Affinity * 0.7f + (CompanionsSurvived / 12.0f) * 0.3f;

	if (VariationScore >= 0.75f) return ESEEEndingVariation::Ideal;
	if (VariationScore >= 0.50f) return ESEEEndingVariation::Good;
	if (VariationScore >= 0.25f) return ESEEEndingVariation::Bittersweet;
	return ESEEEndingVariation::Dark;
}

FSEEEndingResult USEEEndingCalculator::SelectEnding(ESEEEnding Ending)
{
	SelectedEnding = Ending;

	FSEEEndingResult Result;
	Result.Ending = Ending;
	Result.bUnlocked = true;
	Result.Variation = CalculateEndingVariation(Ending);

	USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
	if (Ledger)
	{
		Result.Affinity = CalculateAffinity(Ending, Ledger->GetLedgerSnapshot());
	}

	OnEndingSelected.Broadcast(Result);

	return Result;
}

FSEEEndgameStats USEEEndingCalculator::BuildEndgameStats()
{
	FSEEEndgameStats Stats;
	Stats.EndingChosen = SelectedEnding;
	Stats.Variation = CalculateEndingVariation(SelectedEnding);

	USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
	if (Ledger)
	{
		Stats.FinalLedger = Ledger->GetLedgerSnapshot();
		Stats.MoralChoicesMade = Ledger->GetTotalChoicesMade();
		Stats.EnemiesKilled = Ledger->GetEnemiesKilled();
		Stats.EnemiesSpared = Ledger->GetEnemiesSpared();
	}

	UCompanionRosterSubsystem* Roster = GetGameInstance()->GetSubsystem<UCompanionRosterSubsystem>();
	if (Roster)
	{
		Stats.CompanionsRecruited = Roster->GetFullRoster().Num();
		Stats.CompanionsLost = Roster->GetDeadCompanions().Num();
	}

	USEEQuestManager* QuestMgr = GetGameInstance()->GetSubsystem<USEEQuestManager>();
	if (QuestMgr)
	{
		Stats.QuestsCompleted = QuestMgr->GetCompletedQuests().Num();
	}

	return Stats;
}

FSEEEndingCinematicData USEEEndingCalculator::GetEndingCinematicData(ESEEEnding Ending) const
{
	const FSEEEndingCinematicData* Found = CinematicDataMap.Find(Ending);
	if (Found)
	{
		FSEEEndingCinematicData Data = *Found;
		// Populate surviving companions at runtime
		Data.SurvivingCompanionsInEnding = const_cast<USEEEndingCalculator*>(this)->GetSurvivingCompanionNames();
		return Data;
	}
	return FSEEEndingCinematicData();
}

FSEEEndingResult USEEEndingCalculator::EvaluateEnding(const FSEEEndingRequirement& Requirement)
{
	FSEEEndingResult Result;
	Result.Ending = Requirement.Ending;
	Result.bUnlocked = true;

	USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>();
	if (!Ledger)
	{
		Result.bUnlocked = false;
		Result.UnmetRequirements.Add(FName("LedgerSubsystem_Missing"));
		return Result;
	}

	FSEELedgerSnapshot Snapshot = Ledger->GetLedgerSnapshot();

	// Check minimum axis scores
	for (const auto& Pair : Requirement.MinAxisScores)
	{
		int32 Current = Snapshot.GetAxisScore(Pair.Key);
		if (Current < Pair.Value)
		{
			Result.bUnlocked = false;
			Result.UnmetRequirements.Add(FName(*FString::Printf(TEXT("Axis_%d_Below_%d"), (int32)Pair.Key, Pair.Value)));
		}
	}

	// Check maximum axis scores (for endings requiring low scores)
	for (const auto& Pair : Requirement.MaxAxisScores)
	{
		int32 Current = Snapshot.GetAxisScore(Pair.Key);
		if (Current > Pair.Value)
		{
			Result.bUnlocked = false;
			Result.UnmetRequirements.Add(FName(*FString::Printf(TEXT("Axis_%d_Above_%d"), (int32)Pair.Key, Pair.Value)));
		}
	}

	// Check required flags
	for (const FName& Flag : Requirement.RequiredFlags)
	{
		if (!Ledger->GetGlobalFlag(Flag))
		{
			Result.bUnlocked = false;
			Result.UnmetRequirements.Add(Flag);
		}
	}

	// Check blocking flags
	for (const FName& Flag : Requirement.BlockingFlags)
	{
		if (Ledger->GetGlobalFlag(Flag))
		{
			Result.bUnlocked = false;
			Result.UnmetRequirements.Add(FName(*FString::Printf(TEXT("Blocked_%s"), *Flag.ToString())));
		}
	}

	// Check companion survival
	int32 SurvivingCount = GetSurvivingCompanionCount();
	if (SurvivingCount < Requirement.MinCompanionsSurvived)
	{
		Result.bUnlocked = false;
		Result.UnmetRequirements.Add(FName("Not_Enough_Companions"));
	}
	if (SurvivingCount > Requirement.MaxCompanionsSurvived)
	{
		Result.bUnlocked = false;
		Result.UnmetRequirements.Add(FName("Too_Many_Companions_Survived"));
	}

	// Check collectibles (for secret ending)
	if (Requirement.MinCollectiblesFound > 0)
	{
		int32 CollectiblesFound = Ledger->GetGlobalIntFlag(FName("Collectibles_Found"));
		if (CollectiblesFound < Requirement.MinCollectiblesFound)
		{
			Result.bUnlocked = false;
			Result.UnmetRequirements.Add(FName("Not_Enough_Collectibles"));
		}
	}

	// Calculate affinity and variation
	Result.Affinity = CalculateAffinity(Requirement.Ending, Snapshot);
	Result.Variation = CalculateEndingVariation(Requirement.Ending);

	return Result;
}

float USEEEndingCalculator::CalculateAffinity(ESEEEnding Ending, const FSEELedgerSnapshot& Ledger)
{
	// Each ending has an ideal Ledger profile. Affinity measures how close
	// the player's actual profile is to the ideal.

	float Score = 0.0f;

	switch (Ending)
	{
	case ESEEEnding::TheNewWilford:
		// High pragmatism, high cunning, moderate force
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.ForceVsCunning) * 0.5f;
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.TruthVsStability) * 0.3f;
		Score /= 1.8f;
		break;

	case ESEEEnding::TheRevolution:
		// Balanced mercy, high collective, high truth
		Score += FMath::GetMappedRangeValueClamped(FVector2D(-50, 50), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.IndividualVsCollective);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.TruthVsStability);
		Score /= 3.0f;
		break;

	case ESEEEnding::TheDerailment:
		// High force, low mercy, low stability, loss
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.ForceVsCunning);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.TruthVsStability);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.PresentVsFuture);
		Score /= 4.0f;
		break;

	case ESEEEnding::TheExodus:
		// High future, high mercy, evidence-based
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.PresentVsFuture);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.TruthVsStability) * 0.5f;
		Score /= 2.5f;
		break;

	case ESEEEnding::TheEternalLoop:
		// Maximum truth, high collectibles, documentation-focused
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.TruthVsStability);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(-50, 50), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism) * 0.3f;
		Score += FMath::GetMappedRangeValueClamped(FVector2D(-50, 50), FVector2D(0, 1), (float)Ledger.ForceVsCunning) * 0.3f;
		Score /= 1.6f;
		break;

	case ESEEEnding::TheBridge:
		// High mercy, high truth, high future — the most demanding profile
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.MercyVsPragmatism);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, 100), FVector2D(0, 1), (float)Ledger.TruthVsStability);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(0, -100), FVector2D(0, 1), (float)Ledger.PresentVsFuture);
		Score += FMath::GetMappedRangeValueClamped(FVector2D(-100, 100), FVector2D(0, 1), (float)Ledger.IndividualVsCollective) * 0.5f;
		Score += FMath::GetMappedRangeValueClamped(FVector2D(-100, 100), FVector2D(0, 1), (float)Ledger.ForceVsCunning) * 0.5f;
		Score /= 4.0f;
		break;

	default:
		break;
	}

	return FMath::Clamp(Score, 0.0f, 1.0f);
}

int32 USEEEndingCalculator::GetSurvivingCompanionCount() const
{
	UCompanionRosterSubsystem* Roster = GetGameInstance()->GetSubsystem<UCompanionRosterSubsystem>();
	if (!Roster)
	{
		return 0;
	}

	int32 Total = Roster->GetFullRoster().Num();
	int32 Dead = Roster->GetDeadCompanions().Num();
	return Total - Dead;
}

TArray<FName> USEEEndingCalculator::GetSurvivingCompanionNames() const
{
	TArray<FName> Names;
	UCompanionRosterSubsystem* Roster = GetGameInstance()->GetSubsystem<UCompanionRosterSubsystem>();
	if (!Roster)
	{
		return Names;
	}

	TArray<UCompanionComponent*> Dead = Roster->GetDeadCompanions();
	TSet<UCompanionComponent*> DeadSet(Dead);

	for (UCompanionComponent* Comp : Roster->GetFullRoster())
	{
		if (Comp && !DeadSet.Contains(Comp))
		{
			Names.Add(Comp->GetFName());
		}
	}

	return Names;
}
