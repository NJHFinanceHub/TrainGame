// TransportDeckTypes.h
// Snowpiercer: Eternal Engine - Lower Transport Deck type definitions

#pragma once

#include "CoreMinimal.h"
#include "SEETypes.h"
#include "TransportDeckTypes.generated.h"

/** Cargo categories transported on the lower deck mini-rail */
UENUM(BlueprintType)
enum class ECargoType : uint8
{
	FoodCrates      UMETA(DisplayName = "Food Crates"),
	MedicalSupplies UMETA(DisplayName = "Medical Supplies"),
	LuxuryGoods     UMETA(DisplayName = "Luxury Goods"),
	Contraband      UMETA(DisplayName = "Contraband"),
	IndustrialParts UMETA(DisplayName = "Industrial Parts"),
	WaterBarrels    UMETA(DisplayName = "Water Barrels")
};

/** Access point types connecting upper and lower decks */
UENUM(BlueprintType)
enum class EDeckAccessType : uint8
{
	FloorHatch          UMETA(DisplayName = "Floor Hatch"),
	FreightElevator     UMETA(DisplayName = "Freight Elevator"),
	MaintenanceStairs   UMETA(DisplayName = "Maintenance Stairs"),
	CargoChute          UMETA(DisplayName = "Cargo Chute")
};

/** Mini-rail track junction types */
UENUM(BlueprintType)
enum class ERailJunction : uint8
{
	Straight    UMETA(DisplayName = "Straight"),
	SwitchLeft  UMETA(DisplayName = "Switch Left"),
	SwitchRight UMETA(DisplayName = "Switch Right"),
	Terminus    UMETA(DisplayName = "Terminus")
};

/** Mini-rail cart state */
UENUM(BlueprintType)
enum class ECartState : uint8
{
	Stationary  UMETA(DisplayName = "Stationary"),
	Moving      UMETA(DisplayName = "Moving"),
	Loading     UMETA(DisplayName = "Loading"),
	Derailed    UMETA(DisplayName = "Derailed")
};

/** Automated security threat types on the lower deck */
UENUM(BlueprintType)
enum class EDeckSecurityType : uint8
{
	None,
	LaserTripwire   UMETA(DisplayName = "Laser Tripwire"),
	MotionSensor    UMETA(DisplayName = "Motion Sensor"),
	PatrolDrone     UMETA(DisplayName = "Patrol Drone"),
	PressurePlate   UMETA(DisplayName = "Pressure Plate"),
	SecurityCamera  UMETA(DisplayName = "Security Camera")
};

/** Alert level on the lower deck */
UENUM(BlueprintType)
enum class EDeckAlertLevel : uint8
{
	Clear       UMETA(DisplayName = "Clear"),
	Suspicious  UMETA(DisplayName = "Suspicious"),
	Alert       UMETA(DisplayName = "Alert"),
	Lockdown    UMETA(DisplayName = "Lockdown")
};

/** Lower deck segment — defines a section of transport deck beneath a car */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FTransportDeckSegment
{
	GENERATED_BODY()

	/** Upper-level car index this segment sits beneath */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	int32 CarIndex = 0;

	/** Name of this lower deck segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	FName SegmentName = NAME_None;

	/** Which zone this segment belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	ESEETrainZone Zone = ESEETrainZone::ThirdClass;

	/** Whether this segment has an active mini-rail track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	bool bHasRailTrack = true;

	/** Track junction type at this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	ERailJunction JunctionType = ERailJunction::Straight;

	/** Access points available from upper level */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	TArray<EDeckAccessType> AccessPoints;

	/** Cargo types stored/transited in this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	TArray<ECargoType> CargoPresent;

	/** Security devices in this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	TArray<EDeckSecurityType> SecurityDevices;

	/** Which faction controls this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	ESEEFaction ControllingFaction = ESEEFaction::Neutral;

	/** Whether the Kronole Network uses this segment for smuggling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	bool bKronoleSmuggling = false;

	/** Sub-level name for streaming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransportDeck")
	FName SubLevelName = NAME_None;
};

/** Cargo manifest entry — a specific cargo item on a cart */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FCargoManifest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	FName CargoID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	ECargoType Type = ECargoType::FoodCrates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	FText Description;

	/** Source car index (loaded from) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	int32 SourceCar = 0;

	/** Destination car index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	int32 DestinationCar = 0;

	/** Economic value (for smuggling/trading) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	int32 Value = 0;

	/** Whether this cargo is illegal (triggers alert if scanned) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cargo")
	bool bContraband = false;
};

/** Mini-rail cart definition */
USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FMiniRailCart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	FName CartID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	ECartState State = ECartState::Stationary;

	/** Current segment car index */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	int32 CurrentCarIndex = 0;

	/** Speed in m/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	float Speed = 5.0f;

	/** Maximum speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	float MaxSpeed = 12.0f;

	/** Cargo loaded on this cart */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	TArray<FCargoManifest> LoadedCargo;

	/** Whether player is riding this cart */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	bool bPlayerRiding = false;

	/** Whether cart generates noise (alerting security) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniRail")
	float NoiseRadius = 8.0f;
};
