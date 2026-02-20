// CollectibleTypes.h
// Snowpiercer: Eternal Engine - Exploration System Type Definitions

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "Engine/StaticMesh.h"
#include "CollectibleTypes.generated.h"

/** Category of collectible item */
UENUM(BlueprintType)
enum class ECollectibleType : uint8
{
	TrainLog        UMETA(DisplayName = "Train Log"),
	AudioRecording  UMETA(DisplayName = "Audio Recording"),
	Artifact        UMETA(DisplayName = "Artifact"),
	Blueprint       UMETA(DisplayName = "Blueprint"),
	FactionIntel    UMETA(DisplayName = "Faction Intel"),
	ManifestPage    UMETA(DisplayName = "Manifest Page")
};

/** Rarity tier affecting journal presentation and rewards */
UENUM(BlueprintType)
enum class ECollectibleRarity : uint8
{
	Common      UMETA(DisplayName = "Common"),
	Uncommon    UMETA(DisplayName = "Uncommon"),
	Rare        UMETA(DisplayName = "Rare"),
	Legendary   UMETA(DisplayName = "Legendary")
};

/** Train zone where the collectible is located */
UENUM(BlueprintType)
enum class ETrainZone : uint8
{
	Zone1_Tail          UMETA(DisplayName = "Zone 1 - The Tail"),
	Zone2_ThirdClass    UMETA(DisplayName = "Zone 2 - Third Class"),
	Zone3_SecondClass   UMETA(DisplayName = "Zone 3 - Second Class"),
	Zone4_WorkingSpine  UMETA(DisplayName = "Zone 4 - Working Spine"),
	Zone5_FirstClass    UMETA(DisplayName = "Zone 5 - First Class"),
	Zone6_Sanctum       UMETA(DisplayName = "Zone 6 - The Sanctum"),
	Zone7_Engine        UMETA(DisplayName = "Zone 7 - The Engine"),
	Exterior            UMETA(DisplayName = "Exterior / Secret")
};

/** How the collectible is hidden in the world */
UENUM(BlueprintType)
enum class ECollectibleAccessType : uint8
{
	Visible,            // Directly visible and interactable
	EnvironmentPuzzle,  // Requires environmental interaction (switches, plates)
	LockedContainer,    // Requires CUN-based lockpick or key
	FalseWall,          // Requires STR check or hidden switch
	VentAccess,         // Found inside vent crawlspace
	CompanionTriggered, // Companion presence triggers discovery
	ExteriorOnly,       // Requires rooftop or under-car access
	TimedEvent          // Only available during specific story window
};

/** Persistence category for missable content tracking */
UENUM(BlueprintType)
enum class ECollectiblePersistence : uint8
{
	Persistent,     // Always available, can backtrack
	Timed,          // Available only during story windows
	ChoiceDependent // Only on certain narrative paths
};

/** Data table row for collectible definitions */
USTRUCT(BlueprintType)
struct FCollectibleData : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique identifier (e.g., "TLOG_Z1_001") */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName CollectibleID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECollectibleType Type;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECollectibleRarity Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETrainZone Zone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECollectibleAccessType AccessType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECollectiblePersistence Persistence;

	/** Car number where this collectible is found (0 for secret cars) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0", ClampMax = "110"))
	int32 CarNumber = 0;

	/** Stat requirement to access (NAME = stat, e.g., "STR", value = minimum) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> StatRequirements;

	/** Other collectible IDs this links to in the journal */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> LinkedCollectibles;

	/** Companion whose resonance is triggered by this collectible */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ResonanceCompanion;

	/** Loyalty bonus granted when resonance companion is present */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ResonanceLoyaltyBonus = 0;

	/** Codex entry IDs to create or update when collected */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> CodexEntries;

	// Type-specific references

	/** Audio asset for Audio Recordings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USoundBase> AudioAsset;

	/** Mesh for 3D-inspectable Artifacts */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> InspectMesh;

	/** Crafting recipe ID unlocked by Blueprints */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName UnlockedRecipeID;

	/** Faction affected by Faction Intel */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName FactionID;
};

/** Runtime state for a collected item */
USTRUCT(BlueprintType)
struct FCollectibleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName CollectibleID;

	UPROPERTY(BlueprintReadOnly)
	bool bCollected = false;

	UPROPERTY(BlueprintReadOnly)
	bool bViewed = false;

	/** In-game day when collected */
	UPROPERTY(BlueprintReadOnly)
	int32 CollectionDay = -1;
};

/** Faction intel progress for Web of Power */
USTRUCT(BlueprintType)
struct FFactionIntelProgress
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName FactionID;

	UPROPERTY(BlueprintReadOnly)
	int32 IntelCollected = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 IntelTotal = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bCriticalWeaknessRevealed = false;

	UPROPERTY(BlueprintReadOnly)
	bool bSpecialDialogueUnlocked = false;
};
