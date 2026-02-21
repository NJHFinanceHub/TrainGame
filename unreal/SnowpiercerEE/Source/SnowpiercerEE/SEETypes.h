#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SEETypes.generated.h"

// ---------------------------------------------------------------------------
// NPC enums (SnowpiercerEE module mirror of TrainGame ENPCAIState / ENPCClass)
// ---------------------------------------------------------------------------

/** NPC AI state used by legacy SnowpiercerEE NPC classes */
UENUM(BlueprintType)
enum class ESEENPCState : uint8
{
    Idle            UMETA(DisplayName="Idle"),
    Patrolling      UMETA(DisplayName="Patrolling"),
    Suspicious      UMETA(DisplayName="Suspicious"),
    Alerted         UMETA(DisplayName="Alerted"),
    Combat          UMETA(DisplayName="Combat"),
    Fleeing         UMETA(DisplayName="Fleeing"),
    Dead            UMETA(DisplayName="Dead")
};

/** NPC class/caste used by legacy SnowpiercerEE NPC classes */
UENUM(BlueprintType)
enum class ESEENPCClass : uint8
{
    Tailie          UMETA(DisplayName="Tailie"),
    ThirdClass      UMETA(DisplayName="Third Class"),
    SecondClass     UMETA(DisplayName="Second Class"),
    FirstClass      UMETA(DisplayName="First Class"),
    Jackboot        UMETA(DisplayName="Jackboot"),
    Engineer        UMETA(DisplayName="Engineer"),
    Crowd           UMETA(DisplayName="Crowd")
};

// ---------------------------------------------------------------------------
// Train Zone / Faction
// ---------------------------------------------------------------------------

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
};

/** DataTable row defining a car's properties for level setup */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEECarDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FName CarID = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    int32 CarIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    ESEETrainZone Zone = ESEETrainZone::Tail;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    ESEEFaction PrimaryFaction = ESEEFaction::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FName SubLevelName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    int32 ThreatLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    int32 MaxEnemies = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    TArray<FName> NPCIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    TArray<FName> ItemIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    bool bIsBossEncounter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Car")
    FText ShortDescription;
};
