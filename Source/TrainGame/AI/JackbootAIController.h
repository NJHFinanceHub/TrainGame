// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEEAIController.h"
#include "JackbootAIController.generated.h"

// ============================================================================
// AJackbootAIController
//
// Specialized AI for Jackboot (security) NPCs. Extends the base SEE AI
// controller with alert propagation, backup calling, and formation combat.
// Jackboots are the trained military force of the train — disciplined,
// coordinated, and dangerous in groups.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAlertLevelChanged, EJackbootAlertLevel, OldLevel, EJackbootAlertLevel, NewLevel);

UCLASS()
class TRAINGAME_API AJackbootAIController : public ASEEAIController
{
	GENERATED_BODY()

public:
	AJackbootAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// --- Alert System ---

	/** Raise the alert level for this Jackboot and propagate to nearby units */
	UFUNCTION(BlueprintCallable, Category = "AI|Jackboot")
	void RaiseAlert(EJackbootAlertLevel NewLevel, FVector ThreatLocation);

	/** Receive an alert from another Jackboot */
	UFUNCTION(BlueprintCallable, Category = "AI|Jackboot")
	void ReceiveJackbootAlert(EJackbootAlertLevel Level, FVector ThreatLocation, AActor* Caller);

	/** Call for backup — alerts all Jackboots within radio range */
	UFUNCTION(BlueprintCallable, Category = "AI|Jackboot")
	void CallBackup(FVector ThreatLocation);

	/** Get current alert level */
	UFUNCTION(BlueprintPure, Category = "AI|Jackboot")
	EJackbootAlertLevel GetAlertLevel() const { return AlertLevel; }

	// --- Formation ---

	/** Get the current formation type */
	UFUNCTION(BlueprintPure, Category = "AI|Jackboot")
	EJackbootFormation GetFormation() const { return CurrentFormation; }

	/** Set formation type (called by squad leader) */
	UFUNCTION(BlueprintCallable, Category = "AI|Jackboot")
	void SetFormation(EJackbootFormation NewFormation);

	/** Get the desired position within the current formation */
	UFUNCTION(BlueprintPure, Category = "AI|Jackboot")
	FVector GetFormationPosition() const;

	/** Whether this Jackboot is the squad leader */
	UFUNCTION(BlueprintPure, Category = "AI|Jackboot")
	bool IsSquadLeader() const { return bIsSquadLeader; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "AI|Jackboot")
	FOnAlertLevelChanged OnAlertLevelChanged;

protected:
	/** Radio range for backup calls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	float RadioRange = 10000.f;

	/** Vocal alert radius (shouting to nearby units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	float VocalRange = 2000.f;

	/** Whether this Jackboot has a radio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	bool bHasRadio = true;

	/** Whether this is the squad leader */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	bool bIsSquadLeader = false;

	/** Formation index within squad (0 = point, 1 = left, 2 = right, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	int32 FormationIndex = 0;

	/** Time before alert level de-escalates (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	float AlertCooldownDuration = 60.f;

	/** Distance between formation members */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Jackboot")
	float FormationSpacing = 200.f;

	virtual void InitializeBlackboard() override;

private:
	void UpdateAlertCooldown(float DeltaTime);
	void PropagateAlert(EJackbootAlertLevel Level, FVector ThreatLocation);
	TArray<AJackbootAIController*> FindNearbyJackboots(float Range) const;

	EJackbootAlertLevel AlertLevel = EJackbootAlertLevel::Green;
	EJackbootFormation CurrentFormation = EJackbootFormation::SingleFile;
	FVector LastThreatLocation = FVector::ZeroVector;
	float AlertCooldownTimer = 0.f;
};
