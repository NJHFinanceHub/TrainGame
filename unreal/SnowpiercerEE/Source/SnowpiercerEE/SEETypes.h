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
