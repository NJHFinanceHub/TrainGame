// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "AdaptiveMusicComponent.h"
#include "ZoneAudioProfile.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

UAdaptiveMusicComponent::UAdaptiveMusicComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20Hz for smooth fades

	// Default state transitions matching design doc
	StateTransitions.Add({EMusicState::Exploration, EMusicState::Suspicion, 2.f, false});
	StateTransitions.Add({EMusicState::Suspicion, EMusicState::Exploration, 4.f, false});
	StateTransitions.Add({EMusicState::Suspicion, EMusicState::Detection, 0.f, true}); // immediate
	StateTransitions.Add({EMusicState::Detection, EMusicState::Suspicion, 3.f, false});
	StateTransitions.Add({EMusicState::Detection, EMusicState::Combat, 0.5f, true}); // smash cut
	StateTransitions.Add({EMusicState::Detection, EMusicState::Exploration, 6.f, false}); // full escape
	StateTransitions.Add({EMusicState::Combat, EMusicState::PostCombat, 2.f, false});
	StateTransitions.Add({EMusicState::PostCombat, EMusicState::Exploration, 4.f, false});
	StateTransitions.Add({EMusicState::PostCombat, EMusicState::MoralChoice, 3.f, false});
}

void UAdaptiveMusicComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeLayerStates();
}

void UAdaptiveMusicComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up all audio components
	for (FMusicLayerState& State : LayerStates)
	{
		for (UAudioComponent* Comp : State.ActiveStemComponents)
		{
			if (Comp)
			{
				Comp->Stop();
				Comp->DestroyComponent();
			}
		}
		State.ActiveStemComponents.Empty();
	}

	if (MoralChoiceTone)
	{
		MoralChoiceTone->Stop();
		MoralChoiceTone->DestroyComponent();
		MoralChoiceTone = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UAdaptiveMusicComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Auto-detect state changes from gameplay parameters
	EMusicState DetectedState = DetermineStateFromGameplay();
	if (DetectedState != CurrentSnapshot.CurrentState)
	{
		// Only auto-transition for awareness-driven states, not narrative
		if (CurrentSnapshot.CurrentState != EMusicState::MoralChoice)
		{
			TransitionToState(DetectedState);
		}
	}

	// Update intensity from game state
	UpdateIntensityFromGameState();

	// Update layer volumes (smooth crossfades)
	UpdateLayerVolumes(DeltaTime);

	// Update stem activation based on intensity
	UpdateStemActivation(DeltaTime);

	// Handle post-combat timer
	if (CurrentSnapshot.CurrentState == EMusicState::PostCombat)
	{
		PostCombatTimer -= DeltaTime;
		if (PostCombatTimer <= 0.f)
		{
			TransitionToState(EMusicState::Exploration);
		}
	}
}

void UAdaptiveMusicComponent::InitializeLayerStates()
{
	int32 LayerCount = static_cast<int32>(EMusicLayer::Count);
	LayerStates.SetNum(LayerCount);

	for (int32 i = 0; i < LayerCount; ++i)
	{
		LayerStates[i].CurrentVolume = 0.f;
		LayerStates[i].TargetVolume = 0.f;
		LayerStates[i].FadeSpeed = 1.f;
		LayerStates[i].bActive = false;
	}

	// Zone base is always active at minimum
	LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].bActive = true;
	LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.3f;
}

// --- Zone Management ---

void UAdaptiveMusicComponent::SetAudioZone(EAudioZone NewZone)
{
	if (NewZone == CurrentSnapshot.CurrentZone)
	{
		return;
	}

	EAudioZone OldZone = CurrentSnapshot.CurrentZone;
	CurrentSnapshot.CurrentZone = NewZone;

	// Look up new zone profile
	if (UZoneAudioProfile** FoundProfile = ZoneProfiles.Find(NewZone))
	{
		UZoneAudioProfile* OldProfile = ActiveProfile;
		ActiveProfile = *FoundProfile;

		// Crossfade zone-specific layers
		float CrossfadeDuration = ActiveProfile ? ActiveProfile->ZoneEntryCrossfadeDuration : 3.f;

		// Tear down old zone stems and spawn new ones
		CleanupLayerStems(EMusicLayer::ZoneBase);
		CleanupLayerStems(EMusicLayer::Exploration);
		CleanupLayerStems(EMusicLayer::Tension);
		CleanupLayerStems(EMusicLayer::Combat);

		if (ActiveProfile)
		{
			SpawnStemsForLayer(EMusicLayer::ZoneBase, ActiveProfile->ZoneBaseLayer);
			SpawnStemsForLayer(EMusicLayer::Exploration, ActiveProfile->ExplorationLayer);
			SpawnStemsForLayer(EMusicLayer::Tension, ActiveProfile->TensionLayer);
			SpawnStemsForLayer(EMusicLayer::Combat, ActiveProfile->CombatLayer);

			// Play zone entry stinger
			if (ActiveProfile->ZoneEntryStinger && GetOwner())
			{
				UGameplayStatics::PlaySoundAtLocation(
					GetWorld(), ActiveProfile->ZoneEntryStinger,
					GetOwner()->GetActorLocation(), 1.f, 1.f
				);
			}
		}

		// Re-activate layers for current state
		ActivateLayersForState(CurrentSnapshot.CurrentState);
	}

	OnAudioZoneChanged.Broadcast(OldZone, NewZone);
}

// --- State Transitions ---

void UAdaptiveMusicComponent::SetMusicState(EMusicState NewState)
{
	TransitionToState(NewState);
}

void UAdaptiveMusicComponent::TransitionToState(EMusicState NewState)
{
	if (NewState == CurrentSnapshot.CurrentState)
	{
		return;
	}

	EMusicState OldState = CurrentSnapshot.CurrentState;
	float TransitionDuration = GetTransitionDuration(OldState, NewState);

	CurrentSnapshot.CurrentState = NewState;

	// Set fade speeds based on transition duration
	float FadeSpeed = TransitionDuration > 0.f ? (1.f / TransitionDuration) : 100.f;
	for (FMusicLayerState& State : LayerStates)
	{
		State.FadeSpeed = FadeSpeed;
	}

	ActivateLayersForState(NewState);

	// Handle post-combat timer
	if (NewState == EMusicState::PostCombat)
	{
		PostCombatTimer = PostCombatDuration;
	}

	OnMusicStateChanged.Broadcast(OldState, NewState);
}

float UAdaptiveMusicComponent::GetTransitionDuration(EMusicState From, EMusicState To) const
{
	for (const FMusicTransition& Transition : StateTransitions)
	{
		if (Transition.FromState == From && Transition.ToState == To)
		{
			return Transition.CrossfadeDuration;
		}
	}
	return 2.f; // Default 2 second crossfade
}

void UAdaptiveMusicComponent::ActivateLayersForState(EMusicState State)
{
	// Per design doc: which layers are active in each state
	switch (State)
	{
	case EMusicState::Exploration:
		// Train Ambience + Zone Base + Exploration
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.5f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.7f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 0.f;
		break;

	case EMusicState::Suspicion:
		// Train Ambience + Zone Base + Exploration (reduced) + Tension (low)
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.5f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.3f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = CurrentSnapshot.EnemyAwareness;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 0.f;
		break;

	case EMusicState::Detection:
		// Train Ambience (reduced) + Zone Base + Tension (full)
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.4f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = 1.f;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 0.f;
		break;

	case EMusicState::Combat:
		// Train Ambience (minimal) + Zone Base (minimal) + Combat
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.15f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 1.f;
		break;

	case EMusicState::PostCombat:
		// Train Ambience (rising) + Zone Base (rising) + wind-down
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.4f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 0.1f; // Lingering combat energy
		break;

	case EMusicState::MoralChoice:
		// Near-silence — only train ambience heartbeat
		LayerStates[static_cast<int32>(EMusicLayer::ZoneBase)].TargetVolume = 0.05f;
		LayerStates[static_cast<int32>(EMusicLayer::Exploration)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Tension)].TargetVolume = 0.f;
		LayerStates[static_cast<int32>(EMusicLayer::Combat)].TargetVolume = 0.f;
		break;
	}
}

// --- Gameplay Input ---

void UAdaptiveMusicComponent::SetEnemyAwareness(float Awareness)
{
	CurrentSnapshot.EnemyAwareness = FMath::Clamp(Awareness, 0.f, 1.f);
}

void UAdaptiveMusicComponent::SetActiveEnemyCount(int32 Count)
{
	CurrentSnapshot.ActiveEnemyCount = FMath::Max(0, Count);
}

void UAdaptiveMusicComponent::SetPlayerHealthRatio(float HealthRatio)
{
	CurrentSnapshot.PlayerHealthRatio = FMath::Clamp(HealthRatio, 0.f, 1.f);
}

void UAdaptiveMusicComponent::SetIntensity(float NewIntensity)
{
	CurrentSnapshot.Intensity = FMath::Clamp(NewIntensity, 0.f, 1.f);
}

// --- Narrative ---

void UAdaptiveMusicComponent::PlayStinger(USoundBase* StingerSound)
{
	if (StingerSound && GetOwner())
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(), StingerSound,
			GetOwner()->GetActorLocation(), 1.f, 1.f
		);
	}
}

void UAdaptiveMusicComponent::EnterMoralChoice()
{
	TransitionToState(EMusicState::MoralChoice);
}

void UAdaptiveMusicComponent::ResolveMoralChoice(bool bMercyChosen)
{
	// Play the resolution tone based on choice
	if (ActiveProfile && GetOwner())
	{
		USoundBase* Resolution = bMercyChosen
			? ActiveProfile->PostCombatMercyResolution
			: ActiveProfile->PostCombatLethalResolution;

		if (Resolution)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), Resolution,
				GetOwner()->GetActorLocation(), 1.f, 1.f
			);
		}
	}

	// Clean up moral choice tone
	if (MoralChoiceTone)
	{
		MoralChoiceTone->FadeOut(2.f, 0.f);
		MoralChoiceTone = nullptr;
	}

	TransitionToState(EMusicState::Exploration);
}

// --- Combat Events ---

void UAdaptiveMusicComponent::OnCombatStarted()
{
	TransitionToState(EMusicState::Combat);
}

void UAdaptiveMusicComponent::OnCombatEnded(bool bPlayerShowedMercy)
{
	TransitionToState(EMusicState::PostCombat);

	// Play appropriate post-combat sound
	if (ActiveProfile && GetOwner())
	{
		USoundBase* PostCombat = bPlayerShowedMercy
			? ActiveProfile->PostCombatMercyResolution
			: ActiveProfile->PostCombatLethalResolution;

		if (!PostCombat)
		{
			PostCombat = ActiveProfile->PostCombatSound;
		}

		if (PostCombat)
		{
			UGameplayStatics::PlaySoundAtLocation(
				GetWorld(), PostCombat,
				GetOwner()->GetActorLocation(), 0.7f, 1.f
			);
		}
	}
}

void UAdaptiveMusicComponent::OnPlayerHitLanded()
{
	// Hit-sync: play a percussive accent within the combat layer
	// This is a placeholder — full implementation would sync to the beat grid
	if (CurrentSnapshot.CurrentState == EMusicState::Combat && ActiveProfile)
	{
		// Could trigger a one-shot percussion hit here
		// For now, briefly boost combat intensity
		CurrentSnapshot.Intensity = FMath::Min(1.f, CurrentSnapshot.Intensity + 0.05f);
	}
}

// --- Internal Update ---

void UAdaptiveMusicComponent::UpdateIntensityFromGameState()
{
	float TargetIntensity = 0.f;

	switch (CurrentSnapshot.CurrentState)
	{
	case EMusicState::Exploration:
		TargetIntensity = 0.3f;
		break;

	case EMusicState::Suspicion:
		TargetIntensity = 0.3f + CurrentSnapshot.EnemyAwareness * 0.3f;
		break;

	case EMusicState::Detection:
		TargetIntensity = 0.6f + CurrentSnapshot.EnemyAwareness * 0.2f;
		break;

	case EMusicState::Combat:
	{
		// More enemies = fuller arrangement, low health = more frantic
		float EnemyFactor = FMath::Min(1.f, CurrentSnapshot.ActiveEnemyCount / 5.f);
		float HealthPanic = 1.f - CurrentSnapshot.PlayerHealthRatio;
		TargetIntensity = 0.5f + EnemyFactor * 0.3f + HealthPanic * 0.2f;
		break;
	}

	case EMusicState::PostCombat:
		TargetIntensity = 0.2f;
		break;

	case EMusicState::MoralChoice:
		TargetIntensity = 0.05f;
		break;
	}

	// Smooth interpolation
	CurrentSnapshot.Intensity = FMath::FInterpTo(
		CurrentSnapshot.Intensity, TargetIntensity,
		GetWorld()->GetDeltaSeconds(), 2.f
	);
}

void UAdaptiveMusicComponent::UpdateLayerVolumes(float DeltaTime)
{
	for (FMusicLayerState& State : LayerStates)
	{
		if (!FMath::IsNearlyEqual(State.CurrentVolume, State.TargetVolume, 0.005f))
		{
			State.CurrentVolume = FMath::FInterpTo(
				State.CurrentVolume, State.TargetVolume,
				DeltaTime, State.FadeSpeed
			);
		}
		else
		{
			State.CurrentVolume = State.TargetVolume;
		}

		// Apply volume to all stem components in this layer
		for (UAudioComponent* Comp : State.ActiveStemComponents)
		{
			if (Comp)
			{
				Comp->SetVolumeMultiplier(State.CurrentVolume);
			}
		}

		State.bActive = State.TargetVolume > 0.01f;
	}
}

void UAdaptiveMusicComponent::UpdateStemActivation(float DeltaTime)
{
	if (!ActiveProfile)
	{
		return;
	}

	// For each layer, check stem activation thresholds against current intensity
	for (int32 LayerIdx = 0; LayerIdx < LayerStates.Num(); ++LayerIdx)
	{
		FMusicLayerState& State = LayerStates[LayerIdx];
		const FMusicLayerConfig* Config = GetLayerConfig(static_cast<EMusicLayer>(LayerIdx));

		if (!Config || !State.bActive)
		{
			continue;
		}

		for (int32 StemIdx = 0; StemIdx < Config->Stems.Num() && StemIdx < State.ActiveStemComponents.Num(); ++StemIdx)
		{
			UAudioComponent* Comp = State.ActiveStemComponents[StemIdx];
			if (!Comp)
			{
				continue;
			}

			const FStemConfig& StemCfg = Config->Stems[StemIdx];

			// Hysteresis: activate at threshold, deactivate at threshold - margin
			float ActivateAt = StemCfg.ActivationThreshold;
			float DeactivateAt = FMath::Max(0.f, ActivateAt - HysteresisMargin);

			bool bShouldPlay = Comp->IsPlaying();

			if (CurrentSnapshot.Intensity >= ActivateAt)
			{
				bShouldPlay = true;
			}
			else if (CurrentSnapshot.Intensity < DeactivateAt)
			{
				bShouldPlay = false;
			}

			if (bShouldPlay && !Comp->IsPlaying())
			{
				Comp->Play();
			}
			else if (!bShouldPlay && Comp->IsPlaying())
			{
				Comp->FadeOut(1.f, 0.f);
			}
		}
	}
}

EMusicState UAdaptiveMusicComponent::DetermineStateFromGameplay() const
{
	// Don't auto-transition out of narrative states
	if (CurrentSnapshot.CurrentState == EMusicState::MoralChoice ||
		CurrentSnapshot.CurrentState == EMusicState::PostCombat)
	{
		return CurrentSnapshot.CurrentState;
	}

	// Combat takes priority
	if (CurrentSnapshot.ActiveEnemyCount > 0 && CurrentSnapshot.CurrentState == EMusicState::Combat)
	{
		return EMusicState::Combat;
	}

	// Awareness-driven transitions
	if (CurrentSnapshot.EnemyAwareness >= 0.5f)
	{
		return EMusicState::Detection;
	}
	else if (CurrentSnapshot.EnemyAwareness > 0.f)
	{
		return EMusicState::Suspicion;
	}

	return EMusicState::Exploration;
}

void UAdaptiveMusicComponent::SpawnStemsForLayer(EMusicLayer Layer, const FMusicLayerConfig& Config)
{
	int32 LayerIdx = static_cast<int32>(Layer);
	if (LayerIdx >= LayerStates.Num())
	{
		return;
	}

	FMusicLayerState& State = LayerStates[LayerIdx];

	for (const FStemConfig& Stem : Config.Stems)
	{
		UAudioComponent* Comp = nullptr;

		if (Stem.Sound && GetOwner())
		{
			Comp = UGameplayStatics::SpawnSoundAttached(
				Stem.Sound,
				GetOwner()->GetRootComponent(),
				NAME_None,
				FVector::ZeroVector,
				EAttachLocation::KeepRelativeOffset,
				false,
				Stem.VolumeMultiplier * State.CurrentVolume,
				1.f,
				0.f,
				nullptr,
				nullptr,
				false // Don't auto-play — stem activation handles this
			);

			if (Comp)
			{
				Comp->bAutoDestroy = false;
				Comp->bIsUISound = true;

				// Start playing if intensity is above threshold
				if (CurrentSnapshot.Intensity >= Stem.ActivationThreshold)
				{
					Comp->Play();
				}
			}
		}

		State.ActiveStemComponents.Add(Comp);
	}
}

void UAdaptiveMusicComponent::CleanupLayerStems(EMusicLayer Layer)
{
	int32 LayerIdx = static_cast<int32>(Layer);
	if (LayerIdx >= LayerStates.Num())
	{
		return;
	}

	FMusicLayerState& State = LayerStates[LayerIdx];

	for (UAudioComponent* Comp : State.ActiveStemComponents)
	{
		if (Comp)
		{
			Comp->FadeOut(1.f, 0.f);
			Comp->DestroyComponent();
		}
	}
	State.ActiveStemComponents.Empty();
}

float UAdaptiveMusicComponent::GetTargetVolumeForLayer(EMusicLayer Layer) const
{
	int32 LayerIdx = static_cast<int32>(Layer);
	if (LayerIdx < LayerStates.Num())
	{
		return LayerStates[LayerIdx].TargetVolume;
	}
	return 0.f;
}

const FMusicLayerConfig* UAdaptiveMusicComponent::GetLayerConfig(EMusicLayer Layer) const
{
	if (!ActiveProfile)
	{
		return nullptr;
	}

	switch (Layer)
	{
	case EMusicLayer::ZoneBase:		return &ActiveProfile->ZoneBaseLayer;
	case EMusicLayer::Exploration:	return &ActiveProfile->ExplorationLayer;
	case EMusicLayer::Tension:		return &ActiveProfile->TensionLayer;
	case EMusicLayer::Combat:		return &ActiveProfile->CombatLayer;
	default: return nullptr;
	}
}

void UAdaptiveMusicComponent::DeactivateLayer(EMusicLayer Layer, float FadeDuration)
{
	int32 LayerIdx = static_cast<int32>(Layer);
	if (LayerIdx < LayerStates.Num())
	{
		LayerStates[LayerIdx].TargetVolume = 0.f;
		LayerStates[LayerIdx].FadeSpeed = FadeDuration > 0.f ? (1.f / FadeDuration) : 100.f;
	}
}

void UAdaptiveMusicComponent::ActivateLayer(EMusicLayer Layer, float FadeDuration)
{
	int32 LayerIdx = static_cast<int32>(Layer);
	if (LayerIdx < LayerStates.Num())
	{
		LayerStates[LayerIdx].bActive = true;
		LayerStates[LayerIdx].FadeSpeed = FadeDuration > 0.f ? (1.f / FadeDuration) : 100.f;
	}
}
