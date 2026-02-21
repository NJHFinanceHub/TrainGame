// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TrainGame/Core/CombatTypes.h"
#include "SEECombatAIController.generated.h"

class USEECombatComponent;

// ============================================================================
// ASEECombatAIController
//
// Basic AI for melee enemies in tight train corridors.
// Enemies in Snowpiercer fight differently than in open fields:
// - They can't surround you if you control the corridor width
// - They form queues and take turns or try to flank through vents
// - They use the environment (push you into hazards)
// - Jackboots are disciplined; Tailies are desperate
// ============================================================================

/** Combat AI behavior profile */
UENUM(BlueprintType)
enum class ESEECombatAIProfile : uint8
{
	/** Desperate, aggressive, low skill - Tail fighters */
	Desperate	UMETA(DisplayName = "Desperate"),

	/** Trained, disciplined, uses formations - Jackboots */
	Disciplined	UMETA(DisplayName = "Disciplined"),

	/** Uses environment, flanking, dirty tricks - Smugglers/Kronole Network */
	Cunning		UMETA(DisplayName = "Cunning"),

	/** Slow but powerful, hard to stagger - Laborers/Heavies */
	Brute		UMETA(DisplayName = "Brute")
};

UCLASS()
class SNOWPIERCEREE_API ASEECombatAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASEECombatAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

protected:
	/** AI behavior profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	ESEECombatAIProfile Profile = ESEECombatAIProfile::Desperate;

	/** How close the AI tries to get before attacking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float PreferredCombatRange = 180.f;

	/** How often the AI re-evaluates its decision (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float DecisionInterval = 0.5f;

	/** Chance to block when being attacked (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float BlockChance = 0.3f;

	/** Chance to dodge when being attacked (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float DodgeChance = 0.1f;

	/** Chance to use environmental hazard when near one (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float EnvironmentalUseChance = 0.15f;

	/** Maximum number of AI that can attack a single target simultaneously */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	int32 MaxSimultaneousAttackers = 2;

private:
	void MakeDecision();
	void MoveToTarget();
	void PerformAttack();
	void ConsiderBlock();
	void ConsiderEnvironmentalKill();
	void ApplyProfileModifiers();

	/** Find the player or nearest enemy */
	AActor* FindTarget() const;

	/** Check if there's an environmental hazard near the target */
	AActor* FindNearbyHazard(AActor* NearActor) const;

	UPROPERTY()
	USEECombatComponent* CombatComp = nullptr;

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	float DecisionTimer = 0.f;
	bool bIsAttacking = false;
	bool bIsApproaching = false;

	/** Corridor-aware: tracks how many other AI are already engaging the same target */
	int32 GetEngagingAICount() const;
};
