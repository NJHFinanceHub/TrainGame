// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "TailZoneAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UTailZoneAudioComponent::UTailZoneAudioComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — one-shot scheduling doesn't need high frequency
}

void UTailZoneAudioComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTailZoneAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeactivateZoneAudio(0.f);
	Super::EndPlay(EndPlayReason);
}

void UTailZoneAudioComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive)
	{
		return;
	}

	// Random one-shot trigger
	NextOneShotTimer -= DeltaTime;
	if (NextOneShotTimer <= 0.f)
	{
		TriggerRandomOneShot();
		ScheduleNextOneShot();
	}

	// Revolutionary motif check
	if (MotifCooldown > 0.f)
	{
		MotifCooldown -= DeltaTime;
	}

	MotifTimer -= DeltaTime;
	if (MotifTimer <= 0.f && RevolutionProgress > 0.1f)
	{
		TriggerMotifCheck();
	}
}

void UTailZoneAudioComponent::ActivateZoneAudio()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;

	// Start ambient loops
	if (MetalRattleLoop)
	{
		ActiveLoops.Add(StartLoop(MetalRattleLoop, 0.4f));
	}
	if (WindWhistleLoop)
	{
		ActiveLoops.Add(StartLoop(WindWhistleLoop, 0.5f));
	}
	if (CrowdMurmurLoop)
	{
		ActiveLoops.Add(StartLoop(CrowdMurmurLoop, 0.25f));
	}
	if (TrainDroneLoop)
	{
		ActiveLoops.Add(StartLoop(TrainDroneLoop, 0.35f));
	}

	// Schedule first one-shot
	ScheduleNextOneShot();

	// Initialize motif timer
	MotifTimer = FMath::RandRange(30.f, 60.f);

	UE_LOG(LogTemp, Log, TEXT("Tail zone audio activated — %d ambient loops started"), ActiveLoops.Num());
}

void UTailZoneAudioComponent::DeactivateZoneAudio(float FadeOutDuration)
{
	bIsActive = false;

	for (UAudioComponent* Loop : ActiveLoops)
	{
		if (Loop)
		{
			if (FadeOutDuration > 0.f)
			{
				Loop->FadeOut(FadeOutDuration, 0.f);
			}
			else
			{
				Loop->Stop();
				Loop->DestroyComponent();
			}
		}
	}
	ActiveLoops.Empty();
}

void UTailZoneAudioComponent::SetRevolutionProgress(float Progress)
{
	RevolutionProgress = FMath::Clamp(Progress, 0.f, 1.f);
}

void UTailZoneAudioComponent::ScheduleNextOneShot()
{
	NextOneShotTimer = FMath::RandRange(MinOneShotInterval, MaxOneShotInterval);
}

void UTailZoneAudioComponent::TriggerRandomOneShot()
{
	if (!GetOwner())
	{
		return;
	}

	// Build a weighted pool of all one-shot categories
	// Pipes and metal stress are most common in the Tail
	struct FSoundPool
	{
		const TArray<USoundBase*>* Sounds;
		float Weight;
	};

	TArray<FSoundPool> Pools;
	Pools.Add({&PipeClankSounds, 3.f});
	Pools.Add({&ChainRattleSounds, 2.f});
	Pools.Add({&WaterDripSounds, 2.f});
	Pools.Add({&DistantVoiceSounds, 1.5f});
	Pools.Add({&MetalStressSounds, 1.5f});

	// Calculate total weight of non-empty pools
	float TotalWeight = 0.f;
	for (const FSoundPool& Pool : Pools)
	{
		if (Pool.Sounds && Pool.Sounds->Num() > 0)
		{
			TotalWeight += Pool.Weight;
		}
	}

	if (TotalWeight <= 0.f)
	{
		return;
	}

	// Weighted random selection
	float Roll = FMath::FRand() * TotalWeight;
	float Accumulated = 0.f;
	const TArray<USoundBase*>* SelectedPool = nullptr;

	for (const FSoundPool& Pool : Pools)
	{
		if (Pool.Sounds && Pool.Sounds->Num() > 0)
		{
			Accumulated += Pool.Weight;
			if (Roll <= Accumulated)
			{
				SelectedPool = Pool.Sounds;
				break;
			}
		}
	}

	if (!SelectedPool)
	{
		return;
	}

	USoundBase* Sound = PickRandomSound(*SelectedPool);
	if (!Sound)
	{
		return;
	}

	// Randomize volume and pitch within configured ranges
	float Volume = FMath::RandRange(OneShotVolumeRange.X, OneShotVolumeRange.Y);
	float Pitch = FMath::RandRange(OneShotPitchRange.X, OneShotPitchRange.Y);

	// Spatialized: play at a random offset from the player
	FVector Location = GetOwner()->GetActorLocation() + GetRandomSpatialOffset();
	UGameplayStatics::PlaySoundAtLocation(
		GetWorld(), Sound, Location, Volume, Pitch
	);
}

void UTailZoneAudioComponent::TriggerMotifCheck()
{
	if (MotifCooldown > 0.f || !RevolutionaryMotif || !GetOwner())
	{
		// Reset timer and return
		MotifTimer = FMath::RandRange(30.f, 90.f);
		return;
	}

	// Probability of motif playing increases with revolution progress
	// At 10% progress: ~5% chance per check
	// At 50% progress: ~25% chance
	// At 100% progress: ~50% chance
	float PlayChance = RevolutionProgress * 0.5f;

	if (FMath::FRand() < PlayChance)
	{
		// Play the revolutionary motif — quiet at first, louder as revolution builds
		float Volume = 0.2f + RevolutionProgress * 0.5f;
		float Pitch = 1.f; // Always at natural pitch — the motif must be recognizable

		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), RevolutionaryMotif,
			GetOwner()->GetActorLocation(),
			Volume, Pitch
		);

		// Cooldown scales inversely with progress (more frequent as revolution builds)
		MotifCooldown = FMath::Lerp(120.f, 30.f, RevolutionProgress);
	}

	// Schedule next check
	float BaseInterval = FMath::Lerp(60.f, 20.f, RevolutionProgress);
	MotifTimer = FMath::RandRange(BaseInterval * 0.5f, BaseInterval * 1.5f);
}

USoundBase* UTailZoneAudioComponent::PickRandomSound(const TArray<USoundBase*>& Sounds) const
{
	if (Sounds.Num() == 0)
	{
		return nullptr;
	}
	return Sounds[FMath::RandRange(0, Sounds.Num() - 1)];
}

FVector UTailZoneAudioComponent::GetRandomSpatialOffset() const
{
	// Generate a random position in the car around the player
	// Tail cars are narrow corridors — favor X (forward/back) and Z (up/down) over Y (lateral)
	float X = FMath::RandRange(-OneShotMaxDistance, OneShotMaxDistance);
	float Y = FMath::RandRange(-300.f, 300.f); // Narrow — train car width
	float Z = FMath::RandRange(-100.f, 200.f); // Slightly favoring above

	return FVector(X, Y, Z);
}

UAudioComponent* UTailZoneAudioComponent::StartLoop(USoundBase* Sound, float Volume)
{
	if (!Sound || !GetOwner())
	{
		return nullptr;
	}

	UAudioComponent* Comp = UGameplayStatics::SpawnSoundAttached(
		Sound,
		GetOwner()->GetRootComponent(),
		NAME_None,
		FVector::ZeroVector,
		EAttachLocation::KeepRelativeOffset,
		false,
		Volume,
		1.f,
		0.f,
		nullptr,
		nullptr,
		true
	);

	if (Comp)
	{
		Comp->bAutoDestroy = false;
		Comp->bIsUISound = true; // Non-spatialized ambient bed
	}

	return Comp;
}
