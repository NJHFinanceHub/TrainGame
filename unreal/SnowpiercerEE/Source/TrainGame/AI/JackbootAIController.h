// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NPCAIController.h"
#include "JackbootAIController.generated.h"

class UAlertPropagationSubsystem;

// ============================================================================
// AJackbootAIController
//
// AI controller for Jackboot (security) NPCs. Extends NPCAIController with:
// - Alert propagation: notifies nearby Jackboots when threats detected
// - Backup calling: requests reinforcements via radio/vocal alert
// - Formation combat: coordinates attacks in disciplined formations
// - Body discovery: reacts to found unconscious/dead NPCs
// ============================================================================

UCLASS()
class TRAINGAME_API AJackbootAIController : public ANPCAIController
{
	GENERATED_BODY()

public:
	AJackbootAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// --- Alert Propagation ---

	/** Propagate alert to nearby Jackboots */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Alert")
	void PropagateAlert(EAlertLevel Level, FVector ThreatLocation, AActor* ThreatActor = nullptr);

	/** Receive alert from another Jackboot */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Alert")
	void ReceiveAlert(EAlertLevel Level, FVector ThreatLocation, AActor* ThreatInstigator);

	/** Get current alert level */
	UFUNCTION(BlueprintPure, Category = "Jackboot AI|Alert")
	EAlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

	// --- Backup ---

	/** Call for backup — finds Jackboots within radio range */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Backup")
	void CallForBackup(FVector ThreatLocation);

	/** Respond to a backup call */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Backup")
	void RespondToBackup(FVector RallyPoint);

	/** Check if this Jackboot is responding to a backup call */
	UFUNCTION(BlueprintPure, Category = "Jackboot AI|Backup")
	bool IsRespondingToBackup() const { return bRespondingToBackup; }

	// --- Formation Combat ---

	/** Get optimal combat position relative to target and allies */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Formation")
	FVector GetFormationCombatPosition() const;

	/** Check if we should wait for formation before engaging */
	UFUNCTION(BlueprintPure, Category = "Jackboot AI|Formation")
	bool ShouldWaitForFormation() const;

	/** Get the number of allied Jackboots nearby */
	UFUNCTION(BlueprintPure, Category = "Jackboot AI|Formation")
	int32 GetNearbyAllyCount() const;

	// --- Body Discovery ---

	/** React to discovering a body */
	UFUNCTION(BlueprintCallable, Category = "Jackboot AI|Body")
	void OnBodyDiscovered(AActor* Body, EBodyState BodyState);

protected:
	/** Range for vocal alert propagation (without radio) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Alert")
	float VocalAlertRange = 1500.f;

	/** Range for radio alert propagation (with radio equipment) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Alert")
	float RadioAlertRange = 5000.f;

	/** Whether this Jackboot has a radio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Alert")
	bool bHasRadio = true;

	/** Minimum allies to form up before engaging (0 = engage immediately) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Formation")
	int32 MinFormationSize = 2;

	/** Maximum time to wait for formation before engaging solo (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Formation")
	float MaxFormationWaitTime = 8.f;

	/** Desired spacing between Jackboots in formation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Formation")
	float FormationSpacing = 150.f;

	/** Cooldown between alert propagations to prevent spam */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jackboot AI|Alert")
	float AlertCooldown = 5.f;

private:
	void TickFormationCombat(float DeltaTime);
	void TickBackupResponse(float DeltaTime);
	void UpdateAlertLevel();

	/** Find all Jackboots within a given range */
	TArray<AJackbootAIController*> FindNearbyJackboots(float Range) const;

	EAlertLevel CurrentAlertLevel = EAlertLevel::Green;
	FVector LastThreatLocation = FVector::ZeroVector;
	FVector BackupRallyPoint = FVector::ZeroVector;
	float FormationWaitTimer = 0.f;
	float AlertCooldownTimer = 0.f;
	bool bRespondingToBackup = false;
	bool bWaitingForFormation = false;
};
