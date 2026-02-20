// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthTypes.h"
#include "StealthComponent.generated.h"

class UDisguiseComponent;

// ============================================================================
// UStealthComponent
//
// Attached to the player character. Tracks visibility, movement noise,
// composure while hiding, and interaction with hiding spots. This is the
// player-side counterpart to NPC DetectionComponents.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVisibilityChanged, float, NewVisibility);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComposureChanged, float, NewComposure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnteredHidingSpot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitedHidingSpot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakedownCompleted, const FTakedownResult&, Result);

UCLASS(ClassGroup=(Stealth), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UStealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Noise Generation ---

	/** Generate a noise event at the player's location */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Noise")
	void GenerateNoise(float Radius, ESoundIntensity Intensity);

	/** Get footstep noise radius for current surface and movement mode */
	UFUNCTION(BlueprintPure, Category = "Stealth|Noise")
	float GetFootstepNoiseRadius() const;

	// --- Hiding ---

	/** Attempt to enter a hiding spot */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	bool EnterHidingSpot(AActor* HidingSpot);

	/** Exit the current hiding spot */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Hiding")
	void ExitHidingSpot();

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	bool IsHiding() const { return bIsHiding; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Hiding")
	float GetComposure() const { return Composure; }

	// --- Takedowns ---

	/** Attempt a stealth takedown on a target NPC */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Takedown")
	FTakedownResult AttemptTakedown(AActor* Target, bool bLethal = false);

	/** Check if a stealth takedown is possible on the target */
	UFUNCTION(BlueprintPure, Category = "Stealth|Takedown")
	bool CanPerformTakedown(const AActor* Target) const;

	// --- Body Interaction ---

	/** Begin dragging a body */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Body")
	bool StartDragBody(AActor* Body);

	/** Stop dragging a body */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Body")
	void StopDragBody();

	/** Place a body into a hiding spot */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Body")
	bool PlaceBodyInHidingSpot(AActor* Body, AActor* HidingSpot);

	/** Restrain an unconscious NPC */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Body")
	bool RestrainTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Stealth|Body")
	bool IsDraggingBody() const { return bIsDraggingBody; }

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Stealth")
	EStealthMovement GetCurrentMovementMode() const { return CurrentMovement; }

	UFUNCTION(BlueprintPure, Category = "Stealth")
	float GetCurrentVisibility() const;

	UFUNCTION(BlueprintPure, Category = "Stealth")
	int32 GetKillCount() const { return KillCount; }

	UFUNCTION(BlueprintPure, Category = "Stealth")
	int32 GetNonLethalTakedownCount() const { return NonLethalTakedownCount; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Stealth")
	FOnVisibilityChanged OnVisibilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stealth")
	FOnComposureChanged OnComposureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stealth")
	FOnEnteredHidingSpot OnEnteredHidingSpot;

	UPROPERTY(BlueprintAssignable, Category = "Stealth")
	FOnExitedHidingSpot OnExitedHidingSpot;

	UPROPERTY(BlueprintAssignable, Category = "Stealth")
	FOnTakedownCompleted OnTakedownCompleted;

protected:
	virtual void BeginPlay() override;

	// --- Hiding ---

	/** Time to enter/exit a hiding spot (interruptible) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	float HidingTransitionTime = 1.5f;

	/** Composure drain rate per second when NPC is within 5m */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	float ComposureDrainRate = 10.f;

	/** Distance at which nearby NPCs drain composure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Hiding")
	float ComposureDrainDistance = 500.f;

	// --- Takedowns ---

	/** Maximum distance for stealth takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Takedown")
	float TakedownRange = 150.f;

	/** Duration of standard stealth takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Takedown")
	float TakedownDuration = 2.f;

	/** Duration of lethal stealth takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Takedown")
	float LethalTakedownDuration = 3.5f;

	/** Noise radius of a stealth takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Takedown")
	float TakedownNoiseRadius = 300.f;

	// --- Body Dragging ---

	/** Movement speed multiplier while dragging a body */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Body")
	float DragSpeedMultiplier = 0.4f;

	/** How long drag marks remain visible (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Body")
	float DragMarkDuration = 60.f;

private:
	void UpdateMovementMode();
	void UpdateComposure(float DeltaTime);
	void UpdateFootstepNoise(float DeltaTime);
	float CalculateEnvironmentalMasking() const;

	EStealthMovement CurrentMovement = EStealthMovement::Stationary;
	ESurfaceType CurrentSurface = ESurfaceType::MetalGrating;

	bool bIsHiding = false;
	float Composure = 100.f;

	bool bIsDraggingBody = false;
	UPROPERTY()
	AActor* DraggedBody = nullptr;

	UPROPERTY()
	AActor* CurrentHidingSpot = nullptr;

	int32 KillCount = 0;
	int32 NonLethalTakedownCount = 0;

	float FootstepNoiseTimer = 0.f;
};
