// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SnowpiercerEE/SEEHealthComponent.h"
#include "SEENPCAIController.generated.h"

class ACharacter;
class APawn;
class USEEAnimDriverComponent;

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
//   - Chase via MoveToActor; the first Failed request marks navigation as
//     unavailable (degenerate/missing navmesh) and the chase switches to
//     continuous straight AddMovementInput steering — train corridors run
//     along +X so direct steering works. Pathfinding is re-tested every
//     NavRetestInterval seconds.
//   - In attack range: telegraphed swing (windup delay) on a randomized
//     cooldown, damage routed through the player's USEECombatComponent
//     (block/parry/dodge respected) then USEEHealthComponent.
//   - Loses sight for LoseSightDuration -> walks back to its spawn point.
//   - On death: ragdoll (or hide if the pawn has no mesh), collision/AI off,
//     corpse cleaned up after 20s.
//
// Friendlies (!bHostile):
//   - Short random wander around the spawn point every few seconds. Works
//     with OR without a navmesh: goals are validated by a floor line trace
//     and a capsule-radius sweep, then walked via direct AddMovementInput
//     steering when pathfinding is unavailable (arrive radius / timeout /
//     stall detection included). Goals are clamped to the car interior.
//   - Ambient life while idle: occasional in-place turns toward nearby NPCs
//     or small random yaw drift so groups feel alive.
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

	/** Idle seconds between wander legs (randomized). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderIntervalMin = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderIntervalMax = 9.0f;

	/** A wander leg counts as arrived inside this distance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderArriveRadius = 80.0f;

	/** Give up on a wander leg after this many seconds (stuck on props/pawns). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderMoveTimeout = 4.0f;

	/** Wander goals are clamped to |Y| below this (train-car interior half width). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float WanderYClamp = 1500.0f;

	/** Friendlies turn to face the player inside this range. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float FacePlayerRange = 450.0f;

	/** Chance that a wander timer fires an in-place ambient turn instead of a walk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float AmbientTurnChance = 0.35f;

	/** Ambient turns prefer facing another NPC inside this range (groups look alive). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float AmbientFaceNPCRange = 600.0f;

	/** Max random yaw drift (degrees, either direction) for an ambient turn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Wander")
	float AmbientYawDriftMax = 75.0f;

	/** After a pathed move fails (no navmesh), re-test pathfinding this often. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPCBrain|Movement")
	float NavRetestInterval = 30.0f;

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
	void UpdateReturnHome(float DeltaTime);
	void UpdateWander(float DeltaTime);
	void UpdateDirectMove(float DeltaTime);
	void RequestMove(const FVector& GoalLocation, AActor* GoalActor, float AcceptanceRadius);
	void StartDirectMove(const FVector& Goal, float Duration, float StopDist);
	void SetMoveSpeed(float Speed) const;

	/** True while pathfinding is known-broken and the retest interval has not elapsed. */
	bool IsNavBlocked() const;

	// --- Attacks ---
	void BeginAttackWindup();
	void DeliverAttack();

	// --- Friendly wander / ambient life ---
	void ScheduleNextWander();
	void OnWanderTimer();

	/** Pick a validated wander goal: random point in the leash, Y-clamped to the car
	  * interior, floor confirmed by a downward line trace, capsule-radius sweep so
	  * the straight-line approach is not buried in a wall. */
	bool TryPickWanderPoint(FVector& OutGoal) const;

	/** Idle flavor: turn in place — face a nearby NPC or drift yaw randomly. */
	void DoAmbientTurn();

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

	/** Straight-line steering used when pathfinding fails (no/partial navmesh). */
	bool bDirectMoveActive = false;
	FVector DirectMoveGoal = FVector::ZeroVector;
	float DirectMoveTimeLeft = 0.0f;
	float DirectMoveStopDist = 70.0f;
	float DirectStuckTime = 0.0f;

	/** Set when a pathed move request returns Failed (degenerate/missing navmesh).
	  * All movement steers directly until NextNavRetestTime re-tests pathfinding. */
	bool bNavUnavailable = false;
	float NextNavRetestTime = 0.0f;

	/** Goal of the wander leg in flight (pathed or direct). */
	FVector ActiveWanderGoal = FVector::ZeroVector;
	float WanderElapsed = 0.0f;

	FTimerHandle PerceptionTimerHandle;
	FTimerHandle WanderTimerHandle;
	FTimerHandle AttackWindupTimerHandle;

	UPROPERTY()
	TObjectPtr<USEEHealthComponent> PawnHealth;

	/** Code-driven animation for the possessed pawn (locomotion + one-shots).
	 *  Spawned and initialized on possess so adopted plain-ACharacter NPCs animate. */
	UPROPERTY()
	TObjectPtr<USEEAnimDriverComponent> PawnAnimDriver;

	/** Attach + init the anim driver on the possessed pawn (idempotent). */
	void EnsurePawnAnimDriver();
};
