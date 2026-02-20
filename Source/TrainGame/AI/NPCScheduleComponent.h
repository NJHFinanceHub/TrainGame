// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "NPCScheduleComponent.generated.h"

// ============================================================================
// UNPCScheduleComponent
//
// Drives NPC time-of-day routines. Each NPC has a daily schedule of activities
// (sleep, work, patrol, eat, socialize) mapped to locations on the train.
// The schedule component tells the AI controller where the NPC should be
// and what they should be doing at any given game hour.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnScheduleActivityChanged,
	EScheduleActivity, OldActivity,
	EScheduleActivity, NewActivity,
	FName, NewLocationTag);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCScheduleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Schedule Management ---

	/** Set the NPC's full daily schedule */
	UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
	void SetSchedule(const TArray<FScheduleEntry>& NewSchedule);

	/** Get the current activity based on game time */
	UFUNCTION(BlueprintPure, Category = "NPC Schedule")
	EScheduleActivity GetCurrentActivity() const { return CurrentActivity; }

	/** Get the target location tag for the current activity */
	UFUNCTION(BlueprintPure, Category = "NPC Schedule")
	FName GetCurrentLocationTag() const { return CurrentLocationTag; }

	/** Get the animation tag for the current activity */
	UFUNCTION(BlueprintPure, Category = "NPC Schedule")
	FName GetCurrentAnimationTag() const { return CurrentAnimationTag; }

	/** Check if the NPC is where they should be */
	UFUNCTION(BlueprintPure, Category = "NPC Schedule")
	bool IsAtScheduledLocation() const { return bAtScheduledLocation; }

	/** Mark that the NPC has arrived at their scheduled location */
	UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
	void SetAtScheduledLocation(bool bArrived) { bAtScheduledLocation = bArrived; }

	/** Set the current game time (hours, 0-24) */
	UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
	void SetGameTimeHours(float Hours);

	/** Temporarily override the schedule (e.g., during combat/alert) */
	UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
	void SuspendSchedule();

	/** Resume the normal schedule after override */
	UFUNCTION(BlueprintCallable, Category = "NPC Schedule")
	void ResumeSchedule();

	UPROPERTY(BlueprintAssignable, Category = "NPC Schedule")
	FOnScheduleActivityChanged OnScheduleActivityChanged;

protected:
	/** The NPC's daily routine */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
	TArray<FScheduleEntry> DailySchedule;

	/** How often to check for schedule changes (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
	float CheckInterval = 5.f;

private:
	void EvaluateSchedule();

	EScheduleActivity CurrentActivity = EScheduleActivity::Sleep;
	FName CurrentLocationTag = NAME_None;
	FName CurrentAnimationTag = NAME_None;
	float GameTimeHours = 0.f;
	float CheckTimer = 0.f;
	bool bAtScheduledLocation = false;
	bool bScheduleSuspended = false;
};
