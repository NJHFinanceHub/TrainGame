// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TrainGame/Combat/CombatAIController.h"
#include "CompanionTypes.h"
#include "CompanionAIController.generated.h"

class UCompanionComponent;

// ============================================================================
// ACompanionAIController
//
// AI controller for companion characters. Extends CombatAIController with
// behavior mode support, corridor-aware navigation, and formation logic.
// ============================================================================

UCLASS()
class TRAINGAME_API ACompanionAIController : public ACombatAIController
{
	GENERATED_BODY()

public:
	ACompanionAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// --- Behavior Mode ---

	/** Update AI behavior based on companion's current mode */
	UFUNCTION(BlueprintCallable, Category = "Companion AI")
	void UpdateBehaviorMode(ECompanionBehavior NewBehavior);

	// --- Corridor Navigation ---

	/** Check if current corridor requires single-file formation */
	UFUNCTION(BlueprintPure, Category = "Companion AI|Navigation")
	bool IsInNarrowCorridor() const;

	/** Get the corridor width at the companion's current position */
	UFUNCTION(BlueprintPure, Category = "Companion AI|Navigation")
	float GetCurrentCorridorWidth() const;

	// --- Combat AI ---

	/** Select target based on behavior mode and threat assessment */
	UFUNCTION(BlueprintCallable, Category = "Companion AI|Combat")
	AActor* SelectCombatTarget() const;

	/** Determine optimal position based on behavior and formation */
	UFUNCTION(BlueprintCallable, Category = "Companion AI|Combat")
	FVector GetDesiredCombatPosition() const;

	/** Check if companion should use unique ability */
	UFUNCTION(BlueprintPure, Category = "Companion AI|Combat")
	bool ShouldUseUniqueAbility() const;

	// --- Formation ---

	/** Follow the player in formation, respecting corridor width */
	UFUNCTION(BlueprintCallable, Category = "Companion AI|Formation")
	void MoveToFormationPosition();

	/** Handle doorway stacking behavior when entering a new car */
	UFUNCTION(BlueprintCallable, Category = "Companion AI|Formation")
	void HandleDoorwayBehavior(FVector DoorLocation);

protected:
	/** Threshold for single-file mode (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Navigation")
	float NarrowCorridorThreshold = 2.0f;

	/** Distance to maintain from player in formation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Formation")
	float FormationFollowDistance = 200.0f;

	/** Distance player must advance into new car before companions follow through door */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Formation")
	float DoorwayAdvanceDistance = 300.0f;

	/** Duration of formation swap animation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Formation")
	float FormationSwapDuration = 2.0f;

	// --- Behavior-Specific Parameters ---

	/** How aggressively to pursue targets in Aggressive mode (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Behavior")
	float AggressivePursuitWeight = 0.9f;

	/** How close to stay to player in Defensive mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Behavior")
	float DefensivePlayerRadius = 150.0f;

	/** Damage threshold before Passive companion retaliates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion AI|Behavior")
	float PassiveRetaliationThreshold = 0.2f;

private:
	void UpdateAggressiveBehavior(float DeltaTime);
	void UpdateDefensiveBehavior(float DeltaTime);
	void UpdatePassiveBehavior(float DeltaTime);
	void UpdateHoldPositionBehavior(float DeltaTime);

	/** Cached reference to companion component */
	UPROPERTY()
	UCompanionComponent* CompanionComp = nullptr;

	FVector HoldPositionLocation = FVector::ZeroVector;
	bool bIsSwappingFormation = false;
	float SwapTimer = 0.0f;
};
