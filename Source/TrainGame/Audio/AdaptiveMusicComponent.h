// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioTypes.h"
#include "AdaptiveMusicComponent.generated.h"

class UZoneAudioProfile;

// ============================================================================
// Adaptive Music Component — Layered music system responding to gameplay
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMusicStateChanged, EMusicState, OldState, EMusicState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAudioZoneChanged, EAudioZone, OldZone, EAudioZone, NewZone);

/**
 * UAdaptiveMusicComponent
 *
 * The heart of the game's adaptive music system. Manages a 6-layer music stack
 * that responds to gameplay state (exploration, suspicion, combat, etc.).
 *
 * Architecture follows the design docs:
 * - Layer 0 (Train Ambience): Managed by UTrainAmbienceComponent
 * - Layer 1 (Zone Base): Always playing, zone-specific identity
 * - Layer 2 (Exploration): Melodic content, active during safe exploration
 * - Layer 3 (Tension): Suspense elements, scales with enemy awareness
 * - Layer 4 (Combat): Full combat arrangement
 * - Layer 5 (Narrative): Scripted musical moments (stingers)
 *
 * Intensity is a 0.0-1.0 float that drives vertical remixing within layers.
 * Stems within a layer activate/deactivate based on intensity thresholds
 * with hysteresis to prevent flickering.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRAINGAME_API UAdaptiveMusicComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAdaptiveMusicComponent();

	// --- Configuration ---

	/** Registered zone audio profiles (populated at startup or via data assets) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
	TMap<EAudioZone, UZoneAudioProfile*> ZoneProfiles;

	/** Default transitions between music states */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music|Transitions")
	TArray<FMusicTransition> StateTransitions;

	/** Hysteresis margin for stem activation thresholds (prevents flicker) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music", meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float HysteresisMargin = 0.1f;

	/** Maximum simultaneous active stems (performance budget) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
	int32 MaxActiveStems = 12;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Adaptive Music|Events")
	FOnMusicStateChanged OnMusicStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Adaptive Music|Events")
	FOnAudioZoneChanged OnAudioZoneChanged;

	// --- Zone Management ---

	/** Set the active audio zone (triggers zone crossfade) */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetAudioZone(EAudioZone NewZone);

	/** Get current audio zone */
	UFUNCTION(BlueprintPure, Category = "Adaptive Music")
	EAudioZone GetCurrentZone() const { return CurrentSnapshot.CurrentZone; }

	// --- State Transitions ---

	/** Set the music state directly (usually driven by gameplay systems) */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetMusicState(EMusicState NewState);

	/** Get current music state */
	UFUNCTION(BlueprintPure, Category = "Adaptive Music")
	EMusicState GetMusicState() const { return CurrentSnapshot.CurrentState; }

	// --- Gameplay Input ---

	/** Update enemy awareness level (0.0-1.0) — drives suspicion/detection */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetEnemyAwareness(float Awareness);

	/** Update active enemy count — drives combat intensity */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetActiveEnemyCount(int32 Count);

	/** Update player health ratio (0.0-1.0) — affects combat music */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetPlayerHealthRatio(float HealthRatio);

	/** Set overall music intensity manually (0.0-1.0) */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void SetIntensity(float NewIntensity);

	// --- Narrative ---

	/** Play a one-shot narrative stinger */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void PlayStinger(USoundBase* StingerSound);

	/** Enter moral choice state (near-silence + sustained tone) */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void EnterMoralChoice();

	/** Exit moral choice with resolution direction */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void ResolveMoralChoice(bool bMercyChosen);

	// --- Query ---

	/** Get a snapshot of the current adaptive music state */
	UFUNCTION(BlueprintPure, Category = "Adaptive Music")
	FAdaptiveMusicSnapshot GetSnapshot() const { return CurrentSnapshot; }

	// --- Combat Events ---

	/** Notify that combat has started */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void OnCombatStarted();

	/** Notify that combat has ended */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void OnCombatEnded(bool bPlayerShowedMercy);

	/** Notify of a successful player hit (for hit-sync percussion accents) */
	UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
	void OnPlayerHitLanded();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	FAdaptiveMusicSnapshot CurrentSnapshot;

	/** Per-layer runtime state */
	TArray<FMusicLayerState> LayerStates;

	/** Active zone profile */
	UPROPERTY()
	UZoneAudioProfile* ActiveProfile = nullptr;

	/** Post-combat wind-down timer */
	float PostCombatTimer = 0.f;
	float PostCombatDuration = 6.f;

	/** Moral choice sustained tone */
	UPROPERTY()
	UAudioComponent* MoralChoiceTone = nullptr;

	// --- Internal ---

	void InitializeLayerStates();
	void TransitionToState(EMusicState NewState);
	float GetTransitionDuration(EMusicState From, EMusicState To) const;
	void UpdateLayerVolumes(float DeltaTime);
	void UpdateIntensityFromGameState();
	void ActivateLayersForState(EMusicState State);
	void DeactivateLayer(EMusicLayer Layer, float FadeDuration);
	void ActivateLayer(EMusicLayer Layer, float FadeDuration);
	void SpawnStemsForLayer(EMusicLayer Layer, const FMusicLayerConfig& Config);
	void CleanupLayerStems(EMusicLayer Layer);
	void UpdateStemActivation(float DeltaTime);

	/** Calculate target volume for a layer based on current state */
	float GetTargetVolumeForLayer(EMusicLayer Layer) const;

	/** Get the music layer config from the active profile */
	const FMusicLayerConfig* GetLayerConfig(EMusicLayer Layer) const;

	/** Auto-detect music state from gameplay parameters */
	EMusicState DetermineStateFromGameplay() const;
};
