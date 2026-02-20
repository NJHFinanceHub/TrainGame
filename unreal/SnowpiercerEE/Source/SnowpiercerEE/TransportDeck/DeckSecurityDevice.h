// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TransportDeckTypes.h"
#include "DeckSecurityDevice.generated.h"

/**
 * Automated security device on the Lower Transport Deck.
 * Detects unauthorized presence and raises alerts.
 *
 * Types:
 * - MotionSensor: Sweeps area, triggers on movement speed
 * - LaserTripwire: Static beam, instant trigger
 * - PatrolDrone: Moves along track, has vision cone
 * - PressurePlate: Floor-mounted, weight-triggered
 * - SecurityCamera: Fixed rotation, records evidence
 *
 * All devices can be disabled via CUN check or EMP.
 * Higher alert levels make devices harder to bypass.
 */
UCLASS()
class SNOWPIERCEREE_API ADeckSecurityDevice : public AActor
{
	GENERATED_BODY()

public:
	ADeckSecurityDevice();

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	EDeckSecurityType SecurityType = EDeckSecurityType::MotionSensor;

	/** Detection radius (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	float DetectionRadius = 8.0f;

	/** Detection cone half-angle for cameras/drones (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	float DetectionConeAngle = 45.0f;

	/** CUN difficulty to disable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	int32 DisableDifficulty = 8;

	/** Time before auto-reset after triggering (seconds, 0 = no reset) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	float ResetTime = 30.0f;

	/** Car index this device is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Security")
	int32 CarIndex = 0;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Security")
	bool bActive = true;

	UPROPERTY(BlueprintReadOnly, Category = "Security")
	bool bTriggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Security")
	bool bDisabled = false;

	// --- Interface ---

	/** Check if a location/actor triggers this device */
	UFUNCTION(BlueprintCallable, Category = "Security")
	bool CheckDetection(AActor* Target);

	/** Disable this device (from player CUN check) */
	UFUNCTION(BlueprintCallable, Category = "Security")
	bool Disable(int32 CunningScore);

	/** Temporarily jam the device (EMP, hacking) */
	UFUNCTION(BlueprintCallable, Category = "Security")
	void JamDevice(float Duration);

	/** Force-trigger the device (throwing objects, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Security")
	void ForceTrigger();

	/** Reset after being triggered */
	UFUNCTION(BlueprintCallable, Category = "Security")
	void Reset();

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Security|Events")
	FOnSecurityTriggered OnTriggered;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	void TriggerAlert();
	bool IsInDetectionCone(const FVector& TargetLocation) const;

	FTimerHandle ResetTimerHandle;
	FTimerHandle JamTimerHandle;

	bool bJammed = false;
};
