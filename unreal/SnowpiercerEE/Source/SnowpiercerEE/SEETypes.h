#pragma once

#include "CoreMinimal.h"
#include "SEETypes.generated.h"

UENUM(BlueprintType)
enum class ESEETrainZone : uint8
{
    Tail            UMETA(DisplayName="Tail"),
    ThirdClass      UMETA(DisplayName="Third Class"),
    SecondClass     UMETA(DisplayName="Second Class"),
    WorkingSpine    UMETA(DisplayName="Working Spine"),
    FirstClass      UMETA(DisplayName="First Class"),
    Sanctum         UMETA(DisplayName="Sanctum"),
    Engine          UMETA(DisplayName="Engine")
};

UENUM(BlueprintType)
enum class ESEEFaction : uint8
{
    Neutral             UMETA(DisplayName="Neutral"),
    Tailies             UMETA(DisplayName="Tailies"),
    ThirdClassUnion     UMETA(DisplayName="Third Class Union"),
    Jackboots           UMETA(DisplayName="Jackboots"),
    Bureaucracy         UMETA(DisplayName="Bureaucracy"),
    FirstClassElite     UMETA(DisplayName="First Class Elite"),
    OrderOfTheEngine    UMETA(DisplayName="Order of the Engine"),
    KronoleNetwork      UMETA(DisplayName="Kronole Network"),
    TheThaw             UMETA(DisplayName="The Thaw")
};

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEECarData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    int32 CarIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FName CarName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    ESEETrainZone Zone = ESEETrainZone::Tail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    ESEEFaction PrimaryFaction = ESEEFaction::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FName SubLevelName = NAME_None;
};

// ── Difficulty ──

UENUM(BlueprintType)
enum class ESEEDifficulty : uint8
{
    Passenger       UMETA(DisplayName="Passenger"),
    Survivor        UMETA(DisplayName="Survivor"),
    EternalEngine   UMETA(DisplayName="Eternal Engine"),
    MrWilford       UMETA(DisplayName="Mr Wilford")
};

// ── Accessibility Enums ──

UENUM(BlueprintType)
enum class ESEEColorblindMode : uint8
{
    None            UMETA(DisplayName="None"),
    Protanopia      UMETA(DisplayName="Protanopia"),
    Deuteranopia    UMETA(DisplayName="Deuteranopia"),
    Tritanopia      UMETA(DisplayName="Tritanopia")
};

UENUM(BlueprintType)
enum class ESEESpeakerIDMode : uint8
{
    Off             UMETA(DisplayName="Off"),
    NameOnly        UMETA(DisplayName="Name Only"),
    NameAndColor    UMETA(DisplayName="Name + Color")
};

UENUM(BlueprintType)
enum class ESEESubtitleSize : uint8
{
    Small           UMETA(DisplayName="Small"),
    Medium          UMETA(DisplayName="Medium"),
    Large           UMETA(DisplayName="Large"),
    ExtraLarge      UMETA(DisplayName="Extra Large")
};

UENUM(BlueprintType)
enum class ESEESaveSlotType : uint8
{
    Manual          UMETA(DisplayName="Manual"),
    Autosave        UMETA(DisplayName="Autosave")
};

// ── Per-Car Save State ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEECarState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    bool bVisited = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    TMap<FName, int32> IntFlags;

    // Door states: DoorID -> bLocked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    TMap<FName, bool> DoorStates;

    // Loot container states: ContainerID -> bLooted
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    TMap<FName, bool> LootStates;

    // Faction currently controlling this car
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    ESEEFaction ControllingFaction = ESEEFaction::Neutral;

    // Destructible object states: ObjectID -> bDestroyed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="State")
    TMap<FName, bool> DestructibleStates;
};

// ── Per-NPC Save State ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEENPCSaveState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    FName NPCID;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    bool bAlive = true;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    int32 CarIndex = -1;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    FRotator Rotation = FRotator::ZeroRotator;

    // Disposition toward player: -100 (hostile) to +100 (devoted)
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    int32 Disposition = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="NPC")
    TMap<FName, bool> ConversationFlags;
};

// ── Player Save State ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEPlayerSaveState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    int32 CurrentCarIndex = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    float Health = 100.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    float Stamina = 100.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    float Hunger = 100.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    float ColdExposure = 0.0f;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    int32 Level = 1;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    int32 XP = 0;

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    int32 SkillPoints = 0;

    // Stat values keyed by stat name (STR, AGI, END, CUN, PER, CHA)
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    TMap<FName, int32> Stats;

    // Inventory: ItemID -> Quantity
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Player")
    TMap<FName, int32> InventoryItems;
};

// ── Quest Save State ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEQuestSaveState
{
    GENERATED_BODY()

    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Quest")
    FName QuestID;

    // 0=Available, 1=Active, 2=Completed, 3=Failed (mirrors ESEEQuestState)
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Quest")
    uint8 State = 0;

    // Current objective index
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Quest")
    int32 CurrentStep = 0;

    // Per-step completion flags
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Quest")
    TMap<FName, bool> StepFlags;

    // Player choices made during this quest
    UPROPERTY(SaveGame, BlueprintReadWrite, Category="Quest")
    TMap<FName, int32> PlayerChoices;
};

// ── Save Slot Metadata ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEESaveSlotInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="Save")
    FString SlotName;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    ESEESaveSlotType SlotType = ESEESaveSlotType::Manual;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    bool bOccupied = false;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    FDateTime Timestamp;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    float PlayTimeSeconds = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    int32 PlayerLevel = 1;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    FString CurrentZoneName;

    UPROPERTY(BlueprintReadOnly, Category="Save")
    ESEEDifficulty Difficulty = ESEEDifficulty::Survivor;
};

// ── Difficulty Modifiers ──

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEEDifficultyModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float EnemyHealthMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float EnemyDamageMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float DetectionRangeMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float DetectionSpeedMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float ResourceDropMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float ResourceDegradationMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float ColdExposureTimeMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float StaminaDrainMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float XPGainMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float FallDamageMult = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float CompanionDownTimer = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    int32 StatCheckBonus = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    float ResourceLossOnDeath = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    bool bPermadeath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    bool bCompanionPermadeath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    bool bShowQuestMarkers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Difficulty")
    bool bFriendlyFire = false;
};
