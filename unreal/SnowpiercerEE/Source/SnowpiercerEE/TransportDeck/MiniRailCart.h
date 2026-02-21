// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransportDeckTypes.h"
#include "MiniRailCart.generated.h"

/**
 * A rideable mini-rail cart on the Lower Transport Deck.
 *
 * Carts travel along registered track segments, carrying cargo between
 * loading bays. The player can board, ride, and dismount carts. At
 * junctions, the player can switch tracks to redirect the cart.
 *
 * Carts generate noise proportional to speed — riding a cart is faster
 * than walking the lower deck but louder. Carts halt during lockdown.
 */
UCLASS()
class SNOWPIERCEREE_API AMiniRailCart : public AActor
{
	GENERATED_BODY()

public:
	AMiniRailCart();

	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cart")
	FName CartID;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Cart")
	EMiniRailCartState CartState = EMiniRailCartState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Cart")
	float CurrentSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Cart")
	int32 CurrentCarIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Cart")
	bool bPlayerOnBoard = false;

	UPROPERTY(BlueprintReadOnly, Category = "Cart")
	FName CurrentSegmentID = NAME_None;

	// --- Configuration ---

	/** Maximum speed (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Movement")
	float MaxSpeed = 5.0f;

	/** Acceleration (m/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Movement")
	float Acceleration = 2.0f;

	/** Braking deceleration (m/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Movement")
	float BrakeDeceleration = 4.0f;

	/** Noise radius at max speed (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Noise")
	float MaxSpeedNoiseRadius = 12.0f;

	/** Noise radius when idle (loading/unloading) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Noise")
	float IdleNoiseRadius = 2.0f;

	/** Cargo capacity (weight units) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cart|Cargo")
	float CargoCapacity = 200.0f;

	// --- Cargo ---

	UPROPERTY(BlueprintReadOnly, Category = "Cart|Cargo")
	TArray<FCargoManifest> LoadedCargo;

	UPROPERTY(BlueprintReadOnly, Category = "Cart|Cargo")
	float CurrentCargoWeight = 0.0f;

	// --- Interface ---

	/** Start moving along the assigned route */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void StartMoving();

	/** Apply brakes */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void ApplyBrakes();

	/** Emergency stop */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void EmergencyStop();

	/** Load cargo onto cart */
	UFUNCTION(BlueprintCallable, Category = "Cart|Cargo")
	bool LoadCargo(const FCargoManifest& Cargo);

	/** Unload specific cargo */
	UFUNCTION(BlueprintCallable, Category = "Cart|Cargo")
	bool UnloadCargo(FName CargoID);

	/** Set the route (list of segment IDs to follow) */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void SetRoute(const TArray<FName>& NewRoute);

	/** Player boards this cart */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void OnPlayerBoard();

	/** Player dismounts this cart */
	UFUNCTION(BlueprintCallable, Category = "Cart")
	void OnPlayerDismount();

	// --- Queries ---

	/** Get noise radius at current speed */
	UFUNCTION(BlueprintPure, Category = "Cart|Noise")
	float GetCurrentNoiseRadius() const;

	/** Get remaining cargo capacity */
	UFUNCTION(BlueprintPure, Category = "Cart|Cargo")
	float GetRemainingCapacity() const { return CargoCapacity - CurrentCargoWeight; }

	/** Check if cart can move (not locked, not derailed, track powered) */
	UFUNCTION(BlueprintPure, Category = "Cart")
	bool CanMove() const;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateMovement(float DeltaTime);
	void UpdateCarTracking();
	void GenerateMovementNoise();

	/** Assigned route segments */
	UPROPERTY()
	TArray<FName> Route;

	/** Current index in the route */
	UPROPERTY()
	int32 RouteIndex = 0;

	/** Progress along current segment (0.0 to 1.0) */
	UPROPERTY()
	float SegmentProgress = 0.0f;

	/** Time since last noise event */
	float NoiseCooldown = 0.0f;
};
