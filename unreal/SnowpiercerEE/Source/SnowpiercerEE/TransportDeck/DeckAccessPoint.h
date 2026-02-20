// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransportDeckTypes.h"
#include "DeckAccessPoint.generated.h"

/**
 * World-placeable access point between upper passenger deck and
 * Lower Transport Deck. Types: floor hatches, freight elevators,
 * maintenance stairs, emergency ladders, vent shafts.
 *
 * Access points can be locked (requiring a key or CUN check),
 * generate noise on use, and optionally alert NPCs above.
 */
UCLASS()
class SNOWPIERCEREE_API ADeckAccessPoint : public AActor
{
	GENERATED_BODY()

public:
	ADeckAccessPoint();

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Access")
	FDeckAccessConfig Config;

	/** Whether this access point has been discovered by the player */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Access")
	bool bDiscovered = false;

	/** Whether the player has unlocked this access point */
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Access")
	bool bUnlocked = false;

	// --- Interface ---

	/** Attempt to use this access point (enter or exit lower deck) */
	UFUNCTION(BlueprintCallable, Category = "Access")
	bool TryUse(AActor* User, bool bGoingDown);

	/** Attempt to unlock (pick lock / hack panel) */
	UFUNCTION(BlueprintCallable, Category = "Access")
	bool TryUnlock(int32 CunningScore);

	/** Mark as discovered (shows on map) */
	UFUNCTION(BlueprintCallable, Category = "Access")
	void Discover();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Access")
	bool IsLocked() const { return Config.bLocked && !bUnlocked; }

	UFUNCTION(BlueprintPure, Category = "Access")
	EDeckAccessType GetAccessType() const { return Config.AccessType; }

	UFUNCTION(BlueprintPure, Category = "Access")
	int32 GetCarIndex() const { return Config.CarIndex; }

protected:
	virtual void BeginPlay() override;

private:
	void GenerateAccessNoise();
};
