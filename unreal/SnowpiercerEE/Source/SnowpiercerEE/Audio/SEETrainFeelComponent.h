#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEETrainFeelComponent.generated.h"

class UCameraComponent;
class UAudioComponent;

// ============================================================================
// USEETrainFeelComponent
//
// Continuous "you are on a moving train" feel for the owning character.
// Only does work for the locally player-controlled pawn (NPCs share the
// base character class — the component idles on them).
//
//  - CAMERA SWAY: two summed roll sines plus a small vertical bob, with an
//    occasional rail-joint jolt (a roll kick that eases back out). Applied
//    via UCameraComponent::AddAdditiveOffset on the ACTIVE camera each tick:
//    the additive offset composes in camera space AFTER bUsePawnControlRotation
//    has set the camera's base rotation, so it never fights control rotation
//    (writing the relative rotation directly would be stomped every frame by
//    the pawn-control-rotation path in GetCameraView).
//  - RAIL CLACK: a low-volume looping 2D ambient (SFX_RailClack, authored
//    seamless) whose pitch drifts slowly between random targets so the
//    rhythm never metronomes. Loaded defensively — the asset may not be
//    synthesized/imported yet; warns once and stays silent.
// ============================================================================
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEETrainFeelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEETrainFeelComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Sway tuning ---

	/** Master toggle for the camera sway (the rail clack keeps playing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainFeel|Sway")
	bool bTrainSwayEnabled = true;

	/** Primary (slow) roll sine amplitude, degrees. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float RollAmplitudePrimary = 0.35f;

	/** Primary roll sine frequency, Hz. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float RollFrequencyPrimary = 0.9f;

	/** Secondary (fast) roll sine amplitude, degrees. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float RollAmplitudeSecondary = 0.15f;

	/** Secondary roll sine frequency, Hz. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float RollFrequencySecondary = 2.3f;

	/** Vertical camera bob amplitude, cm. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float BobAmplitude = 1.2f;

	/** Vertical camera bob frequency, Hz. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Sway")
	float BobFrequency = 1.8f;

	// --- Jolt tuning (occasional rail joints / points) ---

	/** Peak roll of an occasional lateral jolt, degrees (random sign). */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Jolt")
	float JoltRollImpulse = 0.6f;

	/** How long a jolt takes to ease back to zero, seconds. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Jolt")
	float JoltDecayTime = 0.8f;

	/** Minimum seconds between jolts. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Jolt")
	float JoltIntervalMin = 7.0f;

	/** Maximum seconds between jolts. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|Jolt")
	float JoltIntervalMax = 15.0f;

	// --- Rail clack ambient ---

	/** Volume of the looping rail clack bed. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|RailClack")
	float RailClackVolume = 0.18f;

	/** Lower bound of the slow pitch drift. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|RailClack")
	float RailClackPitchMin = 0.97f;

	/** Upper bound of the slow pitch drift. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|RailClack")
	float RailClackPitchMax = 1.03f;

	/** Seconds between picking a new pitch drift target. */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|RailClack")
	float RailClackPitchDriftInterval = 10.0f;

	/** Interp speed toward the current pitch target (low = imperceptible glide). */
	UPROPERTY(EditAnywhere, Category = "TrainFeel|RailClack")
	float RailClackPitchInterpSpeed = 0.3f;

private:
	void UpdateSway(float DeltaTime);
	void UpdateRailClack(float DeltaTime);
	void StartRailClack();
	void StopRailClack();
	UCameraComponent* GetActiveCamera() const;

	/** All camera components on the owner, cached at BeginPlay. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCameraComponent>> OwnerCameras;

	/** Camera we last pushed an additive offset onto (cleared on switch/disable). */
	UPROPERTY(Transient)
	TObjectPtr<UCameraComponent> LastSwayedCamera;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> RailClackComp;

	bool bClackLoadAttempted = false;
	float SwayTime = 0.0f;
	float NextJoltCountdown = 0.0f;
	float JoltTimeRemaining = 0.0f;
	float JoltSign = 1.0f;
	float ClackPitchCurrent = 1.0f;
	float ClackPitchTarget = 1.0f;
	float ClackDriftCountdown = 0.0f;
};
