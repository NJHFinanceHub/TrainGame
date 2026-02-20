// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SEENPCController.h"
#include "AITypes.h"
#include "JackbootAIController.generated.h"

// ============================================================================
// AJackbootAIController
//
// Specialized AI for Jackboot (security) NPCs. Extends SEENPCController with:
// - Alert propagation: vocal shouts + radio to nearby/all Jackboots
// - Backup calling: requests reinforcements from adjacent cars
// - Formation combat: coordinated attacks with squad discipline
// - Rank-based authority: higher ranks command lower ranks
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAlertPropagated, EAlertLevel, Level, FVector, Location, AJackbootAIController*, Source);

UCLASS()
class TRAINGAME_API AJackbootAIController : public ASEENPCController
{
	GENERATED_BODY()

public:
	AJackbootAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// --- Alert Propagation ---

	/** Raise alert to all nearby Jackboots via vocal shout */
	UFUNCTION(BlueprintCallable, Category = "JackbootAI|Alert")
	void RaiseVocalAlert(EAlertLevel Level, FVector TargetLocation);

	/** Raise alert via radio to all Jackboots in the car/train */
	UFUNCTION(BlueprintCallable, Category = "JackbootAI|Alert")
	void RaiseRadioAlert(EAlertLevel Level, FVector TargetLocation);

	/** Receive an alert from another Jackboot */
	UFUNCTION(BlueprintCallable, Category = "JackbootAI|Alert")
	void ReceiveJackbootAlert(EAlertLevel Level, FVector TargetLocation, AJackbootAIController* Source);

	/** Request backup from adjacent cars */
	UFUNCTION(BlueprintCallable, Category = "JackbootAI|Alert")
	void CallForBackup();

	UFUNCTION(BlueprintPure, Category = "JackbootAI|Alert")
	EAlertLevel GetAlertLevel() const { return CurrentAlertLevel; }

	// --- Formation Combat ---

	/** Whether this Jackboot should hold position and let others engage */
	UFUNCTION(BlueprintPure, Category = "JackbootAI|Formation")
	bool ShouldHoldFormation() const;

	/** Get the squad engagement slot for this Jackboot */
	UFUNCTION(BlueprintPure, Category = "JackbootAI|Formation")
	int32 GetEngagementSlot() const;

	/** Get desired formation position relative to squad leader */
	UFUNCTION(BlueprintPure, Category = "JackbootAI|Formation")
	FVector GetFormationPosition() const;

	// --- Rank ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Rank")
	EJackbootRank Rank = EJackbootRank::Grunt;

	/** Whether this Jackboot can command others (Sergeant+) */
	UFUNCTION(BlueprintPure, Category = "JackbootAI|Rank")
	bool CanIssueOrders() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "JackbootAI|Alert")
	FOnAlertPropagated OnAlertPropagated;

protected:
	/** Vocal alert radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Alert")
	float VocalAlertRadius = 2000.f;

	/** Whether this Jackboot has a radio */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Alert")
	bool bHasRadio = false;

	/** Delay before radio alert is sent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Alert")
	float RadioAlertDelay = 2.0f;

	/** Maximum number of Jackboots that can engage one target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Formation")
	int32 MaxSquadEngagers = 3;

	/** Distance between formation positions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "JackbootAI|Formation")
	float FormationSpacing = 150.f;

private:
	void TickAlertPropagation(float DeltaTime);
	void FindNearbyJackboots(TArray<AJackbootAIController*>& OutJackboots, float Radius) const;
	int32 CountEngagedSquadMembers() const;

	EAlertLevel CurrentAlertLevel = EAlertLevel::Green;

	bool bRadioAlertPending = false;
	float RadioAlertTimer = 0.f;
	FVector PendingAlertLocation = FVector::ZeroVector;
	EAlertLevel PendingAlertLevel = EAlertLevel::Green;

	bool bBackupCalled = false;
};
