// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TrainGame/Core/CombatTypes.h"
#include "CombatAIController.generated.h"

class UCombatComponent;
class UWeaponComponent;

// ============================================================================
// ACombatAIController
//
// AI for enemies in tight train corridors. Supports 6 enemy profiles with
// distinct combat behaviors:
// - Desperate (Tailies): Wild, unpredictable, poor defense
// - Disciplined (Jackboots): Formation fighters, coordinated attacks
// - Cunning (Smugglers): Dirty fighters, environmental exploitation
// - Brute (Heavies): Slow, devastating, hard to stagger
// - Captain (Jackboot Officers): Command buffs, ranged + melee switching
// - Zealot (Order of Wilford): Fanatical, relentless, self-sacrifice
// ============================================================================

/** Combat AI behavior profile */
UENUM(BlueprintType)
enum class ECombatAIProfile : uint8
{
	/** Desperate, aggressive, low skill - Tail fighters */
	Desperate	UMETA(DisplayName = "Desperate"),

	/** Trained, disciplined, uses formations - Jackboots */
	Disciplined	UMETA(DisplayName = "Disciplined"),

	/** Uses environment, flanking, dirty tricks - Smugglers/Kronole Network */
	Cunning		UMETA(DisplayName = "Cunning"),

	/** Slow but powerful, hard to stagger - Laborers/Heavies */
	Brute		UMETA(DisplayName = "Brute"),

	/** Jackboot officers - commands nearby troops, switches melee/ranged */
	Captain		UMETA(DisplayName = "Captain"),

	/** Order of Wilford zealots - fanatical, relentless, suicidal charges */
	Zealot		UMETA(DisplayName = "Zealot"),

	/** First Class personal guards - precise, defensive, counter-attackers */
	FirstClassGuard	UMETA(DisplayName = "First Class Guard")
};

UCLASS()
class TRAINGAME_API ACombatAIController : public AAIController
{
	GENERATED_BODY()

public:
	ACombatAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	/** Get this AI's combat profile */
	UFUNCTION(BlueprintPure, Category = "CombatAI")
	ECombatAIProfile GetProfile() const { return Profile; }

	/** Set combat profile (recalculates modifiers) */
	UFUNCTION(BlueprintCallable, Category = "CombatAI")
	void SetProfile(ECombatAIProfile NewProfile);

protected:
	/** AI behavior profile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	ECombatAIProfile Profile = ECombatAIProfile::Desperate;

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

	/** Chance to use ranged attack when in range (for Captain/Guard profiles) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float RangedAttackChance = 0.f;

	/** Preferred range for ranged attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float PreferredRangedRange = 800.f;

	/** Chance for Zealots to perform suicidal charge at low health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float SuicidalChargeChance = 0.f;

	/** Health threshold below which Zealots become frenzied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI", meta = (ClampMin = "0", ClampMax = "1"))
	float FrenzyHealthThreshold = 0.3f;

	/** Captain command radius — buffs nearby allies within this range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float CommandRadius = 500.f;

	/** Counter-attack chance after perfect block (First Class Guard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatAI")
	float CounterAttackChance = 0.f;

private:
	void MakeDecision();
	void MoveToTarget();
	void PerformAttack();
	void PerformRangedAttack();
	void ConsiderBlock();
	void ConsiderEnvironmentalKill();
	void ApplyProfileModifiers();

	// Profile-specific behaviors
	void ExecuteCaptainBehavior();
	void ExecuteZealotBehavior();
	void ExecuteGuardBehavior();
	void BuffNearbyAllies();

	/** Find the player or nearest enemy */
	AActor* FindTarget() const;

	/** Check if there's an environmental hazard near the target */
	AActor* FindNearbyHazard(AActor* NearActor) const;

	UPROPERTY()
	UCombatComponent* CombatComp = nullptr;

	UPROPERTY()
	UWeaponComponent* WeaponComp = nullptr;

	UPROPERTY()
	AActor* CurrentTarget = nullptr;

	float DecisionTimer = 0.f;
	bool bIsAttacking = false;
	bool bIsApproaching = false;
	bool bIsFrenzied = false;

	/** Corridor-aware: tracks how many other AI are already engaging the same target */
	int32 GetEngagingAICount() const;
};
