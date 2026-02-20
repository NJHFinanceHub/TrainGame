// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCScheduleComponent.h"

UNPCScheduleComponent::UNPCScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.f; // Don't need to tick every frame
}

void UNPCScheduleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bScheduleSuspended || DailySchedule.Num() == 0) return;

	CheckTimer -= DeltaTime;
	if (CheckTimer <= 0.f)
	{
		CheckTimer = CheckInterval;
		EvaluateSchedule();
	}
}

void UNPCScheduleComponent::SetSchedule(const TArray<FScheduleEntry>& NewSchedule)
{
	DailySchedule = NewSchedule;
	EvaluateSchedule();
}

void UNPCScheduleComponent::SetGameTimeHours(float Hours)
{
	GameTimeHours = FMath::Fmod(Hours, 24.f);
}

void UNPCScheduleComponent::SuspendSchedule()
{
	bScheduleSuspended = true;
}

void UNPCScheduleComponent::ResumeSchedule()
{
	bScheduleSuspended = false;
	bAtScheduledLocation = false; // Force re-navigation
	EvaluateSchedule();
}

void UNPCScheduleComponent::EvaluateSchedule()
{
	int32 CurrentHour = FMath::FloorToInt(GameTimeHours) % 24;

	for (const FScheduleEntry& Entry : DailySchedule)
	{
		bool bInRange = false;
		if (Entry.StartHour <= Entry.EndHour)
		{
			bInRange = (CurrentHour >= Entry.StartHour && CurrentHour < Entry.EndHour);
		}
		else
		{
			bInRange = (CurrentHour >= Entry.StartHour || CurrentHour < Entry.EndHour);
		}

		if (bInRange)
		{
			if (CurrentActivity != Entry.Activity || CurrentLocationTag != Entry.LocationTag)
			{
				EScheduleActivity OldActivity = CurrentActivity;
				CurrentActivity = Entry.Activity;
				CurrentLocationTag = Entry.LocationTag;
				CurrentAnimationTag = Entry.AnimationTag;
				bAtScheduledLocation = false;

				OnScheduleActivityChanged.Broadcast(OldActivity, CurrentActivity, CurrentLocationTag);
			}
			return;
		}
	}
}
