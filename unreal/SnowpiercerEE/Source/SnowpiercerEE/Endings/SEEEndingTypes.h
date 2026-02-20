// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEEEndingTypes.generated.h"

// ============================================================================
// Ending System Type Definitions
// Snowpiercer: Eternal Engine - 6 Endings + Hidden Ledger
// ============================================================================

/** The six possible endings of the game */
UENUM(BlueprintType)
enum class ESEEEnding : uint8
{
	None			UMETA(DisplayName = "None"),
	TheNewWilford	UMETA(DisplayName = "The New Wilford"),
	TheRevolution	UMETA(DisplayName = "The Revolution"),
	TheDerailment	UMETA(DisplayName = "The Derailment"),
	TheExodus		UMETA(DisplayName = "The Exodus"),
	TheEternalLoop	UMETA(DisplayName = "The Eternal Loop"),
	TheBridge		UMETA(DisplayName = "The Bridge (Secret)")
};

/** The five consequence axes tracked by the Hidden Ledger */
UENUM(BlueprintType)
enum class ESEELedgerAxis : uint8
{
	MercyVsPragmatism		UMETA(DisplayName = "Mercy vs. Pragmatism"),
	IndividualVsCollective	UMETA(DisplayName = "Individual vs. Collective"),
	TruthVsStability		UMETA(DisplayName = "Truth vs. Stability"),
	ForceVsCunning			UMETA(DisplayName = "Force vs. Cunning"),
	PresentVsFuture			UMETA(DisplayName = "Present vs. Future"),
	COUNT					UMETA(Hidden)
};

/** Ending variation quality — how the ending plays out based on Ledger scores */
UENUM(BlueprintType)
enum class ESEEEndingVariation : uint8
{
	Ideal		UMETA(DisplayName = "Ideal"),
	Good		UMETA(DisplayName = "Good"),
	Bittersweet	UMETA(DisplayName = "Bittersweet"),
	Dark		UMETA(DisplayName = "Dark")
};

/** Cinematic type for ending sequences */
UENUM(BlueprintType)
enum class ESEEEndingCinematicPhase : uint8
{
	Confrontation	UMETA(DisplayName = "Wilford Confrontation"),
	Decision		UMETA(DisplayName = "The Decision"),
	Consequence		UMETA(DisplayName = "Immediate Consequence"),
	Epilogue		UMETA(DisplayName = "Epilogue Montage"),
	Credits			UMETA(DisplayName = "Credits")
};

/** A single moral choice record logged to the Ledger */
USTRUCT(BlueprintType)
struct FSEELedgerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SourceQuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarNumber = 0;

	/** Axis deltas: positive = first value (Mercy, Individual, Truth, Force, Present) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESEELedgerAxis, int32> AxisDeltas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceDescription;
};

/** Snapshot of all five Ledger axis scores */
USTRUCT(BlueprintType)
struct FSEELedgerSnapshot
{
	GENERATED_BODY()

	/** Mercy(+) vs Pragmatism(-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 MercyVsPragmatism = 0;

	/** Individual(+) vs Collective(-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 IndividualVsCollective = 0;

	/** Truth(+) vs Stability(-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 TruthVsStability = 0;

	/** Force(+) vs Cunning(-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 ForceVsCunning = 0;

	/** Present(+) vs Future(-). Range: -100 to +100 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 PresentVsFuture = 0;

	int32 GetAxisScore(ESEELedgerAxis Axis) const
	{
		switch (Axis)
		{
		case ESEELedgerAxis::MercyVsPragmatism: return MercyVsPragmatism;
		case ESEELedgerAxis::IndividualVsCollective: return IndividualVsCollective;
		case ESEELedgerAxis::TruthVsStability: return TruthVsStability;
		case ESEELedgerAxis::ForceVsCunning: return ForceVsCunning;
		case ESEELedgerAxis::PresentVsFuture: return PresentVsFuture;
		default: return 0;
		}
	}

	void SetAxisScore(ESEELedgerAxis Axis, int32 Value)
	{
		Value = FMath::Clamp(Value, -100, 100);
		switch (Axis)
		{
		case ESEELedgerAxis::MercyVsPragmatism: MercyVsPragmatism = Value; break;
		case ESEELedgerAxis::IndividualVsCollective: IndividualVsCollective = Value; break;
		case ESEELedgerAxis::TruthVsStability: TruthVsStability = Value; break;
		case ESEELedgerAxis::ForceVsCunning: ForceVsCunning = Value; break;
		case ESEELedgerAxis::PresentVsFuture: PresentVsFuture = Value; break;
		default: break;
		}
	}

	void ApplyDelta(ESEELedgerAxis Axis, int32 Delta)
	{
		SetAxisScore(Axis, GetAxisScore(Axis) + Delta);
	}
};

/** Requirements for an ending to become available */
USTRUCT(BlueprintType)
struct FSEEEndingRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEnding Ending = ESEEEnding::None;

	/** Minimum Ledger axis scores required (axis -> threshold). Positive = first value side */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESEELedgerAxis, int32> MinAxisScores;

	/** Maximum Ledger axis scores (axis -> ceiling). Used for endings that require low scores */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESEELedgerAxis, int32> MaxAxisScores;

	/** Required global game flags (e.g., "Gilliam_Recording_Found") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredFlags;

	/** Flags that must NOT be set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> BlockingFlags;

	/** Minimum companion survival count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCompanionsSurvived = 0;

	/** Maximum companion deaths (for endings requiring loss) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCompanionsSurvived = 12;

	/** Required faction standings (faction -> minimum rep) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> RequiredFactionStandings;

	/** Minimum collectibles found (for secret ending) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinCollectiblesFound = 0;

	/** Priority for display order (lower = shown first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DisplayPriority = 100;
};

/** Result of the ending calculation — which ending and how it plays out */
USTRUCT(BlueprintType)
struct FSEEEndingResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEnding Ending = ESEEEnding::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEndingVariation Variation = ESEEEndingVariation::Bittersweet;

	/** 0.0 to 1.0 — how well the player's profile matches this ending */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Affinity = 0.0f;

	/** Whether this ending is unlocked (all requirements met) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> UnmetRequirements;
};

/** Data for a single ending's cinematic presentation */
USTRUCT(BlueprintType)
struct FSEEEndingCinematicData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEnding Ending = ESEEEnding::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EndingTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EndingSubtitle;

	/** Wilford's dialogue line for this ending (varies by Ledger) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESEEEndingVariation, FText> WilfordDialogue;

	/** Epilogue narration text (varies by variation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ESEEEndingVariation, FText> EpilogueNarration;

	/** Level sequence asset path for the ending cutscene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> CutsceneSequence;

	/** Credits music asset path (each ending has unique music) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> CreditsMusic;

	/** Names of companions who appear in the ending (populated at runtime) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> SurvivingCompanionsInEnding;
};

/** Post-game statistics shown during credits */
USTRUCT(BlueprintType)
struct FSEEEndgameStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEnding EndingChosen = ESEEEnding::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEEndingVariation Variation = ESEEEndingVariation::Bittersweet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarsVisited = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompanionsRecruited = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompanionsLost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MoralChoicesMade = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuestsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CollectiblesFound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesKilled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EnemiesSpared = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSEELedgerSnapshot FinalLedger;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlaytimeHours = 0.0f;
};

/** Global game flag entry — persistent across the entire playthrough */
USTRUCT(BlueprintType)
struct FSEEGlobalFlag
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName FlagName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 IntValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bBoolValue = false;
};
