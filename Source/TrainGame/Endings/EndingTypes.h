// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EndingTypes.generated.h"

// ============================================================================
// Ending System Type Definitions
// Snowpiercer: Eternal Engine - 6 Endings + Ending Calculator
// ============================================================================

// ---------------------------------------------------------------------------
// Ending Identifiers
// ---------------------------------------------------------------------------

/** The six possible game endings */
UENUM(BlueprintType)
enum class EEndingType : uint8
{
	None				UMETA(DisplayName = "None"),
	TheNewWilford		UMETA(DisplayName = "The New Wilford"),		// Take the Engine, become the tyrant
	TheRevolution		UMETA(DisplayName = "The Revolution"),		// Destroy class system, democracy on rails
	TheDerailment		UMETA(DisplayName = "The Derailment"),		// Destroy the Engine, face the cold
	TheExodus			UMETA(DisplayName = "The Exodus"),			// Open the front door, lead survivors out
	TheEternalLoop		UMETA(DisplayName = "The Eternal Loop"),		// Discover truth, choose silence or revelation
	TheBridge			UMETA(DisplayName = "The Bridge")			// SECRET: Contact another train
};

// ---------------------------------------------------------------------------
// Moral Ledger Axes (Hidden from player)
// ---------------------------------------------------------------------------

/** The five moral axes tracked by the Hidden Ledger */
UENUM(BlueprintType)
enum class EMoralAxis : uint8
{
	MercyVsPragmatism		UMETA(DisplayName = "Mercy vs. Pragmatism"),		// + = Mercy, - = Pragmatism
	IndividualVsCollective	UMETA(DisplayName = "Individual vs. Collective"),	// + = Individual, - = Collective
	TruthVsStability		UMETA(DisplayName = "Truth vs. Stability"),			// + = Truth, - = Stability
	ForceVsCunning			UMETA(DisplayName = "Force vs. Cunning"),			// + = Force, - = Cunning
	PresentVsFuture			UMETA(DisplayName = "Present vs. Future"),			// + = Present, - = Future
	MAX						UMETA(Hidden)
};

// ---------------------------------------------------------------------------
// Ending Eligibility & Scoring
// ---------------------------------------------------------------------------

/** Ending trigger — the specific action that locks in an ending at the endgame decision point */
UENUM(BlueprintType)
enum class EEndingTrigger : uint8
{
	None				UMETA(DisplayName = "None"),
	SeizeEngine			UMETA(DisplayName = "Seize the Engine"),
	HoldElection		UMETA(DisplayName = "Hold the Election"),
	DetonateEngine		UMETA(DisplayName = "Detonate the Engine"),
	OpenFrontDoor		UMETA(DisplayName = "Open the Front Door"),
	AccessArchive		UMETA(DisplayName = "Access the Archive"),
	ActivateTransmitter	UMETA(DisplayName = "Activate the Transmitter")
};

/** Whether an ending is available and why/why not */
UENUM(BlueprintType)
enum class EEndingEligibility : uint8
{
	Locked			UMETA(DisplayName = "Locked"),			// Prerequisites not met
	Available		UMETA(DisplayName = "Available"),		// Can be chosen
	Recommended		UMETA(DisplayName = "Recommended"),		// Best fit for player's choices
	Hidden			UMETA(DisplayName = "Hidden")			// Secret ending, not yet discovered
};

/** Prerequisite check result for a single ending */
USTRUCT(BlueprintType)
struct FEndingPrerequisite
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PrerequisiteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMet = false;
};

/** Full eligibility snapshot for one ending */
USTRUCT(BlueprintType)
struct FEndingEligibilityResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEndingType EndingType = EEndingType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEndingEligibility Eligibility = EEndingEligibility::Locked;

	/** 0.0 - 1.0 affinity score based on moral ledger alignment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AffinityScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FEndingPrerequisite> Prerequisites;

	/** Fraction of prerequisites met (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrerequisiteProgress = 0.0f;
};

// ---------------------------------------------------------------------------
// Moral Ledger Data
// ---------------------------------------------------------------------------

/** A single moral choice record stored in the ledger */
USTRUCT(BlueprintType)
struct FMoralChoiceRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OptionChosen;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarIndex = 0;

	/** Axis deltas applied by this choice (keyed by EMoralAxis as uint8) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<uint8, int32> AxisDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GameTimeWhenChosen = 0.0f;
};

/** Snapshot of the full moral ledger — used for save/load */
USTRUCT(BlueprintType)
struct FMoralLedgerSnapshot
{
	GENERATED_BODY()

	/** Current axis values. Index = EMoralAxis cast to int. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> AxisValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMoralChoiceRecord> ChoiceHistory;

	/** Global story flags set by choices, quests, or world events */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, bool> StoryFlags;
};

// ---------------------------------------------------------------------------
// Ending Cinematic Data
// ---------------------------------------------------------------------------

/** Music/audio cue for credits variation */
UENUM(BlueprintType)
enum class ECreditsMusic : uint8
{
	Default				UMETA(DisplayName = "Default"),
	Triumphant			UMETA(DisplayName = "Triumphant"),			// TheNewWilford / TheRevolution
	Somber				UMETA(DisplayName = "Somber"),				// TheDerailment
	Hopeful				UMETA(DisplayName = "Hopeful"),				// TheExodus
	Unsettling			UMETA(DisplayName = "Unsettling"),			// TheEternalLoop
	Mysterious			UMETA(DisplayName = "Mysterious")			// TheBridge
};

/** Per-ending cinematic configuration — exposed as a DataAsset */
USTRUCT(BlueprintType)
struct FEndingCinematicData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEndingType EndingType = EEndingType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EndingTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EndingEpilogue;

	/** Level sequence asset for the ending cutscene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> CutsceneSequence;

	/** Map to load for the ending environment (if different from current) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> EndingMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECreditsMusic CreditsMusic = ECreditsMusic::Default;

	/** Sound cue for ending-specific credits music */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> CreditsMusicAsset;

	/** Duration in seconds before auto-transition to credits */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CutsceneDuration = 60.0f;

	/** Companion-specific epilogue lines (companion name -> epilogue text) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FText> CompanionEpilogues;
};

// ---------------------------------------------------------------------------
// Companion Ending State
// ---------------------------------------------------------------------------

/** Companion's fate as evaluated at ending time */
USTRUCT(BlueprintType)
struct FCompanionEndingFate
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CompanionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAlive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInParty = false;

	/** Loyalty at time of ending evaluation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalLoyalty = 0;

	/** Whether personal quest was completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPersonalQuestComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EpilogueText;
};
