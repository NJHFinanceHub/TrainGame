// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SnowpiercerEE/SEEHealthComponent.h"
#include "SEENPCAIController.generated.h"

class ACharacter;
class APawn;

/** High-level brain state for corridor NPCs. */
UENUM(BlueprintType)
enum class ESEENPCBrainState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Wander		UMETA(DisplayName = "Wander"),
	Chase		UMETA(DisplayName = "Chase"),
	ReturnHome	UMETA(DisplayName = "Return Home"),
	Dead		UMETA(DisplayName = "Dead")
};

// ============================================================================
// ASEENPCAIController
//
// Dependency-light brain for every placed NPC (jackboots, the boss, civilians,
// the merchant). No behavior trees, no UAIPerception — a 0.25s perception
// timer plus a small state machine driven from Tick.
//
// Hostiles (bHostile):
//   - Aggro on sight (range + frontal cone + occlusion line trace) or on
//     taking any damage.
//   - Chase via MoveToActor; if pathfinding fails or the pawn stalls (no
//     navmesh / partial path), falls back to straight AddMovementInput —
//     train corridors run along +X so direct steering works.
//   - In attack range: telegraphed swing (windup delay) on a randomized
//     cooldown, damage routed through the player's USEECombatComponent
//     (block/parry/dodge respected) then USEEHealthComponent.
//   - Loses sight for LoseSightDuration -> walks back to its spawn point.
//   - On death: ragdoll (or hide if the pawn has no mesh), collision/AI off,
//     corpse cleaned up after 20s.
//
// Friendlies (!bHostile):
//   - Short random wander around the spawn point every few seconds.
//   - Face the player when they are close, and stand still while in dialogue
//     (bInDialogue is set by the UI subsystem when a conversation opens).
//
// The brain reaches the placed Blueprint NPCs two ways:
//   1. ASEECivilianCharacter / ASEEJackbootCharacter set AIControllerClass to
//      this class with AutoPossessAI = PlacedInWorldOrSpawned (future BPs).
//   2. USEENPCBrainSubsystem adopts the existing plain-ACharacter BP_NPC_*
//      instances at runtime: spawns this controller, configures it per NPC
//      type and possesses the pawn.
// ============================================================================

UCLASS()
class SNOWPIERCEREE_API ASEENPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ASEENPCAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// --- Tunables (configured per NPC type by USEENPCBrainSubsystem) ---

	/** Hostile NPCs hunt the player; friendly NPCs wander and talk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain")
	bool bHostile = false;

	/** Max health applied to the runtime-attached health component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Perception")
	float SightRange = 2500.0f;

	/** Full frontal vision cone in degrees (ignored once already aggroed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Perception")
	float SightConeDegrees = 140.0f;

	/** Seconds without line of sight before a hostile gives up and leashes home. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Perception")
	float LoseSightDuration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float AttackDamage = 18.0f;

	/** Randomized seconds between swings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float AttackIntervalMin = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float AttackIntervalMax = 1.8f;

	/** Telegraph delay before the swing actually lands (dodge/block window). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float AttackWindupTime = 0.4f;

	/** Knockback impulse applied to the player when a swing connects. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Combat")
	float HitKnockbackImpulse = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Movement")
	float ChaseAcceptanceRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Movement")
	float ChaseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Movement")
	float WalkSpeed = 180.0f;

	/** Friendly idle-wander leash around the spawn point. Zero = stand still (merchant). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderRadius = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderIntervalMin = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderIntervalMax = 10.0f;

	/** Friendlies turn to face the player inside this range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float FacePlayerRange = 450.0f;

	/** Entry row in DT_Dialogue_Zone1 used when the player talks to this NPC. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Dialogue")
	FName DialogueEntryNode;

	// --- Queries / dialogue hooks ---

	UFUNCTION(BlueprintPure, Category = "NPCBrain")
	ESEENPCBrainState GetBrainState() const { return BrainState; }

	UFUNCTION(BlueprintPure, Category = "NPCBrain")
	bool IsAggroed() const { return BrainState == ESEENPCBrainState::Chase; }

	/** True when the player may open dialogue: alive, has an entry node, not mid-fight. */
	UFUNCTION(BlueprintPure, Category = "NPCBrain")
	bool CanStartDialogue() const;

	UFUNCTION(BlueprintCallable, Category = "NPCBrain")
	void SetInDialogue(bool bNowInDialogue);

	UFUNCTION(BlueprintPure, Category = "NPCBrain")
	bool IsInDialogue() const { return bInDialogue; }

	/** True once the brain subsystem (or pawn-class defaults) configured this controller. */
	bool IsConfigured() const { return bConfigured; }
	void MarkConfigured() { bConfigured = true; }

	/** Attach/refresh the health component, bind delegates and apply move speed. */
	void ApplyConfiguration();

private:
	// --- Perception / decisions (0.25s timer) ---
	void UpdatePerception();
	bool CanSeePlayer(const APawn* Player, bool bUseCone) const;

	// --- State transitions ---
	void StartChase();
	void StartReturnHome();
	void EnterIdle();

	// --- Per-tick movement ---
	void UpdateChase(float DeltaTime);
	void UpdateReturnHome();
	void UpdateDirectMove(float DeltaTime);
	void RequestMove(const FVector& GoalLocation, AActor* GoalActor, float AcceptanceRadius);
	void SetMoveSpeed(float Speed) const;

	// --- Attacks ---
	void BeginAttackWindup();
	void DeliverAttack();

	// --- Friendly wander ---
	void ScheduleNextWander();
	void OnWanderTimer();

	// --- Damage/death (dynamic delegate targets must be UFUNCTIONs) ---
	UFUNCTION()
	void HandlePawnDamaged(float Damage, ESEEDamageType DamageType, AActor* DamageInstigator);

	UFUNCTION()
	void HandlePawnDeath();

	ACharacter* GetPawnCharacter() const;
	APawn* GetPlayerPawn() const;
	bool IsPlayerAlive(const APawn* Player) const;

	// --- Runtime state ---
	ESEENPCBrainState BrainState = ESEENPCBrainState::Idle;
	FVector HomeLocation = FVector::ZeroVector;
	float TimeSinceSeenPlayer = 0.0f;
	float NextAttackTime = 0.0f;
	float StuckTime = 0.0f;
	float PathRetryCooldown = 0.0f;
	bool bWindingUp = false;
	bool bInDialogue = false;
	bool bConfigured = false;

	/** Straight-line fallback used when pathfinding fails (no/partial navmesh). */
	bool bDirectMoveActive = false;
	FVector DirectMoveGoal = FVector::ZeroVector;
	float DirectMoveTimeLeft = 0.0f;

	FTimerHandle PerceptionTimerHandle;
	FTimerHandle WanderTimerHandle;
	FTimerHandle AttackWindupTimerHandle;

	UPROPERTY()
	TObjectPtr<USEEHealthComponent> PawnHealth;
};
