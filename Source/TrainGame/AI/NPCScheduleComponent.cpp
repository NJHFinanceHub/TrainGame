// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCScheduleComponent.h"
#include "Kismet/GameplayStatics.h"

UNPCScheduleComponent::UNPCScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Check once per second
}

void UNPCScheduleComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initial schedule evaluation
	if (DailySchedule.Num() > 0)
	{
		// TODO: Get actual game time from game mode
		EvaluateSchedule(6); // Default to 6 AM on start
	}
}

void UNPCScheduleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bHasOverride || DailySchedule.Num() == 0)
	{
		return;
	}

	// Check schedule periodically (game time drives this)
	ScheduleCheckTimer += DeltaTime;
	if (ScheduleCheckTimer >= 5.0f)
	{
		ScheduleCheckTimer = 0.f;
		// TODO: Query actual game-time hour from time-of-day system
		// For now, use a placeholder that advances with real time
		int32 CurrentHour = FMath::FloorToInt(FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.f, 24.f));
		EvaluateSchedule(CurrentHour);
	}
}

void UNPCScheduleComponent::SetSchedule(const TArray<FScheduleEntry>& NewSchedule)
{
	DailySchedule = NewSchedule;
	ActiveScheduleIndex = -1;
	// Re-evaluate immediately
	int32 CurrentHour = FMath::FloorToInt(FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.f, 24.f));
	EvaluateSchedule(CurrentHour);
}

FName UNPCScheduleComponent::GetCurrentLocationTag() const
{
	if (bHasOverride)
	{
		return OverrideLocationTag;
	}

	if (ActiveScheduleIndex >= 0 && ActiveScheduleIndex < DailySchedule.Num())
	{
		return DailySchedule[ActiveScheduleIndex].LocationTag;
	}

	return NAME_None;
}

FName UNPCScheduleComponent::GetCurrentPatrolRouteTag() const
{
	if (ActiveScheduleIndex >= 0 && ActiveScheduleIndex < DailySchedule.Num())
	{
		return DailySchedule[ActiveScheduleIndex].PatrolRouteTag;
	}

	return NAME_None;
}

void UNPCScheduleComponent::OverrideActivity(EScheduleActivity Activity, FName LocationTag)
{
	bHasOverride = true;
	OverrideActivityValue = Activity;
	OverrideLocationTag = LocationTag;

	EScheduleActivity OldActivity = CurrentActivity;
	CurrentActivity = Activity;

	if (OldActivity != Activity)
	{
		OnScheduleActivityChanged.Broadcast(OldActivity, Activity);
	}

	OnScheduleLocationChanged.Broadcast(LocationTag);
}

void UNPCScheduleComponent::ClearOverride()
{
	bHasOverride = false;

	// Re-evaluate normal schedule
	ActiveScheduleIndex = -1;
	int32 CurrentHour = FMath::FloorToInt(FMath::Fmod(GetWorld()->GetTimeSeconds() / 60.f, 24.f));
	EvaluateSchedule(CurrentHour);
}

void UNPCScheduleComponent::EvaluateSchedule(int32 CurrentHour)
{
	int32 NewIndex = FindScheduleEntryForHour(CurrentHour);
	if (NewIndex == ActiveScheduleIndex)
	{
		return;
	}

	ActiveScheduleIndex = NewIndex;

	if (NewIndex < 0)
	{
		return;
	}

	const FScheduleEntry& Entry = DailySchedule[NewIndex];
	EScheduleActivity OldActivity = CurrentActivity;
	CurrentActivity = Entry.Activity;

	if (OldActivity != CurrentActivity)
	{
		OnScheduleActivityChanged.Broadcast(OldActivity, CurrentActivity);
	}

	OnScheduleLocationChanged.Broadcast(Entry.LocationTag);
}

int32 UNPCScheduleComponent::FindScheduleEntryForHour(int32 Hour) const
{
	for (int32 i = 0; i < DailySchedule.Num(); ++i)
	{
		const FScheduleEntry& Entry = DailySchedule[i];

		if (Entry.StartHour <= Entry.EndHour)
		{
			// Normal range (e.g., 8-16)
			if (Hour >= Entry.StartHour && Hour < Entry.EndHour)
			{
				return i;
			}
		}
		else
		{
			// Wrapping range (e.g., 22-6 = sleep overnight)
			if (Hour >= Entry.StartHour || Hour < Entry.EndHour)
			{
				return i;
			}
		}
	}

	return -1;
}
