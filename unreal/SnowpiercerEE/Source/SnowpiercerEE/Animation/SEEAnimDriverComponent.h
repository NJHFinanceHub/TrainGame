// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEAnimDriverComponent.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UCharacterMovementComponent;

/**
 * One-shot action the driver can play over locomotion.
 * Each maps to a named AnimSequence resolved from the owner mesh's model folder.
 */
UENUM(BlueprintType)
enum class ESEEAnimAction : uint8
{
	None		UMETA(DisplayName = "None"),
	Attack		UMETA(DisplayName = "Attack"),	// resolves to Slash / Punch / Stab
	HitReact	UMETA(DisplayName = "Hit React"),
	Death		UMETA(DisplayName = "Death")
};

// ============================================================================
// USEEAnimDriverComponent
//
// Code-driven, AnimBlueprint-free character animation. Drives the owner's
// USkeletalMeshComponent in EAnimationMode::AnimationSingleNode and swaps the
// looping locomotion clip (Idle/Walk/Run) every tick from ground speed, while
// letting PlayAction() interrupt with a non-looping one-shot (Attack/HitReact/
// Death) that locks out locomotion for the clip's length.
//
// Anim assets are resolved at init from the SAME package folder the owner's
// skeletal mesh lives in (e.g. /Game/Characters/QuaterniusZombieApocalypse/
// Characters_Sam) so each model plays its own skeleton-bound sequences. All
// loads are defensive: missing clips warn once and degrade to "no anim" rather
// than crash or T-pose-restart every frame.
//
// Works for the player (ASEEPlayerCharacter wires it from BeginPlay) and for
// adopted plain-ACharacter NPCs (ASEENPCAIController attaches+inits it on
// possess). If the owner mesh begins simulating physics (death ragdoll), the
// driver stops driving so the two systems never fight.
// ============================================================================
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEAnimDriverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEAnimDriverComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	/** Resolve the mesh + anim set and switch the mesh to single-node mode.
	 *  Safe to call again (re-resolves). Auto-runs from BeginPlay; the AI
	 *  controller calls it explicitly after possessing so adopted NPCs animate. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void InitDriver();

	/** Play a non-looping one-shot that suppresses locomotion for its duration.
	 *  Death holds its last frame and never returns to locomotion. */
	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayAction(ESEEAnimAction Action);

	/** True once a Death action has played — locomotion is permanently off. */
	UFUNCTION(BlueprintPure, Category = "Animation")
	bool IsDead() const { return bDeathPlayed; }

	/** The resolved Death sequence (may be null). Callers use its length to time
	 *  a deferred physics ragdoll so the keyframed death reads first. */
	UAnimSequence* GetDeathAnim() const { return DeathAnim; }

	// --- Locomotion thresholds (cm/s of 2D ground speed) ---

	/** Below this the owner is considered standing still -> Idle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
	float IdleSpeedThreshold = 10.0f;

	/** At/above this -> Run; between Idle and this -> Walk. When zero or
	 *  negative the run threshold is derived from the owner's MaxWalkSpeed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
	float RunSpeedThreshold = 0.0f;

	/** Fraction of MaxWalkSpeed used as the run threshold when RunSpeedThreshold
	 *  is auto (<=0). 0.66 -> jog past two-thirds of top speed reads as Run. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Locomotion")
	float RunThresholdSpeedFraction = 0.66f;

protected:
	// --- Resolved at init from the owner mesh's model folder ---

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> IdleAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> WalkAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> RunAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> SlashAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> PunchAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> StabAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> HitReactAnim;

	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> DeathAnim;

private:
	/** Find the owner's skeletal mesh component (the Character's GetMesh()). */
	USkeletalMeshComponent* ResolveMeshComponent() const;

	/** Derive the package folder of the assigned skeletal mesh and load the
	 *  named sequences from it. Returns false if no mesh/folder. */
	bool ResolveAnimSet();

	/** Load one AnimSequence from a model folder by trying a set of name
	 *  variants ("<Model>_<Name>", "<Name>", then a substring scan). */
	UAnimSequence* LoadSequenceFromFolder(const FString& FolderPath,
		const FString& ModelName, const FString& BaseName) const;

	/** Pick the locomotion clip for a 2D ground speed (may be null). */
	UAnimSequence* SelectLocomotionAnim(float GroundSpeed) const;

	/** Drive the single-node player only when the desired looping clip changes. */
	void PlayLooping(UAnimSequence* Seq);

	/** True when the owner mesh is ragdolling — the driver must yield. */
	bool IsMeshSimulatingPhysics() const;

	/** Currently selected looping clip (so we only restart on change). */
	UPROPERTY(Transient)
	TObjectPtr<UAnimSequence> CurrentLoopingAnim;

	/** Remaining seconds a one-shot action suppresses locomotion. */
	float ActionLockTimer = 0.0f;

	bool bInitialized = false;
	bool bDeathPlayed = false;
	bool bWarnedNoMesh = false;
	bool bWarnedNoAnims = false;

	/** Alternate Slash/Punch on successive Attack actions for variety. */
	bool bAttackUsePunch = false;
};
