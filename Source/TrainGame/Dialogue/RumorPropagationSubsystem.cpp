// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "RumorPropagationSubsystem.h"
#include "NPCMemoryComponent.h"
#include "EngineUtils.h"

void URumorPropagationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URumorPropagationSubsystem::Deinitialize()
{
	ActiveRumors.Empty();
	DeliveredCars.Empty();
	Super::Deinitialize();
}

void URumorPropagationSubsystem::CreateRumor(FName RumorTag, int32 OriginCar)
{
	CreateRumorWithSpeed(RumorTag, OriginCar, 0.067f); // Default: ~1 car per 15 min
}

void URumorPropagationSubsystem::CreateRumorWithSpeed(FName RumorTag, int32 OriginCar, float PropagationRate)
{
	FRumorData NewRumor;
	NewRumor.RumorTag = RumorTag;
	NewRumor.OriginCar = OriginCar;
	NewRumor.CurrentReach = 0;
	NewRumor.CreationTime = GetWorld()->GetTimeSeconds();
	NewRumor.PropagationRate = PropagationRate;

	ActiveRumors.Add(NewRumor);

	// Immediately deliver to origin car
	TSet<int32>& Delivered = DeliveredCars.FindOrAdd(RumorTag);
	Delivered.Add(OriginCar);
	DeliverRumorToCar(NewRumor, OriginCar);
}

bool URumorPropagationSubsystem::HasRumorReachedCar(FName RumorTag, int32 CarIndex) const
{
	const TSet<int32>* Delivered = DeliveredCars.Find(RumorTag);
	return Delivered && Delivered->Contains(CarIndex);
}

void URumorPropagationSubsystem::TickRumorPropagation(float GameMinutesElapsed)
{
	for (FRumorData& Rumor : ActiveRumors)
	{
		float NewReach = Rumor.CurrentReach + (Rumor.PropagationRate * GameMinutesElapsed);
		int32 OldReachInt = FMath::FloorToInt(Rumor.CurrentReach);
		int32 NewReachInt = FMath::FloorToInt(NewReach);
		Rumor.CurrentReach = NewReach;

		// Deliver to newly reached cars (both directions from origin)
		if (NewReachInt > OldReachInt)
		{
			TSet<int32>& Delivered = DeliveredCars.FindOrAdd(Rumor.RumorTag);

			for (int32 Dist = OldReachInt + 1; Dist <= NewReachInt; ++Dist)
			{
				// Forward direction
				int32 ForwardCar = Rumor.OriginCar + Dist;
				if (!Delivered.Contains(ForwardCar))
				{
					Delivered.Add(ForwardCar);
					DeliverRumorToCar(Rumor, ForwardCar);
				}

				// Backward direction
				int32 BackwardCar = Rumor.OriginCar - Dist;
				if (BackwardCar >= 0 && !Delivered.Contains(BackwardCar))
				{
					Delivered.Add(BackwardCar);
					DeliverRumorToCar(Rumor, BackwardCar);
				}
			}
		}
	}
}

void URumorPropagationSubsystem::DeliverRumorToCar(const FRumorData& Rumor, int32 CarIndex)
{
	TArray<UNPCMemoryComponent*> NPCs = GetNPCsInCar(CarIndex);
	for (UNPCMemoryComponent* NPC : NPCs)
	{
		if (NPC)
		{
			NPC->ReceiveRumor(Rumor);
		}
	}
}

TArray<UNPCMemoryComponent*> URumorPropagationSubsystem::GetNPCsInCar(int32 CarIndex) const
{
	TArray<UNPCMemoryComponent*> Result;

	// TODO: Use car management system to find actors in specific car index
	// For now, iterate all actors with NPCMemoryComponent
	// This should be replaced with a spatial query once the car system is in place

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (UNPCMemoryComponent* MemComp = It->FindComponentByClass<UNPCMemoryComponent>())
			{
				// TODO: Check if actor is in the target car
				// For now, add all NPCs (stub behavior)
			}
		}
	}

	return Result;
}
