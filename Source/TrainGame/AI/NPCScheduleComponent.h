// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "NPCScheduleComponent.generated.h"

// ============================================================================
// UNPCScheduleComponent
//
// Time-of-day routine system for NPCs. Each NPC has a daily schedule
// of activities (sleep, work, patrol, eat, socialize, etc.) with
// associated locations. The schedule drives NPC movement and behavior
// tree state when not in alert/combat mode.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivityChanged, ENPCActivity, OldActivity, ENPCActivity, NewActivity);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCScheduleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Schedule ---

	/** Set the NPC's daily schedule */
	UFUNCTION(BlueprintCallable, Category = "AI|Schedule")
	void SetSchedule(const FNPCSchedule& InSchedule);

	/** Get the current activity based on game time */
	UFUNCTION(BlueprintPure, Category = "AI|Schedule")
	ENPCActivity GetCurrentActivity() const { return CurrentActivity; }

	/** Get the location for the current activity */
	UFUNCTION(BlueprintPure, Category = "AI|Schedule")
	FVector GetCurrentActivityLocation() const;

	/** Get the location tag for the current activity */
	UFUNCTION(BlueprintPure, Category = "AI|Schedule")
	FName GetCurrentActivityLocationTag() const;

	/** Override the schedule temporarily (e.g. during alert) */
	UFUNCTION(BlueprintCallable, Category = "AI|Schedule")
	void SuspendSchedule();

	/** Resume normal schedule after override */
	UFUNCTION(BlueprintCallable, Category = "AI|Schedule")
	void ResumeSchedule();

	/** Whether the schedule is currently suspended */
	UFUNCTION(BlueprintPure, Category = "AI|Schedule")
	bool IsScheduleSuspended() const { return bSuspended; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "AI|Schedule")
	FOnActivityChanged OnActivityChanged;

protected:
	virtual void BeginPlay() override;

	/** The NPC's daily schedule */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	FNPCSchedule Schedule;

	/** How often to check schedule (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Schedule")
	float ScheduleCheckInterval = 5.f;

private:
	void EvaluateSchedule();
	const FNPCScheduleEntry* FindCurrentEntry(int32 CurrentHour) const;

	ENPCActivity CurrentActivity = ENPCActivity::Idle;
	int32 CurrentEntryIndex = -1;
	float ScheduleCheckTimer = 0.f;
	bool bSuspended = false;
};
