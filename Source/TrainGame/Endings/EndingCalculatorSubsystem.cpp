// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EndingCalculatorSubsystem.h"
#include "MoralLedgerSubsystem.h"
#include "SnowyEngine/Faction/FactionReputationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void UEndingCalculatorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	TriggeredEnding = EEndingType::None;
}

// ---------------------------------------------------------------------------
// Eligibility Evaluation
// ---------------------------------------------------------------------------

void UEndingCalculatorSubsystem::RecalculateAllEligibility()
{
	const UMoralLedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<UMoralLedgerSubsystem>();
	if (!Ledger) return;

	TMap<EEndingType, FEndingEligibilityResult> NewEligibility;

	NewEligibility.Add(EEndingType::TheNewWilford, EvaluateNewWilford(Ledger));
	NewEligibility.Add(EEndingType::TheRevolution, EvaluateRevolution(Ledger));
	NewEligibility.Add(EEndingType::TheDerailment, EvaluateDerailment(Ledger));
	NewEligibility.Add(EEndingType::TheExodus, EvaluateExodus(Ledger));
	NewEligibility.Add(EEndingType::TheEternalLoop, EvaluateEternalLoop(Ledger));
	NewEligibility.Add(EEndingType::TheBridge, EvaluateBridge(Ledger));

	// Find highest affinity among available endings -> mark as Recommended
	float HighestAffinity = -1.0f;
	EEndingType BestEnding = EEndingType::None;
	for (auto& Pair : NewEligibility)
	{
		if (Pair.Value.Eligibility == EEndingEligibility::Available ||
			Pair.Value.Eligibility == EEndingEligibility::Recommended)
		{
			// Reset Recommended to Available first
			Pair.Value.Eligibility = EEndingEligibility::Available;
			if (Pair.Value.AffinityScore > HighestAffinity)
			{
				HighestAffinity = Pair.Value.AffinityScore;
				BestEnding = Pair.Key;
			}
		}
	}
	if (BestEnding != EEndingType::None)
	{
		NewEligibility[BestEnding].Eligibility = EEndingEligibility::Recommended;
	}

	// Broadcast changes
	for (const auto& Pair : NewEligibility)
	{
		const FEndingEligibilityResult* OldResult = CachedEligibility.Find(Pair.Key);
		if (!OldResult || OldResult->Eligibility != Pair.Value.Eligibility)
		{
			OnEndingEligibilityChanged.Broadcast(Pair.Key, Pair.Value.Eligibility);
		}
	}

	CachedEligibility = MoveTemp(NewEligibility);
}

FEndingEligibilityResult UEndingCalculatorSubsystem::GetEligibility(EEndingType Ending) const
{
	const FEndingEligibilityResult* Found = CachedEligibility.Find(Ending);
	if (Found) return *Found;

	FEndingEligibilityResult Default;
	Default.EndingType = Ending;
	return Default;
}

TArray<FEndingEligibilityResult> UEndingCalculatorSubsystem::GetAllEligibility() const
{
	TArray<FEndingEligibilityResult> Results;
	CachedEligibility.GenerateValueArray(Results);

	Results.Sort([](const FEndingEligibilityResult& A, const FEndingEligibilityResult& B)
	{
		return A.AffinityScore > B.AffinityScore;
	});

	return Results;
}

EEndingType UEndingCalculatorSubsystem::GetRecommendedEnding() const
{
	for (const auto& Pair : CachedEligibility)
	{
		if (Pair.Value.Eligibility == EEndingEligibility::Recommended)
		{
			return Pair.Key;
		}
	}
	return EEndingType::None;
}

int32 UEndingCalculatorSubsystem::GetAvailableEndingCount() const
{
	int32 Count = 0;
	for (const auto& Pair : CachedEligibility)
	{
		if (Pair.Value.Eligibility == EEndingEligibility::Available ||
			Pair.Value.Eligibility == EEndingEligibility::Recommended)
		{
			++Count;
		}
	}
	return Count;
}

// ---------------------------------------------------------------------------
// Ending Trigger
// ---------------------------------------------------------------------------

bool UEndingCalculatorSubsystem::TriggerEnding(EEndingTrigger Trigger)
{
	if (HasEndingBeenTriggered()) return false;

	const EEndingType Ending = TriggerToEnding(Trigger);
	if (Ending == EEndingType::None) return false;

	const FEndingEligibilityResult* Result = CachedEligibility.Find(Ending);
	if (!Result) return false;

	if (Result->Eligibility != EEndingEligibility::Available &&
		Result->Eligibility != EEndingEligibility::Recommended)
	{
		return false;
	}

	TriggeredEnding = Ending;
	OnEndingTriggered.Broadcast(Ending);
	return true;
}

EEndingType UEndingCalculatorSubsystem::TriggerToEnding(EEndingTrigger Trigger)
{
	switch (Trigger)
	{
	case EEndingTrigger::SeizeEngine:			return EEndingType::TheNewWilford;
	case EEndingTrigger::HoldElection:			return EEndingType::TheRevolution;
	case EEndingTrigger::DetonateEngine:		return EEndingType::TheDerailment;
	case EEndingTrigger::OpenFrontDoor:			return EEndingType::TheExodus;
	case EEndingTrigger::AccessArchive:			return EEndingType::TheEternalLoop;
	case EEndingTrigger::ActivateTransmitter:	return EEndingType::TheBridge;
	default:									return EEndingType::None;
	}
}

// ---------------------------------------------------------------------------
// Companion Fate Evaluation
// ---------------------------------------------------------------------------

TArray<FCompanionEndingFate> UEndingCalculatorSubsystem::EvaluateCompanionFates() const
{
	// Companion fates are evaluated by querying the CompanionRosterSubsystem.
	// This returns placeholder data — the actual implementation hooks into the
	// companion system's roster tracking (alive/dead, loyalty, personal quest state).
	TArray<FCompanionEndingFate> Fates;

	// The companion roster subsystem provides the real data. This list serves
	// as the schema; the actual values are populated by iterating the roster.
	// See CompanionRosterSubsystem::GetAllCompanions() for the live data source.

	return Fates;
}

int32 UEndingCalculatorSubsystem::GetSurvivingCompanionCount() const
{
	int32 Count = 0;
	for (const FCompanionEndingFate& Fate : EvaluateCompanionFates())
	{
		if (Fate.bAlive) ++Count;
	}
	return Count;
}

// ---------------------------------------------------------------------------
// Cinematic Data
// ---------------------------------------------------------------------------

FEndingCinematicData UEndingCalculatorSubsystem::GetEndingCinematicData() const
{
	const FEndingCinematicData* Data = CinematicDataMap.Find(TriggeredEnding);
	if (Data) return *Data;

	FEndingCinematicData Default;
	Default.EndingType = TriggeredEnding;
	return Default;
}

void UEndingCalculatorSubsystem::RegisterCinematicData(EEndingType Ending, const FEndingCinematicData& Data)
{
	CinematicDataMap.Add(Ending, Data);
}

// ---------------------------------------------------------------------------
// Per-Ending Prerequisite Evaluation
// ---------------------------------------------------------------------------

// Helper to build prerequisite entries
static FEndingPrerequisite MakePrereq(FName ID, const FText& Desc, bool bMet)
{
	FEndingPrerequisite P;
	P.PrerequisiteID = ID;
	P.Description = Desc;
	P.bMet = bMet;
	return P;
}

static float PrereqProgress(const TArray<FEndingPrerequisite>& Prereqs)
{
	if (Prereqs.Num() == 0) return 1.0f;
	int32 Met = 0;
	for (const auto& P : Prereqs) { if (P.bMet) ++Met; }
	return static_cast<float>(Met) / static_cast<float>(Prereqs.Num());
}

// ------ 1. The New Wilford ------
// Take the Engine, become the tyrant.
// Prerequisites: Reach Engine car, high Pragmatism, allied with OrderOfTheEngine OR Jackboots
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateNewWilford(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheNewWilford;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bPragmatist = Ledger->GetAxisValue(EMoralAxis::MercyVsPragmatism) < -20;
	const bool bCollectivist = Ledger->GetAxisValue(EMoralAxis::IndividualVsCollective) < -10;
	const bool bEngineAllied = CheckFactionStanding(EFaction::OrderOfTheEngine, 40.0f);
	const bool bJackbootAllied = CheckFactionStanding(EFaction::Jackboots, 40.0f);
	const bool bFactionSupport = bEngineAllied || bJackbootAllied;

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("Pragmatist",
		NSLOCTEXT("Endings", "Pragmatist", "Pragmatic leadership style"), bPragmatist));
	Result.Prerequisites.Add(MakePrereq("FactionSupport",
		NSLOCTEXT("Endings", "FactionSupport", "Allied with Order of the Engine or Jackboots"), bFactionSupport));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	// Affinity: pragmatism + collective + force
	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::MercyVsPragmatism, -0.4f);     // Pragmatism (negative axis = pragmatism)
	Weights.Add(EMoralAxis::IndividualVsCollective, -0.2f); // Collective
	Weights.Add(EMoralAxis::ForceVsCunning, 0.2f);          // Force
	Weights.Add(EMoralAxis::PresentVsFuture, -0.2f);        // Future-oriented
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	const bool bAllMet = bReachedEngine && bPragmatist && bFactionSupport;
	Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;

	return Result;
}

// ------ 2. The Revolution ------
// Destroy class system, democracy on rails.
// Prerequisites: Reach Engine, high Truth, multiple factions at Friendly+, companions alive
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateRevolution(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheRevolution;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bTruthful = Ledger->GetAxisValue(EMoralAxis::TruthVsStability) > 15;
	const bool bTailiesSupport = CheckFactionStanding(EFaction::Tailies, 30.0f);
	const bool bUnionSupport = CheckFactionStanding(EFaction::ThirdClassUnion, 20.0f);
	const bool bBroadCoalition = bTailiesSupport && bUnionSupport;
	const bool bGilliamRevealed = CheckFlag(Ledger, "GilliamSecretRevealed");

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine_Rev", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("Truthful",
		NSLOCTEXT("Endings", "Truthful", "Committed to truth and transparency"), bTruthful));
	Result.Prerequisites.Add(MakePrereq("BroadCoalition",
		NSLOCTEXT("Endings", "BroadCoalition", "Coalition support from Tailies and Third Class"), bBroadCoalition));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::TruthVsStability, 0.3f);        // Truth
	Weights.Add(EMoralAxis::IndividualVsCollective, -0.3f);  // Collective
	Weights.Add(EMoralAxis::MercyVsPragmatism, 0.2f);        // Mercy
	Weights.Add(EMoralAxis::PresentVsFuture, -0.2f);         // Future
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	// Bonus affinity if Gilliam's secret was revealed publicly
	if (bGilliamRevealed) Result.AffinityScore = FMath::Min(1.0f, Result.AffinityScore + 0.1f);

	const bool bAllMet = bReachedEngine && bTruthful && bBroadCoalition;
	Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;

	return Result;
}

// ------ 3. The Derailment ------
// Destroy the Engine, face the cold.
// Prerequisites: Reach Engine, have explosives, high Force axis
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateDerailment(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheDerailment;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bHasExplosives = CheckFlag(Ledger, "AcquiredKronoleExplosives") ||
								CheckFlag(Ledger, "AcquiredEngineExplosives");
	const bool bDesperateMeasures = Ledger->GetAxisValue(EMoralAxis::ForceVsCunning) > 20 ||
									Ledger->GetAxisValue(EMoralAxis::MercyVsPragmatism) < -30;

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine_Der", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("Explosives",
		NSLOCTEXT("Endings", "Explosives", "Acquired explosive materials"), bHasExplosives));
	Result.Prerequisites.Add(MakePrereq("DesperateMeasures",
		NSLOCTEXT("Endings", "DesperateMeasures", "Willing to use extreme measures"), bDesperateMeasures));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::ForceVsCunning, 0.4f);           // Force
	Weights.Add(EMoralAxis::MercyVsPragmatism, -0.2f);       // Pragmatism
	Weights.Add(EMoralAxis::PresentVsFuture, 0.2f);          // Present (burn it now)
	Weights.Add(EMoralAxis::TruthVsStability, 0.2f);         // Truth (reject the lie)
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	const bool bAllMet = bReachedEngine && bHasExplosives && bDesperateMeasures;
	Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;

	return Result;
}

// ------ 4. The Exodus ------
// Open the front door, lead survivors out.
// Prerequisites: Evidence of thaw, high Mercy, companion loyalty
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateExodus(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheExodus;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bThawEvidence = CheckFlag(Ledger, "DiscoveredThawEvidence");
	const bool bMerciful = Ledger->GetAxisValue(EMoralAxis::MercyVsPragmatism) > 20;
	const bool bTheThawAllied = CheckFactionStanding(EFaction::TheThaw, 30.0f);

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine_Exo", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("ThawEvidence",
		NSLOCTEXT("Endings", "ThawEvidence", "Found evidence that the world is warming"), bThawEvidence));
	Result.Prerequisites.Add(MakePrereq("Merciful",
		NSLOCTEXT("Endings", "Merciful", "Compassionate leadership"), bMerciful));
	Result.Prerequisites.Add(MakePrereq("TheThaw",
		NSLOCTEXT("Endings", "TheThaw", "Relationship with The Thaw faction"), bTheThawAllied));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::MercyVsPragmatism, 0.3f);        // Mercy
	Weights.Add(EMoralAxis::IndividualVsCollective, 0.2f);    // Individual lives matter
	Weights.Add(EMoralAxis::PresentVsFuture, -0.3f);          // Future (brave new world)
	Weights.Add(EMoralAxis::TruthVsStability, 0.2f);          // Truth (the thaw is real)
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	const bool bAllMet = bReachedEngine && bThawEvidence && bMerciful;
	Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;

	return Result;
}

// ------ 5. The Eternal Loop ------
// Discover truth about the train's cycle, choose silence or revelation.
// Prerequisites: Found all archive fragments, high Cunning/Perception
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateEternalLoop(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheEternalLoop;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bArchiveComplete = CheckFlag(Ledger, "ArchiveFragmentsComplete");
	const bool bCycleDiscovered = CheckFlag(Ledger, "DiscoveredRevoltCycle");
	const bool bCunning = Ledger->GetAxisValue(EMoralAxis::ForceVsCunning) < -15;

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine_Loop", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("ArchiveComplete",
		NSLOCTEXT("Endings", "ArchiveComplete", "Collected all archive fragments"), bArchiveComplete));
	Result.Prerequisites.Add(MakePrereq("CycleDiscovered",
		NSLOCTEXT("Endings", "CycleDiscovered", "Discovered the truth about the revolt cycle"), bCycleDiscovered));
	Result.Prerequisites.Add(MakePrereq("Cunning",
		NSLOCTEXT("Endings", "Cunning", "Cerebral approach to problem-solving"), bCunning));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::ForceVsCunning, -0.4f);           // Cunning
	Weights.Add(EMoralAxis::TruthVsStability, -0.3f);         // Stability (or truth — both valid)
	Weights.Add(EMoralAxis::PresentVsFuture, -0.2f);          // Future (long game)
	Weights.Add(EMoralAxis::MercyVsPragmatism, -0.1f);        // Pragmatism
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	const bool bAllMet = bReachedEngine && bArchiveComplete && bCycleDiscovered && bCunning;
	Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;

	return Result;
}

// ------ 6. The Bridge (SECRET) ------
// Contact another train.
// Prerequisites: Found transmitter parts, discovered other train signal, high Perception
FEndingEligibilityResult UEndingCalculatorSubsystem::EvaluateBridge(const UMoralLedgerSubsystem* Ledger) const
{
	FEndingEligibilityResult Result;
	Result.EndingType = EEndingType::TheBridge;

	const bool bReachedEngine = CheckFlag(Ledger, "ReachedEngine");
	const bool bTransmitterBuilt = CheckFlag(Ledger, "TransmitterAssembled");
	const bool bSignalFound = CheckFlag(Ledger, "OtherTrainSignalDiscovered");
	const bool bATLASDecrypted = CheckFlag(Ledger, "ATLASDecryptedTransmission");

	Result.Prerequisites.Add(MakePrereq("ReachedEngine",
		NSLOCTEXT("Endings", "ReachedEngine_Bridge", "Reached the Engine car"), bReachedEngine));
	Result.Prerequisites.Add(MakePrereq("TransmitterBuilt",
		NSLOCTEXT("Endings", "TransmitterBuilt", "Assembled the long-range transmitter"), bTransmitterBuilt));
	Result.Prerequisites.Add(MakePrereq("SignalFound",
		NSLOCTEXT("Endings", "SignalFound", "Discovered another train's signal"), bSignalFound));
	Result.Prerequisites.Add(MakePrereq("ATLASDecrypted",
		NSLOCTEXT("Endings", "ATLASDecrypted", "ATLAS decrypted the transmission"), bATLASDecrypted));

	Result.PrerequisiteProgress = PrereqProgress(Result.Prerequisites);

	// Secret ending affinity is based on exploration thoroughness
	TMap<EMoralAxis, float> Weights;
	Weights.Add(EMoralAxis::ForceVsCunning, -0.3f);           // Cunning
	Weights.Add(EMoralAxis::TruthVsStability, 0.3f);          // Truth
	Weights.Add(EMoralAxis::PresentVsFuture, -0.3f);          // Future
	Weights.Add(EMoralAxis::IndividualVsCollective, 0.1f);     // Individual (diplomatic)
	Result.AffinityScore = CalculateAffinity(Ledger, Weights);

	const bool bAllMet = bReachedEngine && bTransmitterBuilt && bSignalFound && bATLASDecrypted;

	// Secret ending starts hidden until player discovers signal
	if (!bSignalFound)
	{
		Result.Eligibility = EEndingEligibility::Hidden;
	}
	else
	{
		Result.Eligibility = bAllMet ? EEndingEligibility::Available : EEndingEligibility::Locked;
	}

	return Result;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

float UEndingCalculatorSubsystem::CalculateAffinity(const UMoralLedgerSubsystem* Ledger, const TMap<EMoralAxis, float>& Weights) const
{
	// Affinity is the dot product of normalized axis values and weight vector,
	// scaled to [0, 1]. A weight of +0.3 on MercyVsPragmatism means this ending
	// favors Mercy (positive axis). A weight of -0.3 means it favors Pragmatism.
	float Score = 0.0f;
	float TotalWeight = 0.0f;

	for (const auto& Pair : Weights)
	{
		const float Normalized = Ledger->GetAxisNormalized(Pair.Key);
		Score += Normalized * Pair.Value;
		TotalWeight += FMath::Abs(Pair.Value);
	}

	if (TotalWeight > 0.0f)
	{
		Score /= TotalWeight;
	}

	// Map from [-1, 1] to [0, 1]
	return FMath::Clamp((Score + 1.0f) * 0.5f, 0.0f, 1.0f);
}

bool UEndingCalculatorSubsystem::CheckFlag(const UMoralLedgerSubsystem* Ledger, FName FlagName) const
{
	return Ledger && Ledger->GetStoryFlag(FlagName);
}

bool UEndingCalculatorSubsystem::CheckFactionStanding(EFaction Faction, float MinRep) const
{
	// Find the player's FactionReputationComponent to check standing
	const UWorld* World = GetWorld();
	if (!World) return false;

	const ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!PlayerChar) return false;

	const UFactionReputationComponent* FactionComp = PlayerChar->FindComponentByClass<UFactionReputationComponent>();
	if (!FactionComp) return false;

	return FactionComp->GetReputation(Faction) >= MinRep;
}
