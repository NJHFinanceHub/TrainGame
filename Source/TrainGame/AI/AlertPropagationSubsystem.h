// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AITypes.h"
#include "AlertPropagationSubsystem.generated.h"

// ============================================================================
// UAlertPropagationSubsystem
//
// World subsystem that manages alert state across the entire train.
// Tracks per-zone alert levels, handles alert escalation/de-escalation,
// and provides query functions for AI controllers.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneAlertChanged,
	ETrainZone, Zone,
	EAlertLevel, NewLevel);

UCLASS()
class TRAINGAME_API UAlertPropagationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// --- Zone Alert Management ---

	/** Get the alert level for a specific zone */
	UFUNCTION(BlueprintPure, Category = "Alert System")
	EAlertLevel GetZoneAlertLevel(ETrainZone Zone) const;

	/** Set the alert level for a zone (will not de-escalate below current) */
	UFUNCTION(BlueprintCallable, Category = "Alert System")
	void EscalateZoneAlert(ETrainZone Zone, EAlertLevel Level, FVector ThreatLocation);

	/** Force-set alert level (can de-escalate) */
	UFUNCTION(BlueprintCallable, Category = "Alert System")
	void SetZoneAlertLevel(ETrainZone Zone, EAlertLevel Level);

	/** Reset all zones to Green */
	UFUNCTION(BlueprintCallable, Category = "Alert System")
	void ResetAllAlerts();

	/** Get the last known threat location for a zone */
	UFUNCTION(BlueprintPure, Category = "Alert System")
	FVector GetZoneThreatLocation(ETrainZone Zone) const;

	// --- Alert Propagation ---

	/** Propagate alert from one zone to adjacent zones (with reduced intensity) */
	UFUNCTION(BlueprintCallable, Category = "Alert System")
	void PropagateToAdjacentZones(ETrainZone SourceZone);

	// --- De-escalation ---

	/** Start de-escalation timer for a zone (alert will gradually decrease) */
	UFUNCTION(BlueprintCallable, Category = "Alert System")
	void BeginDeescalation(ETrainZone Zone);

	UPROPERTY(BlueprintAssignable, Category = "Alert System")
	FOnZoneAlertChanged OnZoneAlertChanged;

protected:
	/** Time before alert level drops one step (seconds) */
	UPROPERTY(EditAnywhere, Category = "Alert System")
	float DeescalationTime = 60.f;

private:
	struct FZoneAlertState
	{
		EAlertLevel Level = EAlertLevel::Green;
		FVector ThreatLocation = FVector::ZeroVector;
		float DeescalationTimer = -1.f; // Negative = not de-escalating
	};

	TMap<ETrainZone, FZoneAlertState> ZoneAlerts;

	/** Get adjacent zones for propagation */
	TArray<ETrainZone> GetAdjacentZones(ETrainZone Zone) const;

	/** Reduce alert level by one step */
	EAlertLevel ReduceAlertLevel(EAlertLevel Level) const;
};
