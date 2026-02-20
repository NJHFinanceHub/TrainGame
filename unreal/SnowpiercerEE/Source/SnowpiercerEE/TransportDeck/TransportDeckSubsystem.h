// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TransportDeckTypes.h"
#include "TransportDeckSubsystem.generated.h"

/**
 * Manages the Lower Transport Deck state: track network, cart routing,
 * cargo manifests, security alerts, and access point registration.
 * Runs beneath Cars 15-82 (Third Class through First Class).
 *
 * Integrates with SEECarStreamingSubsystem — lower deck geometry streams
 * alongside the upper car sublevels using a matching 3-segment window.
 */
UCLASS()
class SNOWPIERCEREE_API UTransportDeckSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Track Network ---

	/** Register a track segment (called during level load) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Track")
	void RegisterTrackSegment(const FTrackSegment& Segment);

	/** Get track segment by ID */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Track")
	bool GetTrackSegment(FName SegmentID, FTrackSegment& OutSegment) const;

	/** Switch a junction to a target branch */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Track")
	bool SwitchJunction(FName JunctionID, FName TargetSegmentID);

	/** Get all segments beneath a specific car */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Track")
	TArray<FTrackSegment> GetSegmentsForCar(int32 CarIndex) const;

	// --- Cart Management ---

	/** Register a cart actor with the subsystem */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cart")
	void RegisterCart(FName CartID, AActor* CartActor);

	/** Unregister a cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cart")
	void UnregisterCart(FName CartID);

	/** Dispatch a cart along a route (list of segment IDs) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cart")
	bool DispatchCart(FName CartID, const TArray<FName>& Route);

	/** Get all registered cart IDs */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Cart")
	TArray<FName> GetAllCartIDs() const;

	// --- Cargo ---

	/** Register cargo at a loading bay */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cargo")
	void RegisterCargo(const FCargoManifest& Manifest);

	/** Remove cargo (looted or delivered) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cargo")
	bool RemoveCargo(FName CargoID);

	/** Get all cargo at a specific car */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Cargo")
	TArray<FCargoManifest> GetCargoAtCar(int32 CarIndex) const;

	/** Get all contraband cargo */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Cargo")
	TArray<FCargoManifest> GetContrabandCargo() const;

	// --- Access Points ---

	/** Register an access point */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Access")
	void RegisterAccessPoint(const FDeckAccessConfig& Config);

	/** Get access points for a car */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Access")
	TArray<FDeckAccessConfig> GetAccessPointsForCar(int32 CarIndex) const;

	/** Check if a car has any unlocked access points */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Access")
	bool HasUnlockedAccess(int32 CarIndex) const;

	// --- Security ---

	/** Get current deck-wide alert level */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Security")
	EDeckAlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

	/** Raise alert level (security triggered) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Security")
	void RaiseAlert(EDeckAlertLevel Level);

	/** Reduce alert level (timer expired or player left area) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Security")
	void ReduceAlert();

	/** Trigger lockdown (all carts stop, access points seal) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Security")
	void TriggerLockdown();

	/** Clear lockdown */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Security")
	void ClearLockdown();

	// --- Queries ---

	/** Check if a car index has a lower deck (15-82 only) */
	UFUNCTION(BlueprintPure, Category = "TransportDeck")
	static bool HasLowerDeck(int32 CarIndex) { return CarIndex >= 15 && CarIndex <= 82; }

	/** Get the zone for a car index */
	UFUNCTION(BlueprintPure, Category = "TransportDeck")
	static ESEETrainZone GetZoneForCar(int32 CarIndex);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnDeckAlertChanged OnAlertChanged;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnTrackSwitched OnTrackSwitched;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnCargoLooted OnCargoLooted;

private:
	UPROPERTY()
	TMap<FName, FTrackSegment> TrackNetwork;

	UPROPERTY()
	TMap<FName, AActor*> RegisteredCarts;

	UPROPERTY()
	TArray<FCargoManifest> CargoManifests;

	UPROPERTY()
	TArray<FDeckAccessConfig> AccessPoints;

	UPROPERTY()
	EDeckAlertLevel CurrentAlertLevel = EDeckAlertLevel::Clear;

	UPROPERTY()
	bool bLockdownActive = false;

	/** Junction state: JunctionID -> currently active branch SegmentID */
	UPROPERTY()
	TMap<FName, FName> JunctionStates;
};
