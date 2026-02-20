// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EndingTypes.h"
#include "EndingCinematicsComponent.generated.h"

// ============================================================================
// Ending Cinematics Component
// Drives the ending cutscene sequence, epilogue, and credits transition.
// Placed on the player character or a dedicated EndingSequenceActor.
// ============================================================================

class UEndingCalculatorSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingCutsceneStarted, EEndingType, Ending);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndingCutsceneFinished, EEndingType, Ending);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreditsStarted, ECreditsMusic, Music);

UCLASS(ClassGroup=(Endings), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UEndingCinematicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEndingCinematicsComponent();

	// -------------------------------------------------------------------
	// Cutscene Playback
	// -------------------------------------------------------------------

	/** Begin the ending sequence for the triggered ending. */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematics")
	void PlayEndingSequence();

	/** Skip to credits (if player presses skip during cutscene). */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematics")
	void SkipToCredits();

	/** Is a cutscene currently playing? */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematics")
	bool IsCutscenePlaying() const { return bCutscenePlaying; }

	// -------------------------------------------------------------------
	// Epilogue
	// -------------------------------------------------------------------

	/** Get the epilogue text for the current ending + companion fates. */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematics")
	TArray<FText> BuildEpilogueSequence() const;

	// -------------------------------------------------------------------
	// Credits
	// -------------------------------------------------------------------

	/** Get the credits music type for the triggered ending. */
	UFUNCTION(BlueprintPure, Category = "Ending|Cinematics")
	ECreditsMusic GetCreditsMusic() const;

	/** Begin credits roll. Called automatically after cutscene, or manually via skip. */
	UFUNCTION(BlueprintCallable, Category = "Ending|Cinematics")
	void StartCredits();

	// -------------------------------------------------------------------
	// Delegates
	// -------------------------------------------------------------------

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematics")
	FOnEndingCutsceneStarted OnCutsceneStarted;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematics")
	FOnEndingCutsceneFinished OnCutsceneFinished;

	UPROPERTY(BlueprintAssignable, Category = "Ending|Cinematics")
	FOnCreditsStarted OnCreditsStarted;

protected:
	virtual void BeginPlay() override;

private:
	bool bCutscenePlaying = false;
	bool bCreditsStarted = false;

	/** Timer handle for cutscene duration -> credits transition. */
	FTimerHandle CutsceneTimerHandle;

	void OnCutsceneTimerExpired();

	/** Resolve the ending calculator subsystem. */
	UEndingCalculatorSubsystem* GetEndingCalculator() const;
};
