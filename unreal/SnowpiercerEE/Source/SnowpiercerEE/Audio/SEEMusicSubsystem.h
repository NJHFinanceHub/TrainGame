#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEEMusicSubsystem.generated.h"

class UAudioComponent;
class USoundBase;

// ============================================================================
// USEEMusicSubsystem
//
// Dynamic two-layer zone soundtrack (explore / combat) for game worlds.
//
// - On world begin play: loads MUS_Zone1_Explore / MUS_Zone1_Combat
//   defensively (the wavs may not be imported yet — logs once and stays
//   silent if missing) and starts both as looping 2D components: explore
//   audible at MusicVolume, combat held at zero so the layers stay in sync.
// - A 0.5s poll checks for any hostile, aggroed ASEENPCAIController pawn
//   within CombatDetectRange of the player -> crossfades (AdjustVolume, both
//   components kept alive) to the combat layer over CrossfadeTime. When no
//   threat remains, lingers CombatLingerTime before fading back to explore.
// - Player death ducks the active layer to DeathDuckVolume; restores on
//   respawn. Components are non-UI sounds so game pause silences them.
// ============================================================================
UCLASS()
class SNOWPIERCEREE_API USEEMusicSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "Music")
	bool IsCombatMusicActive() const { return bCombatActive; }

protected:
	/** Target volume for the audible music layer. */
	float MusicVolume = 0.35f;

	/** Explore<->combat crossfade duration (seconds). */
	float CrossfadeTime = 1.5f;

	/** How long combat music lingers after the last threat disappears. */
	float CombatLingerTime = 4.0f;

	/** Hostile-aggro detection radius around the player (cm). */
	float CombatDetectRange = 4000.0f;

	/** Combat-state poll interval (seconds). */
	float PollInterval = 0.5f;

	/** Volume the active layer ducks to while the player is dead. */
	float DeathDuckVolume = 0.15f;

private:
	void PollCombatState();
	bool IsAnyHostileAggroNearPlayer(const APawn* PlayerPawn) const;
	void SetCombatMusic(bool bCombat);
	void UpdateDeathDuck(const APawn* PlayerPawn);
	float ActiveLayerTargetVolume() const { return bDucked ? DeathDuckVolume : MusicVolume; }

	/** Spawn a looping, non-UI 2D music component (nullptr-safe). */
	UAudioComponent* SpawnMusicComponent(USoundBase* Sound, float StartVolume);

	UPROPERTY()
	TObjectPtr<UAudioComponent> ExploreComp;

	UPROPERTY()
	TObjectPtr<UAudioComponent> CombatComp;

	FTimerHandle PollTimerHandle;
	bool bCombatActive = false;
	bool bDucked = false;
	float CombatLingerRemaining = 0.0f;
};
