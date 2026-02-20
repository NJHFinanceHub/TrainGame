// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthTypes.h"
#include "HidingSpotComponent.generated.h"

// ============================================================================
// UHidingSpotComponent
//
// Marks an actor as a hiding spot. Placed on lockers, bunks, cargo stacks,
// curtains, vents, and alcoves throughout the train. Tracks occupancy
// (player or body) and whether the spot can be found by searching NPCs.
// ============================================================================

UCLASS(ClassGroup=(Stealth), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UHidingSpotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHidingSpotComponent();

	/** Attempt to occupy this spot (returns false if already occupied) */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	bool Occupy(AActor* Occupant);

	/** Vacate the spot */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	void Vacate();

	/** Place a body in this spot */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	bool StoreBody(AActor* Body);

	/** Remove stored body */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	AActor* RetrieveBody();

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	bool IsOccupied() const { return CurrentOccupant != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	bool HasStoredBody() const { return StoredBody != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	bool IsAvailable() const { return !IsOccupied() && !HasStoredBody(); }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	EHidingSpotType GetSpotType() const { return SpotType; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	bool CanBeSearched() const { return bCanBeSearched; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	float GetSearchTime() const { return SearchTime; }

protected:
	virtual void BeginPlay() override;

	/** Type of hiding spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	EHidingSpotType SpotType = EHidingSpotType::Locker;

	/** Whether ALERTED+ NPCs can search this spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	bool bCanBeSearched = true;

	/** Time for an NPC to search this spot (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	float SearchTime = 30.f;

	/** Whether the occupant's legs are visible (curtains, partial cover) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	bool bPartialConcealment = false;

	/** Whether entering requires a skill check (ceiling vents) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	bool bRequiresSkillCheck = false;

	/** Skill check difficulty if required */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding", meta = (EditCondition = "bRequiresSkillCheck"))
	int32 SkillCheckDC = 12;

private:
	UPROPERTY()
	AActor* CurrentOccupant = nullptr;

	UPROPERTY()
	AActor* StoredBody = nullptr;
};
