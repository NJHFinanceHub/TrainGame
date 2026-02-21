// TransportDeckComponent.h
// Snowpiercer: Eternal Engine - Lower Transport Deck player traversal

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TransportDeck/TransportDeckTypes.h"
#include "TransportDeckComponent.generated.h"

// FOnDeckEntered, FOnDeckExited, FOnCartBoarded, FOnCartExited declared in TransportDeckTypes.h
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSecurityTriggeredLocal, EDeckSecurityType, SecurityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, EDeckAlertLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCargoInteraction, FName, CargoID, ECargoType, CargoType);

/**
 * Manages player traversal of the Lower Transport Deck — the sub-level cargo
 * system running beneath cars 15-82 (Third Class through First Class).
 *
 * Features:
 * - Vertical access via hatches, freight elevators, maintenance stairs
 * - Mini-rail cart riding with track switching
 * - Stealth bypass of upper-level checkpoints
 * - Cargo inspection and smuggling interactions
 * - Automated security avoidance (lasers, sensors, drones, cameras)
 *
 * Attach to player character alongside CrawlspaceComponent and
 * ExteriorTraversalComponent.
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UTransportDeckComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTransportDeckComponent();

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	bool bOnTransportDeck = false;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	int32 CurrentCarIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	EDeckAlertLevel AlertLevel = EDeckAlertLevel::Clear;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	bool bRidingCart = false;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	FName CurrentCartID = NAME_None;

	// --- Configuration ---

	/** First car index that has a transport deck (inclusive) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Config")
	int32 DeckStartCar = 15;

	/** Last car index that has a transport deck (inclusive) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Config")
	int32 DeckEndCar = 82;

	/** Player walk speed on transport deck (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Movement")
	float WalkSpeed = 3.0f;

	/** Player crouch speed on transport deck (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Movement")
	float CrouchSpeed = 1.5f;

	/** Player sprint speed on transport deck (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Movement")
	float SprintSpeed = 5.0f;

	/** Base noise radius when walking (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Stealth")
	float BaseNoiseRadius = 5.0f;

	/** Noise from machinery that masks player movement (subtracted) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Stealth")
	float MachineryMasking = 3.0f;

	/** Time in seconds before alert level decays one step */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Security")
	float AlertDecayTime = 30.0f;

	// --- Access ---

	/** Enter the transport deck via an access point */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	void EnterTransportDeck(int32 CarIndex, EDeckAccessType AccessType);

	/** Exit the transport deck to the upper level */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	void ExitTransportDeck();

	/** Check if a car index has transport deck access */
	UFUNCTION(BlueprintPure, Category = "TransportDeck")
	bool HasTransportDeck(int32 CarIndex) const;

	/** Check if a specific access type is available at a car */
	UFUNCTION(BlueprintPure, Category = "TransportDeck")
	bool HasAccessPoint(int32 CarIndex, EDeckAccessType AccessType) const;

	// --- Mini-Rail ---

	/** Board a stationary mini-rail cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|MiniRail")
	bool BoardCart(FName CartID);

	/** Disembark from the current cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|MiniRail")
	void DisembarkCart();

	/** Set the cart speed (0 = stop, negative = reverse) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|MiniRail")
	void SetCartSpeed(float SpeedMs);

	/** Switch track junction ahead of cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|MiniRail")
	bool SwitchJunction(ERailJunction NewDirection);

	/** Get the current cart data */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|MiniRail")
	FMiniRailCart GetCurrentCart() const;

	/** Check if near a boardable cart */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|MiniRail")
	bool IsNearCart() const;

	// --- Cargo ---

	/** Inspect cargo on the current cart or in the current segment */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cargo")
	TArray<FCargoManifest> InspectCargo() const;

	/** Attempt to take contraband from a cart (CUN check) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cargo")
	bool TakeContraband(FName CargoID, int32 CunningScore);

	/** Plant contraband on a cart (for smuggling missions) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Cargo")
	bool PlantContraband(FName CargoID, FName CartID);

	// --- Security ---

	/** Get the current security devices in this segment */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Security")
	TArray<EDeckSecurityType> GetActiveSecurityDevices() const;

	/** Attempt to disable a security device (CUN or tech skill check) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck|Security")
	bool DisableSecurityDevice(EDeckSecurityType Device, int32 CunningScore);

	/** Get current alert level */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Security")
	EDeckAlertLevel GetAlertLevel() const { return AlertLevel; }

	/** Get noise radius for current movement mode */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Stealth")
	float GetCurrentNoiseRadius() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnDeckEntered OnDeckEntered;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnDeckExited OnDeckExited;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnCartBoarded OnCartBoarded;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnCartExited OnCartExited;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnSecurityTriggeredLocal OnSecurityTriggered;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnAlertLevelChanged OnAlertLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnCargoInteraction OnCargoInteraction;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateCarTracking();
	void UpdateAlertDecay(float DeltaTime);
	void GenerateNoise(float Radius);
	void TriggerSecurity(EDeckSecurityType Device);
	void SetAlertLevel(EDeckAlertLevel NewLevel);

	/** Timer for alert level decay */
	float AlertDecayTimer = 0.0f;

	/** Whether player is crouching */
	bool bCrouching = false;

	/** Whether player is sprinting */
	bool bSprinting = false;
};
