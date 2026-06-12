#include "Audio/SEETrainFeelComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogSEETrainFeel, Log, All);

namespace
{
	const TCHAR* RailClackPath = TEXT("/Game/Audio/Foley/SFX_RailClack.SFX_RailClack");
}

USEETrainFeelComponent::USEETrainFeelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USEETrainFeelComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache every camera on the owner; each tick the sway targets whichever
	// one is active (first/third person toggle).
	if (AActor* Owner = GetOwner())
	{
		TArray<UCameraComponent*> Cameras;
		Owner->GetComponents<UCameraComponent>(Cameras);
		for (UCameraComponent* Camera : Cameras)
		{
			OwnerCameras.Add(Camera);
		}
	}

	NextJoltCountdown = FMath::FRandRange(JoltIntervalMin, JoltIntervalMax);
	ClackDriftCountdown = RailClackPitchDriftInterval;
}

void USEETrainFeelComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopRailClack();

	if (IsValid(LastSwayedCamera))
	{
		LastSwayedCamera->ClearAdditiveOffset();
	}
	LastSwayedCamera = nullptr;

	Super::EndPlay(EndPlayReason);
}

void USEETrainFeelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// NPC characters share the base character class — only the locally
	// player-controlled pawn gets camera sway and the 2D clack bed.
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const bool bLocalPlayer = OwnerPawn && OwnerPawn->IsPlayerControlled() && OwnerPawn->IsLocallyControlled();
	if (!bLocalPlayer)
	{
		StopRailClack();
		return;
	}

	// Lazy clack start: possession may happen after BeginPlay, and the wav may
	// not be imported yet (only try the load once per component).
	if (!RailClackComp && !bClackLoadAttempted)
	{
		bClackLoadAttempted = true;
		StartRailClack();
	}

	UpdateSway(DeltaTime);
	UpdateRailClack(DeltaTime);
}

// ---------------------------------------------------------------------------
// Camera sway
// ---------------------------------------------------------------------------

UCameraComponent* USEETrainFeelComponent::GetActiveCamera() const
{
	for (UCameraComponent* Camera : OwnerCameras)
	{
		if (IsValid(Camera) && Camera->IsActive())
		{
			return Camera;
		}
	}
	return nullptr;
}

void USEETrainFeelComponent::UpdateSway(float DeltaTime)
{
	UCameraComponent* ActiveCamera = bTrainSwayEnabled ? GetActiveCamera() : nullptr;

	// Camera switched (or sway disabled): remove our offset from the old one
	if (LastSwayedCamera && LastSwayedCamera != ActiveCamera && IsValid(LastSwayedCamera))
	{
		LastSwayedCamera->ClearAdditiveOffset();
	}
	LastSwayedCamera = ActiveCamera;

	if (!ActiveCamera)
	{
		return;
	}

	SwayTime += DeltaTime;

	const float TwoPi = 2.0f * PI;

	// Two summed roll sines (phase-offset so they never visibly align) + tiny Z bob
	float RollDeg =
		RollAmplitudePrimary * FMath::Sin(TwoPi * RollFrequencyPrimary * SwayTime) +
		RollAmplitudeSecondary * FMath::Sin(TwoPi * RollFrequencySecondary * SwayTime + 1.3f);
	const float BobZ = BobAmplitude * FMath::Sin(TwoPi * BobFrequency * SwayTime + 0.7f);

	// Occasional rail jolt: an extra roll kick that eases back out
	NextJoltCountdown -= DeltaTime;
	if (NextJoltCountdown <= 0.0f)
	{
		JoltTimeRemaining = JoltDecayTime;
		JoltSign = FMath::RandBool() ? 1.0f : -1.0f;
		NextJoltCountdown = FMath::FRandRange(JoltIntervalMin, JoltIntervalMax);
	}
	if (JoltTimeRemaining > 0.0f && JoltDecayTime > 0.0f)
	{
		JoltTimeRemaining = FMath::Max(0.0f, JoltTimeRemaining - DeltaTime);
		const float Alpha = JoltTimeRemaining / JoltDecayTime; // 1 -> 0
		RollDeg += JoltSign * JoltRollImpulse * Alpha * Alpha; // quadratic ease-out
	}

	// Additive camera-space offset: GetCameraView applies this AFTER the
	// bUsePawnControlRotation path has set the camera's base rotation, so the
	// sway composes with control rotation instead of being stomped by it.
	ActiveCamera->ClearAdditiveOffset();
	ActiveCamera->AddAdditiveOffset(
		FTransform(FRotator(0.0f, 0.0f, RollDeg), FVector(0.0f, 0.0f, BobZ)),
		/*FOV offset*/ 0.0f);
}

// ---------------------------------------------------------------------------
// Rail clack ambient
// ---------------------------------------------------------------------------

void USEETrainFeelComponent::StartRailClack()
{
	USoundBase* ClackSound = LoadObject<USoundBase>(nullptr, RailClackPath);
	if (!ClackSound)
	{
		// Asset is synthesized by an external pipeline and may not exist yet.
		static bool bWarnedMissingClack = false;
		if (!bWarnedMissingClack)
		{
			bWarnedMissingClack = true;
			UE_LOG(LogSEETrainFeel, Warning,
				TEXT("Rail clack sound not found (%s) — train ambient disabled for this session"),
				RailClackPath);
		}
		return;
	}

	if (USoundWave* Wave = Cast<USoundWave>(ClackSound))
	{
		// Authored seamless; force looping in memory (same pattern as the music beds).
		Wave->bLooping = true;
	}

	ClackPitchCurrent = 1.0f;
	ClackPitchTarget = FMath::FRandRange(RailClackPitchMin, RailClackPitchMax);

	RailClackComp = UGameplayStatics::SpawnSound2D(
		this, ClackSound, RailClackVolume, ClackPitchCurrent, 0.0f,
		/*ConcurrencySettings*/ nullptr,
		/*bPersistAcrossLevelTransition*/ false,
		/*bAutoDestroy*/ false);

	if (RailClackComp)
	{
		RailClackComp->bIsUISound = false; // respect game pause
		UE_LOG(LogSEETrainFeel, Log, TEXT("Rail clack ambient started at %.2f volume"), RailClackVolume);
	}
}

void USEETrainFeelComponent::StopRailClack()
{
	if (IsValid(RailClackComp))
	{
		RailClackComp->Stop();
	}
	RailClackComp = nullptr;
}

void USEETrainFeelComponent::UpdateRailClack(float DeltaTime)
{
	if (!IsValid(RailClackComp))
	{
		return;
	}

	// Pick a new drift target every interval; glide toward it slowly so the
	// loop never settles into a perfect metronome.
	ClackDriftCountdown -= DeltaTime;
	if (ClackDriftCountdown <= 0.0f)
	{
		ClackDriftCountdown = RailClackPitchDriftInterval;
		ClackPitchTarget = FMath::FRandRange(RailClackPitchMin, RailClackPitchMax);
	}

	ClackPitchCurrent = FMath::FInterpTo(ClackPitchCurrent, ClackPitchTarget, DeltaTime, RailClackPitchInterpSpeed);
	RailClackComp->SetPitchMultiplier(ClackPitchCurrent);
}
