// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SnowyEngine/Survival/SurvivalTypes.h"
#include "SnowyEngine/Faction/FactionTypes.h"
#include "SaveTypes.generated.h"

// ============================================================================
// Save System Type Definitions
// Binary format with JSON header (.sav), LZ4-compressed payload
// ============================================================================

// Magic bytes for save file validation
inline constexpr uint8 SAVE_MAGIC[4] = { 'S', 'P', 'E', 'E' };
inline constexpr int32 SAVE_FORMAT_VERSION = 1;

UENUM(BlueprintType)
enum class ESaveSlotType : uint8
{
	Manual		UMETA(DisplayName = "Manual Save"),
	QuickSave	UMETA(DisplayName = "Quick Save"),
	Autosave	UMETA(DisplayName = "Autosave")
};

UENUM(BlueprintType)
enum class EAutosaveTrigger : uint8
{
	CarTransition		UMETA(DisplayName = "Car Transition"),
	QuestCompletion		UMETA(DisplayName = "Quest Completion"),
	CompanionRecruit	UMETA(DisplayName = "Companion Recruitment"),
	MajorChoice			UMETA(DisplayName = "Major Choice"),
	TimedInterval		UMETA(DisplayName = "Timed Interval"),
	PreCombat			UMETA(DisplayName = "Pre-Combat")
};

UENUM(BlueprintType)
enum class ESaveResult : uint8
{
	Success			UMETA(DisplayName = "Success"),
	FailedIO		UMETA(DisplayName = "Failed: I/O Error"),
	FailedCompress	UMETA(DisplayName = "Failed: Compression Error"),
	FailedInvalid	UMETA(DisplayName = "Failed: Invalid State"),
	Cancelled		UMETA(DisplayName = "Cancelled")
};

UENUM(BlueprintType)
enum class ELoadResult : uint8
{
	Success				UMETA(DisplayName = "Success"),
	FailedNotFound		UMETA(DisplayName = "Failed: File Not Found"),
	FailedCorrupt		UMETA(DisplayName = "Failed: Corrupt File"),
	FailedVersion		UMETA(DisplayName = "Failed: Incompatible Version"),
	FailedChecksum		UMETA(DisplayName = "Failed: Checksum Mismatch"),
	SuccessModded		UMETA(DisplayName = "Success: Modded Save (checksum mismatch)")
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveComplete, ESaveResult, Result, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadComplete, ELoadResult, Result, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutosaveTriggered, EAutosaveTrigger, Trigger);

// ============================================================================
// Save Data Structs — mirror the binary payload sections
// ============================================================================

/** JSON header metadata — readable without deserializing binary payload */
USTRUCT(BlueprintType)
struct FSaveHeaderData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 FormatVersion = SAVE_FORMAT_VERSION;

	UPROPERTY(BlueprintReadWrite)
	FString GameVersion;

	UPROPERTY(BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(BlueprintReadWrite)
	float PlayTimeSeconds = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString SlotName;

	UPROPERTY(BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentCarIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	FString CurrentZone;

	UPROPERTY(BlueprintReadWrite)
	float CompletionPercent = 0.f;

	UPROPERTY(BlueprintReadWrite)
	FString Checksum;

	UPROPERTY(BlueprintReadWrite)
	ESaveSlotType SlotType = ESaveSlotType::Manual;
};

/** Global state — world time, difficulty, story flags, faction rep */
USTRUCT(BlueprintType)
struct FSaveGlobalState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	double WorldTimeSeconds = 0.0;

	UPROPERTY(BlueprintReadWrite)
	uint8 DifficultyTier = 1; // 0=Passenger, 1=Survivor, 2=EternalEngine, 3=MrWilford

	UPROPERTY(BlueprintReadWrite)
	bool bAdaptiveDifficultyEnabled = false;

	UPROPERTY(BlueprintReadWrite)
	TArray<bool> RevolutionFlags;

	UPROPERTY(BlueprintReadWrite)
	TArray<int32> FactionReputations; // 8 factions, -1000 to +1000

	UPROPERTY(BlueprintReadWrite)
	TArray<bool> DiscoveredCars; // bitfield: which cars visited

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> GlobalEventLog;

	FSaveGlobalState()
	{
		RevolutionFlags.SetNum(32);
		FactionReputations.SetNumZeroed(8);
		DiscoveredCars.SetNum(128);
	}
};

/** Player state — position, stats, inventory, equipment, effects */
USTRUCT(BlueprintType)
struct FSavePlayerState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentCarIndex = 0;

	// 6 stats: STR, AGI, END, CUN, PER, CHA
	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> Stats;

	UPROPERTY(BlueprintReadWrite)
	uint8 Level = 1;

	UPROPERTY(BlueprintReadWrite)
	int32 XP = 0;

	UPROPERTY(BlueprintReadWrite)
	FSurvivalSnapshot SurvivalState;

	// Serialized inventory item IDs + counts + durability
	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveItemEntry> InventoryItems;

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> ActivePerks;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveActiveEffect> ActiveEffects;

	UPROPERTY(BlueprintReadWrite)
	FName EquippedWeaponID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	FName EquippedArmorID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	FName ActiveDisguiseID = NAME_None;

	FSavePlayerState()
	{
		Stats.SetNumZeroed(6);
	}
};

/** A single inventory item in save data */
USTRUCT(BlueprintType)
struct FSaveItemEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName ItemID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	int32 StackCount = 1;

	UPROPERTY(BlueprintReadWrite)
	float Durability = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	float DegradationProgress = 0.f;
};

/** An active buff/debuff in save data */
USTRUCT(BlueprintType)
struct FSaveActiveEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName EffectID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	float RemainingDuration = 0.f;

	UPROPERTY(BlueprintReadWrite)
	float Intensity = 1.f;
};

/** Per-car state delta — only modified cars are stored */
USTRUCT(BlueprintType)
struct FSaveCarState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 CarIndex = 0;

	// Bitfield: which containers looted
	UPROPERTY(BlueprintReadWrite)
	TArray<bool> LootedContainers;

	// Per-door state: 0=default, 1=open, 2=locked, 3=broken
	UPROPERTY(BlueprintReadWrite)
	TArray<uint8> DoorStates;

	// Bitfield: which destructibles destroyed
	UPROPERTY(BlueprintReadWrite)
	TArray<bool> DestroyedDestructibles;

	// NPC overrides for this car (dead, moved, alerted)
	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveNPCOverride> NPCOverrides;

	// Procedural clutter RNG seed for consistent regeneration
	UPROPERTY(BlueprintReadWrite)
	int32 ClutterSeed = 0;

	// Car-specific scripted flags (quest items placed, traps set)
	UPROPERTY(BlueprintReadWrite)
	TArray<FName> CustomFlags;

	// Faction that currently controls this car
	UPROPERTY(BlueprintReadWrite)
	FName ControllingFaction = NAME_None;
};

/** NPC override within a car's delta state */
USTRUCT(BlueprintType)
struct FSaveNPCOverride
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName NPCID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	bool bDead = false;

	UPROPERTY(BlueprintReadWrite)
	bool bAlerted = false;

	UPROPERTY(BlueprintReadWrite)
	int32 OverrideCarIndex = -1; // -1 = default position
};

/** Global NPC state — tracked independently from per-car */
USTRUCT(BlueprintType)
struct FSaveNPCState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName NPCID = NAME_None;

	UPROPERTY(BlueprintReadWrite)
	bool bAlive = true;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentCarIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Disposition = 0; // -100 to +100

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> MemoryTags;

	UPROPERTY(BlueprintReadWrite)
	uint8 ActiveSchedule = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> SeenDialogueNodes;
};

/** Quest state */
USTRUCT(BlueprintType)
struct FSaveQuestState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName QuestID = NAME_None;

	// 0=unknown, 1=active, 2=completed, 3=failed, 4=abandoned
	UPROPERTY(BlueprintReadWrite)
	uint8 Status = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 CurrentStep = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<bool> StepFlags;

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> RecordedChoices;
};

/** Companion state */
USTRUCT(BlueprintType)
struct FSaveCompanionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	uint8 CompanionIndex = 0;

	UPROPERTY(BlueprintReadWrite)
	bool bRecruited = false;

	UPROPERTY(BlueprintReadWrite)
	bool bAlive = true;

	UPROPERTY(BlueprintReadWrite)
	int32 Loyalty = 0; // -100 to +100

	// 0=Hostile, 1=Suspicious, 2=Wary, 3=Neutral, 4=Warm, 5=Trusted, 6=Bonded
	UPROPERTY(BlueprintReadWrite)
	uint8 InternalState = 3;

	UPROPERTY(BlueprintReadWrite)
	bool bInParty = false;

	UPROPERTY(BlueprintReadWrite)
	int32 PersonalQuestStep = 0;

	// 0=none, 1=interested, 2=active, 3=committed
	UPROPERTY(BlueprintReadWrite)
	uint8 RomanceState = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FName> GiftHistory;
};

/** Complete save data — all sections combined */
USTRUCT(BlueprintType)
struct FSaveGameData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FSaveHeaderData Header;

	UPROPERTY(BlueprintReadWrite)
	FSaveGlobalState GlobalState;

	UPROPERTY(BlueprintReadWrite)
	FSavePlayerState PlayerState;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveCarState> ModifiedCars;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveNPCState> NPCStates;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveQuestState> QuestStates;

	UPROPERTY(BlueprintReadWrite)
	TArray<FSaveCompanionState> CompanionStates;
};

/** Slot info displayed in the save/load UI without full deserialization */
USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = -1;

	UPROPERTY(BlueprintReadOnly)
	ESaveSlotType SlotType = ESaveSlotType::Manual;

	UPROPERTY(BlueprintReadOnly)
	bool bIsEmpty = true;

	UPROPERTY(BlueprintReadOnly)
	FSaveHeaderData Header;
};

/** Save slot configuration */
USTRUCT(BlueprintType)
struct FSaveSlotConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ManualSlotCount = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuickSaveSlotCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AutosaveSlotCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutosaveCooldownSeconds = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimedAutosaveIntervalSeconds = 600.f; // 10 minutes

	int32 GetTotalSlotCount() const { return ManualSlotCount + QuickSaveSlotCount + AutosaveSlotCount; }
};
