// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TransportDeckTypes.h"
#include "TransportDeckComponent.generated.h"

/**
 * Player component for interacting with the Lower Transport Deck.
 * Handles deck entry/exit, cart boarding, cargo interaction, and
 * security evasion. Attach to the player character.
 *
 * Works alongside CrawlspaceComponent (under-car) and
 * ExteriorTraversalComponent (roof/between-car).
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UTransportDeckComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTransportDeckComponent();

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	bool bOnLowerDeck = false;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	bool bOnCart = false;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	int32 CurrentDeckCarIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "TransportDeck")
	FName CurrentCartID = NAME_None;

	// --- Configuration ---

	/** Base noise radius when walking on lower deck (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Noise")
	float BaseWalkNoise = 3.0f;

	/** Noise radius when crouching on lower deck */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Noise")
	float CrouchNoise = 1.0f;

	/** Noise radius when running on lower deck */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Noise")
	float RunNoise = 8.0f;

	/** Ambient noise masking from deck machinery (subtracted) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "TransportDeck|Noise")
	float MachineryMasking = 2.0f;

	// --- Interface ---

	/** Enter the lower deck via an access point */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool EnterDeck(int32 CarIndex, EDeckAccessType AccessType);

	/** Exit to upper deck via nearest access point */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool ExitDeck();

	/** Board a mini-rail cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool BoardCart(FName CartID);

	/** Dismount from current cart */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	void DismountCart();

	/** Loot a cargo container (returns items via inventory system) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool LootCargo(FName CargoID);

	/** Attempt to disable a security device (CUN check) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool DisableSecurity(AActor* SecurityDevice, int32 CunningScore);

	/** Switch a track junction (player interaction) */
	UFUNCTION(BlueprintCallable, Category = "TransportDeck")
	bool SwitchTrack(FName JunctionID, FName TargetBranch);

	// --- Queries ---

	/** Get effective noise radius based on current movement */
	UFUNCTION(BlueprintPure, Category = "TransportDeck|Noise")
	float GetCurrentNoiseRadius(bool bCrouching, bool bRunning) const;

	/** Check if player can enter deck at this car */
	UFUNCTION(BlueprintPure, Category = "TransportDeck")
	bool CanEnterDeckAt(int32 CarIndex) const;

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
	FOnCargoLooted OnCargoLooted;

	UPROPERTY(BlueprintAssignable, Category = "TransportDeck|Events")
	FOnSecurityTriggered OnSecurityTriggered;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateDeckCarTracking();
	void GenerateNoise(float Radius);

	/** Timer for security alert decay */
	FTimerHandle AlertDecayTimer;
};
