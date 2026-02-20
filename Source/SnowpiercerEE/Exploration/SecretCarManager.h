// SecretCarManager.h
// Snowpiercer: Eternal Engine - Secret car discovery and access management

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CollectibleTypes.h"
#include "SecretCarManager.generated.h"

/** Identifier for each secret car */
UENUM(BlueprintType)
enum class ESecretCar : uint8
{
	BunkerCar       UMETA(DisplayName = "The Bunker Car"),
	GhostCar        UMETA(DisplayName = "Ghost Car"),
	RebelArchive    UMETA(DisplayName = "Rebel Archive"),
	OutsideLab      UMETA(DisplayName = "The Outside Lab"),
	StowawayCar     UMETA(DisplayName = "Stowaway Car"),
	MirrorCar       UMETA(DisplayName = "The Mirror Car"),
	PrototypeCar    UMETA(DisplayName = "Prototype Car"),
	Nursery         UMETA(DisplayName = "The Nursery"),
	DeadMansSwitch  UMETA(DisplayName = "Dead Man's Switch")
};

/** Access requirement for a secret car */
USTRUCT(BlueprintType)
struct FSecretCarRequirement
{
	GENERATED_BODY()

	/** Minimum zone the player must have reached */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ETrainZone MinimumZone = ETrainZone::Zone1_Tail;

	/** Required stat checks (stat name -> minimum value) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> StatRequirements;

	/** Required collectible IDs (must have collected these) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FName> RequiredCollectibles;

	/** Required companion (empty = no companion needed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName RequiredCompanion;

	/** Required companion loyalty minimum */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredLoyalty = 0;

	/** Required faction standing (faction -> minimum standing) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> FactionRequirements;

	/** Whether EVA/cold suit is required */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequiresColdSuit = false;

	/** Whether this is only accessible via rooftop */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequiresRooftopAccess = false;

	/** Whether this is only accessible via crawlspace */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bRequiresCrawlspaceAccess = false;

	/** Narrative flag that must be set (empty = no flag needed) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NarrativeFlag;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecretCarDiscovered, ESecretCar, Car);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecretCarMilestone, int32, TotalDiscovered);

/**
 * Manages secret car discovery state, access validation, and cumulative rewards.
 */
UCLASS()
class USecretCarManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Check if all requirements are met for a secret car */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	bool CanAccessSecretCar(ESecretCar Car) const;

	/** Mark a secret car as discovered (call when player enters) */
	UFUNCTION(BlueprintCallable, Category = "SecretCars")
	void DiscoverSecretCar(ESecretCar Car);

	/** Check if a secret car has been discovered */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	bool IsDiscovered(ESecretCar Car) const;

	/** Get total number of discovered secret cars */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	int32 GetDiscoveredCount() const;

	/** Check if all 9 secret cars have been found (True Ending eligible) */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	bool AllCarsDiscovered() const;

	/** Get the requirements for a specific secret car */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	FSecretCarRequirement GetRequirements(ESecretCar Car) const;

	/** Check which requirements are met/unmet for a car */
	UFUNCTION(BlueprintPure, Category = "SecretCars")
	TArray<FText> GetUnmetRequirements(ESecretCar Car) const;

	// Cumulative reward queries

	/** Whether "Pathfinder" title is earned (3+ cars) */
	UFUNCTION(BlueprintPure, Category = "SecretCars|Rewards")
	bool HasPathfinderTitle() const;

	/** Whether "Sixth Sense" passive is active (5+ cars) */
	UFUNCTION(BlueprintPure, Category = "SecretCars|Rewards")
	bool HasSixthSense() const;

	/** Whether "Master Explorer" speed bonus is active (7+ cars) */
	UFUNCTION(BlueprintPure, Category = "SecretCars|Rewards")
	bool HasMasterExplorer() const;

	/** Whether "Ghost of the Train" is earned (9 cars — all) */
	UFUNCTION(BlueprintPure, Category = "SecretCars|Rewards")
	bool HasGhostOfTheTrain() const;

	// Events

	UPROPERTY(BlueprintAssignable, Category = "SecretCars|Events")
	FOnSecretCarDiscovered OnSecretCarDiscovered;

	UPROPERTY(BlueprintAssignable, Category = "SecretCars|Events")
	FOnSecretCarMilestone OnSecretCarMilestone;

protected:
	UPROPERTY()
	TMap<ESecretCar, bool> DiscoveryState;

	UPROPERTY()
	TMap<ESecretCar, FSecretCarRequirement> Requirements;

private:
	void InitializeRequirements();
	void CheckMilestones();
};
