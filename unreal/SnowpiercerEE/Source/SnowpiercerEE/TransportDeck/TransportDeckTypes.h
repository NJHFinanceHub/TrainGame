// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SnowpiercerEE/SEETypes.h"
#include "TransportDeckTypes.generated.h"

// ============================================================================
// Lower Transport Deck Type Definitions
// Sub-level cargo system running beneath Cars 15-82
// (Third Class through First Class)
// ============================================================================

/** Cargo category for mini-rail transport */
UENUM(BlueprintType)
enum class ECargoType : uint8
{
	FoodCrates		UMETA(DisplayName = "Food Crates"),
	MedicalSupplies	UMETA(DisplayName = "Medical Supplies"),
	LuxuryGoods		UMETA(DisplayName = "Luxury Goods"),
	Contraband		UMETA(DisplayName = "Contraband"),
	EngineParts		UMETA(DisplayName = "Engine Parts"),
	WaterTanks		UMETA(DisplayName = "Water Tanks"),
	TextilesBedding	UMETA(DisplayName = "Textiles & Bedding"),
	Chemicals		UMETA(DisplayName = "Chemicals")
};

/** Access point type between upper and lower decks */
UENUM(BlueprintType)
enum class EDeckAccessType : uint8
{
	FloorHatch			UMETA(DisplayName = "Floor Hatch"),
	FreightElevator		UMETA(DisplayName = "Freight Elevator"),
	MaintenanceStairs	UMETA(DisplayName = "Maintenance Stairs"),
	EmergencyLadder		UMETA(DisplayName = "Emergency Ladder"),
	VentShaft			UMETA(DisplayName = "Vent Shaft")
};

/** Mini-rail cart operational state */
UENUM(BlueprintType)
enum class EMiniRailCartState : uint8
{
	Idle			UMETA(DisplayName = "Idle"),
	Moving			UMETA(DisplayName = "Moving"),
	Loading			UMETA(DisplayName = "Loading"),
	Unloading		UMETA(DisplayName = "Unloading"),
	Derailed		UMETA(DisplayName = "Derailed"),
	Locked			UMETA(DisplayName = "Locked")
};

/** Track segment type */
UENUM(BlueprintType)
enum class ETrackSegmentType : uint8
{
	Straight		UMETA(DisplayName = "Straight"),
	Junction		UMETA(DisplayName = "Junction"),
	Turntable		UMETA(DisplayName = "Turntable"),
	Loading			UMETA(DisplayName = "Loading Bay"),
	Terminus		UMETA(DisplayName = "Terminus")
};

/** Lower deck security threat type */
UENUM(BlueprintType)
enum class EDeckSecurityType : uint8
{
	MotionSensor		UMETA(DisplayName = "Motion Sensor"),
	LaserTripwire		UMETA(DisplayName = "Laser Tripwire"),
	PatrolDrone			UMETA(DisplayName = "Patrol Drone"),
	PressurePlate		UMETA(DisplayName = "Pressure Plate"),
	SecurityCamera		UMETA(DisplayName = "Security Camera")
};

/** Alert level for deck security system */
UENUM(BlueprintType)
enum class EDeckAlertLevel : uint8
{
	Clear			UMETA(DisplayName = "Clear"),
	Caution			UMETA(DisplayName = "Caution"),
	Alert			UMETA(DisplayName = "Alert"),
	Lockdown		UMETA(DisplayName = "Lockdown")
};

/** Environmental hazard on the lower deck */
UENUM(BlueprintType)
enum class EDeckHazard : uint8
{
	None,
	SteamLeak,			// Periodic burst from pipes
	ElectricalArc,		// Exposed wiring
	RatSwarm,			// Noise hazard, minor damage
	LoosePlating,		// Fall risk / noise
	ColdBreach,			// Hull breach letting in outside cold
	ChemicalSpill		// Toxic area, requires mask
};

// ----------------------------------------------------------------------------
// Structs
// ----------------------------------------------------------------------------

/** Cargo manifest entry for a single crate/container */
USTRUCT(BlueprintType)
struct FCargoManifest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CargoID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECargoType CargoType = ECargoType::FoodCrates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 50.0f;

	/** Source car (where cargo was loaded) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SourceCar = 15;

	/** Destination car */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DestinationCar = 82;

	/** Whether this cargo is flagged as contraband by Jackboots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsContraband = false;

	/** Faction that owns this cargo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEFaction OwnerFaction = ESEEFaction::Neutral;

	/** Value in scrap if stolen/traded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TradeValue = 10.0f;
};

/** Track segment definition for mini-rail routing */
USTRUCT(BlueprintType)
struct FTrackSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SegmentID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ETrackSegmentType SegmentType = ETrackSegmentType::Straight;

	/** Car index this segment is beneath */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarIndex = 0;

	/** World-space start point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector StartPoint = FVector::ZeroVector;

	/** World-space end point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EndPoint = FVector::ZeroVector;

	/** Max cart speed on this segment (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 5.0f;

	/** Connected segment IDs (for junctions, may have 2+) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> ConnectedSegments;

	/** Whether this segment is powered */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPowered = true;

	/** Environmental hazard on this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDeckHazard Hazard = EDeckHazard::None;
};

/** Access point configuration between upper and lower decks */
USTRUCT(BlueprintType)
struct FDeckAccessConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AccessID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EDeckAccessType AccessType = EDeckAccessType::FloorHatch;

	/** Car this access point is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarIndex = 0;

	/** Transition time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TransitionTime = 2.0f;

	/** Whether this access point requires a key/hack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLocked = false;

	/** Cunning score required to pick/hack (0 = no check) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LockDifficulty = 0;

	/** Noise generated when using this access point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NoiseRadius = 5.0f;

	/** Whether using this alerts nearby NPCs above */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAlertsAbove = false;
};

/** Cargo DataTable row for DT_TransportDeck_Cargo */
USTRUCT(BlueprintType)
struct FCargoDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CargoID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECargoType CargoType = ECargoType::FoodCrates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TradeValue = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsContraband = false;

	/** Items granted when opened/looted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> LootTable;

	/** Faction that will be hostile if you steal this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEFaction OwnerFaction = ESEEFaction::Neutral;

	/** Minimum zone where this cargo type appears */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEETrainZone MinZone = ESEETrainZone::ThirdClass;
};

// ----------------------------------------------------------------------------
// Delegates
// ----------------------------------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeckEntered, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeckExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCartBoarded, FName, CartID, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCartExited, FName, CartID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCargoLooted, FName, CargoID, ECargoType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeckAlertChanged, EDeckAlertLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecurityTriggered, EDeckSecurityType, SecurityType, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackSwitched, FName, JunctionID);
