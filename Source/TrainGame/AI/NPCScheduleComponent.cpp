// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCScheduleComponent.h"
#include "Kismet/GameplayStatics.h"

UNPCScheduleComponent::UNPCScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Tick once per second
}

void UNPCScheduleComponent::BeginPlay()
{
	Super::BeginPlay();
	EvaluateSchedule();
}

void UNPCScheduleComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bSuspended)
	{
		return;
	}

	ScheduleCheckTimer += DeltaTime;
	if (ScheduleCheckTimer >= ScheduleCheckInterval)
	{
		ScheduleCheckTimer = 0.f;
		EvaluateSchedule();
	}
}

void UNPCScheduleComponent::SetSchedule(const FNPCSchedule& InSchedule)
{
	Schedule = InSchedule;
	EvaluateSchedule();
}

FVector UNPCScheduleComponent::GetCurrentActivityLocation() const
{
	if (CurrentEntryIndex >= 0 && CurrentEntryIndex < Schedule.Entries.Num())
	{
		return Schedule.Entries[CurrentEntryIndex].Location;
	}
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

FName UNPCScheduleComponent::GetCurrentActivityLocationTag() const
{
	if (CurrentEntryIndex >= 0 && CurrentEntryIndex < Schedule.Entries.Num())
	{
		return Schedule.Entries[CurrentEntryIndex].LocationTag;
	}
	return NAME_None;
}

void UNPCScheduleComponent::SuspendSchedule()
{
	bSuspended = true;
}

void UNPCScheduleComponent::ResumeSchedule()
{
	bSuspended = false;
	EvaluateSchedule();
}

void UNPCScheduleComponent::EvaluateSchedule()
{
	if (Schedule.Entries.Num() == 0)
	{
		return;
	}

	// Get current game time hour (using game time seconds, 1 game hour = 60 real seconds)
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Convert game time to hour of day (0-23)
	// 1 full day = 24 game hours = 1440 real seconds (24 minutes real time)
	float GameTimeSeconds = World->GetTimeSeconds();
	int32 CurrentHour = (static_cast<int32>(GameTimeSeconds / 60.f)) % 24;

	const FNPCScheduleEntry* Entry = FindCurrentEntry(CurrentHour);
	if (!Entry)
	{
		return;
	}

	int32 NewIndex = Schedule.Entries.IndexOfByPredicate([Entry](const FNPCScheduleEntry& E) {
		return &E == Entry;
	});

	if (NewIndex != CurrentEntryIndex)
	{
		ENPCActivity OldActivity = CurrentActivity;
		CurrentActivity = Entry->Activity;
		CurrentEntryIndex = NewIndex;

		OnActivityChanged.Broadcast(OldActivity, CurrentActivity);
	}
}

const FNPCScheduleEntry* UNPCScheduleComponent::FindCurrentEntry(int32 CurrentHour) const
{
	// Find the schedule entry whose start hour is the latest one <= CurrentHour
	const FNPCScheduleEntry* BestEntry = nullptr;
	const FNPCScheduleEntry* EarliestEntry = nullptr;

	for (const FNPCScheduleEntry& Entry : Schedule.Entries)
	{
		if (!EarliestEntry || Entry.StartHour < EarliestEntry->StartHour)
		{
			EarliestEntry = &Entry;
		}

		if (Entry.StartHour <= CurrentHour)
		{
			if (!BestEntry || Entry.StartHour > BestEntry->StartHour)
			{
				BestEntry = &Entry;
			}
		}
	}

	// If no entry found with StartHour <= CurrentHour, wrap to last entry of previous day
	return BestEntry ? BestEntry : (Schedule.Entries.Num() > 0 ? &Schedule.Entries.Last() : nullptr);
}
