// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthTypes.h"
#include "DetectionComponent.generated.h"

// ============================================================================
// UDetectionComponent
//
// Attached to NPCs. Handles sight and sound perception of the player.
// Drives the detection meter and state machine (Unaware -> Suspicious ->
// Alerted -> Combat). Corridor-aware: vision cones clamp to narrow spaces.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDetectionStateChanged, EDetectionState, OldState, EDetectionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionMeterChanged, float, NewMeterValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoiseHeard, const FNoiseEvent&, NoiseEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyDiscovered, AActor*, Body);

UCLASS(ClassGroup=(Stealth), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UDetectionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDetectionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Perception ---

	/** Process a noise event. Returns true if the NPC heard it. */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Detection")
	bool ProcessNoiseEvent(const FNoiseEvent& NoiseEvent);

	/** Check if a target actor is within the vision cone */
	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	bool IsTargetInVisionCone(const AActor* Target) const;

	/** Check if there is line of sight to a target (accounts for obstacles) */
	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	bool HasLineOfSightTo(const AActor* Target) const;

	/** Notify this NPC that a body has been found */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Detection")
	void NotifyBodyDiscovered(AActor* Body, EBodyState BodyState);

	// --- Alert Propagation ---

	/** Receive an alert from another NPC (vocal or radio) */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Detection")
	void ReceiveAlert(EDetectionState MinimumState, FVector InvestigationPoint);

	/** Whether this NPC has a radio (enables car-wide alert propagation) */
	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	bool HasRadio() const { return bHasRadio; }

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	EDetectionState GetDetectionState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	float GetDetectionMeter() const { return DetectionMeter; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	FVector GetLastKnownTargetLocation() const { return LastKnownTargetLocation; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Detection")
	bool IsHeightened() const { return bHeightenedAwareness; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Detection")
	FOnDetectionStateChanged OnDetectionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Detection")
	FOnDetectionMeterChanged OnDetectionMeterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Detection")
	FOnNoiseHeard OnNoiseHeard;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Detection")
	FOnBodyDiscovered OnBodyDiscovered;

protected:
	virtual void BeginPlay() override;

	// --- Vision ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Vision")
	FVisionConeConfig VisionConfig;

	// --- Detection Meter ---

	/** Current detection meter value (0-100) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stealth|Detection")
	float DetectionMeter = 0.f;

	/** Rate at which the detection meter drains when target is out of sight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Detection")
	float MeterDrainRate = 15.f;

	/** Delay before meter starts draining after losing sight */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Detection")
	float MeterDrainDelay = 1.5f;

	// --- Alert ---

	/** Whether this NPC has a radio for car-wide alert propagation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Alert")
	bool bHasRadio = false;

	/** Delay before radio alert is transmitted (gives player time to intervene) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Alert")
	float RadioAlertDelay = 3.f;

	/** Vocal alert radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Alert")
	float VocalAlertRadius = 1500.f;

	// --- Search Behavior ---

	/** How long this NPC searches when SUSPICIOUS before returning to patrol */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Search")
	float SuspiciousSearchDuration = 20.f;

	/** How long this NPC searches when ALERTED before de-escalating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Search")
	float AlertedSearchDuration = 180.f;

	/** How long target must be out of sight before dropping from COMBAT */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Search")
	float CombatLoseSightDuration = 30.f;

	// --- Disguise Awareness ---

	/** Whether this NPC is a home zone resident (better at spotting disguises) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	bool bIsHomeZoneResident = true;

	/** Whether this NPC is security (best at spotting disguises) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	bool bIsSecurity = false;

private:
	void UpdateSightDetection(float DeltaTime);
	void UpdateDetectionState();
	void UpdateSearchTimer(float DeltaTime);
	void DrainDetectionMeter(float DeltaTime);
	void PropagateVocalAlert();
	void PropagateRadioAlert();

	float CalculateSightFillRate(float Distance, bool bInInnerCone) const;
	float GetLightingModifier() const;
	float GetMovementModifier(const AActor* Target) const;
	float GetDisguiseModifier(const AActor* Target) const;

	EDetectionState CurrentState = EDetectionState::Unaware;
	FVector LastKnownTargetLocation = FVector::ZeroVector;

	/** Heightened awareness flag -- set after entering COMBAT, increases detection by 50% */
	bool bHeightenedAwareness = false;

	float TimeSinceLostSight = 0.f;
	float SearchTimer = 0.f;
	float RadioAlertTimer = 0.f;
	bool bRadioAlertPending = false;

	int32 RecentSoundCount = 0;
	float RecentSoundTimer = 0.f;
};
