// MiniRailSubsystem.h
// Snowpiercer: Eternal Engine - Mini-rail track and cart management

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TransportDeck/TransportDeckTypes.h"
#include "MiniRailSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCartArrived, FName, CartID, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCartDerailed, FName, CartID, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCargoDelivered, FName, CartID, FName, CargoID, int32, DestinationCar);

/**
 * World subsystem managing the mini-rail network beneath cars 15-82.
 *
 * Responsibilities:
 * - Track topology: segments, junctions, termini
 * - Cart simulation: movement, speed, collisions
 * - Cargo routing: automated delivery schedules
 * - Security: patrol drone paths, tripwire placement
 *
 * The mini-rail is a single-track system with passing sidings.
 * Carts move automatically on schedules; the player can board
 * stationary carts and drive them manually.
 */
UCLASS()
class SNOWPIERCEREE_API UMiniRailSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Track Management ---

	/** Register a deck segment (called by level builder) */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Track")
	void RegisterSegment(const FTransportDeckSegment& Segment);

	/** Get segment data for a car index */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Track")
	FTransportDeckSegment GetSegment(int32 CarIndex) const;

	/** Get the junction type at a car index */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Track")
	ERailJunction GetJunctionAt(int32 CarIndex) const;

	/** Set junction direction (player or automated) */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Track")
	void SetJunction(int32 CarIndex, ERailJunction Direction);

	// --- Cart Management ---

	/** Spawn a new cart at a car index */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Cart")
	FName SpawnCart(int32 CarIndex);

	/** Get all carts in the system */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Cart")
	TArray<FMiniRailCart> GetAllCarts() const;

	/** Get a specific cart by ID */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Cart")
	FMiniRailCart GetCart(FName CartID) const;

	/** Start cart moving forward */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Cart")
	void StartCart(FName CartID, float Speed);

	/** Stop a cart */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Cart")
	void StopCart(FName CartID);

	/** Load cargo onto a cart */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Cart")
	bool LoadCargo(FName CartID, const FCargoManifest& Cargo);

	/** Unload cargo from a cart */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Cart")
	bool UnloadCargo(FName CartID, FName CargoID);

	/** Get carts near a world position (for boarding) */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Cart")
	TArray<FName> GetCartsNearPosition(FVector WorldPosition, float Radius = 500.0f) const;

	// --- Security ---

	/** Get security devices active in a segment */
	UFUNCTION(BlueprintPure, Category = "MiniRail|Security")
	TArray<EDeckSecurityType> GetSecurityAt(int32 CarIndex) const;

	/** Disable a security device at a segment */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Security")
	void DisableSecurity(int32 CarIndex, EDeckSecurityType Device);

	/** Re-enable security (after alert timer expires) */
	UFUNCTION(BlueprintCallable, Category = "MiniRail|Security")
	void ResetSecurity(int32 CarIndex);

	// --- Queries ---

	/** Get the controlling faction for a deck segment */
	UFUNCTION(BlueprintPure, Category = "MiniRail")
	ESEEFaction GetControllingFaction(int32 CarIndex) const;

	/** Check if a segment is a Kronole smuggling route */
	UFUNCTION(BlueprintPure, Category = "MiniRail")
	bool IsSmuggleRoute(int32 CarIndex) const;

	/** Get total number of registered segments */
	UFUNCTION(BlueprintPure, Category = "MiniRail")
	int32 GetSegmentCount() const { return Segments.Num(); }

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "MiniRail|Events")
	FOnCartArrived OnCartArrived;

	UPROPERTY(BlueprintAssignable, Category = "MiniRail|Events")
	FOnCartDerailed OnCartDerailed;

	UPROPERTY(BlueprintAssignable, Category = "MiniRail|Events")
	FOnCargoDelivered OnCargoDelivered;

private:
	void UpdateCarts(float DeltaTime);
	void AdvanceCart(FMiniRailCart& Cart, float DeltaTime);
	FName GenerateCartID();

	UPROPERTY()
	TMap<int32, FTransportDeckSegment> Segments;

	UPROPERTY()
	TMap<FName, FMiniRailCart> Carts;

	UPROPERTY()
	TMap<int32, TArray<EDeckSecurityType>> DisabledSecurity;

	int32 NextCartIndex = 0;
};
