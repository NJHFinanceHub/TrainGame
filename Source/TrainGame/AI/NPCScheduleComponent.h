// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "NPCScheduleComponent.generated.h"

// ============================================================================
// UNPCScheduleComponent
//
// Drives NPC daily routines. Each NPC has a schedule of time-blocks
// (sleep/work/patrol/eat/socialize). The component ticks against
// game time and notifies the AI controller when the current activity
// changes, so the behavior tree can transition.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScheduleActivityChanged, EScheduleActivity, OldActivity, EScheduleActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScheduleLocationChanged, FName, NewLocationTag);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCScheduleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Schedule Management ---

	UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
	void SetSchedule(const TArray<FScheduleEntry>& NewSchedule);

	UFUNCTION(BlueprintPure, Category = "NPC|Schedule")
	EScheduleActivity GetCurrentActivity() const { return CurrentActivity; }

	UFUNCTION(BlueprintPure, Category = "NPC|Schedule")
	FName GetCurrentLocationTag() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Schedule")
	FName GetCurrentPatrolRouteTag() const;

	/** Override the schedule temporarily (e.g., during alert) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
	void OverrideActivity(EScheduleActivity Activity, FName LocationTag);

	/** Clear the override and return to normal schedule */
	UFUNCTION(BlueprintCallable, Category = "NPC|Schedule")
	void ClearOverride();

	UFUNCTION(BlueprintPure, Category = "NPC|Schedule")
	bool HasOverride() const { return bHasOverride; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "NPC|Schedule")
	FOnScheduleActivityChanged OnScheduleActivityChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Schedule")
	FOnScheduleLocationChanged OnScheduleLocationChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Schedule")
	TArray<FScheduleEntry> DailySchedule;

private:
	void EvaluateSchedule(int32 CurrentHour);
	int32 FindScheduleEntryForHour(int32 Hour) const;

	EScheduleActivity CurrentActivity = EScheduleActivity::Sleep;
	int32 ActiveScheduleIndex = -1;
	float ScheduleCheckTimer = 0.f;

	bool bHasOverride = false;
	EScheduleActivity OverrideActivityValue = EScheduleActivity::Custom;
	FName OverrideLocationTag;
};
