// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCMemoryComponent.h"

UNPCMemoryComponent::UNPCMemoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UNPCMemoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UNPCMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickMoodDecay(DeltaTime);
}

// --- Memory Management ---

void UNPCMemoryComponent::AddMemory(const FNPCMemory& Memory)
{
	Memories.Add(Memory);

	// Apply disposition delta
	if (Memory.DispositionDelta != 0)
	{
		ModifyDisposition(Memory.DispositionDelta);
	}

	OnMemoryAdded.Broadcast(Memory.MemoryTag);
}

bool UNPCMemoryComponent::HasMemory(FName MemoryTag) const
{
	for (const FNPCMemory& Memory : Memories)
	{
		if (Memory.MemoryTag == MemoryTag)
		{
			return true;
		}
	}
	return false;
}

TArray<FNPCMemory> UNPCMemoryComponent::GetMemoriesByCategory(EMemoryCategory Category) const
{
	TArray<FNPCMemory> Result;
	for (const FNPCMemory& Memory : Memories)
	{
		if (Memory.Category == Category)
		{
			Result.Add(Memory);
		}
	}
	return Result;
}

void UNPCMemoryComponent::RemoveMemory(FName MemoryTag)
{
	Memories.RemoveAll([MemoryTag](const FNPCMemory& Memory)
	{
		return Memory.MemoryTag == MemoryTag;
	});
}

// --- Disposition ---

ENPCDisposition UNPCMemoryComponent::GetDispositionBracket() const
{
	if (Disposition <= -50) return ENPCDisposition::Hostile;
	if (Disposition <= -20) return ENPCDisposition::Antagonistic;
	if (Disposition <= 19) return ENPCDisposition::Neutral;
	if (Disposition <= 49) return ENPCDisposition::Friendly;
	return ENPCDisposition::Loyal;
}

void UNPCMemoryComponent::ModifyDisposition(int32 Delta)
{
	int32 OldValue = Disposition;
	Disposition = FMath::Clamp(Disposition + Delta, -100, 100);

	if (OldValue != Disposition)
	{
		OnDispositionChanged.Broadcast(OldValue, Disposition);
	}
}

void UNPCMemoryComponent::SetDisposition(int32 NewValue)
{
	int32 OldValue = Disposition;
	Disposition = FMath::Clamp(NewValue, -100, 100);

	if (OldValue != Disposition)
	{
		OnDispositionChanged.Broadcast(OldValue, Disposition);
	}
}

// --- Rumor System ---

void UNPCMemoryComponent::ReceiveRumor(const FRumorData& Rumor)
{
	int32 Distance = FMath::Abs(CarIndex - Rumor.OriginCar);

	FNPCMemory RumorMemory;
	RumorMemory.Category = EMemoryCategory::Rumor;
	RumorMemory.MemoryTag = Rumor.RumorTag;
	RumorMemory.RumorDistance = Distance;

	// Fidelity affects disposition impact
	float Fidelity = GetRumorFidelity(Distance);
	// TODO: Apply fidelity to the rumor's disposition delta

	AddMemory(RumorMemory);

	// Check if rumor contradicts any active lies
	for (const auto& LiePair : ActiveLies)
	{
		if (LiePair.Value == Rumor.RumorTag)
		{
			ExposeLie(LiePair.Key);
		}
	}
}

float UNPCMemoryComponent::GetRumorFidelity(int32 DistanceFromOrigin) const
{
	// Same car = 1.0, degrades with distance
	if (DistanceFromOrigin <= 0) return 1.0f;
	if (DistanceFromOrigin <= 2) return 0.8f;
	if (DistanceFromOrigin <= 5) return 0.5f;
	return 0.2f; // 6+ cars: mythologized
}

// --- Lie Tracking ---

void UNPCMemoryComponent::RecordLie(FName LieTag, FName ContradictoryTruth)
{
	ActiveLies.Add(LieTag, ContradictoryTruth);
}

bool UNPCMemoryComponent::IsLieExposed(FName LieTag) const
{
	return ExposedLies.Contains(LieTag);
}

void UNPCMemoryComponent::ExposeLie(FName LieTag)
{
	if (!ExposedLies.Contains(LieTag))
	{
		ExposedLies.Add(LieTag);
		ModifyDisposition(-50);
		OnLieExposed.Broadcast(LieTag);
	}
}

// --- Internal ---

void UNPCMemoryComponent::TickMoodDecay(float DeltaTime)
{
	for (int32 i = Memories.Num() - 1; i >= 0; --i)
	{
		FNPCMemory& Memory = Memories[i];
		if (Memory.Category == EMemoryCategory::Mood && Memory.MoodDecayRate > 0.f)
		{
			// Decay the disposition delta over time
			float DecayAmount = Memory.MoodDecayRate * DeltaTime / 60.f; // Per minute

			if (Memory.DispositionDelta > 0)
			{
				Memory.DispositionDelta = FMath::Max(0, Memory.DispositionDelta - FMath::CeilToInt(DecayAmount));
			}
			else if (Memory.DispositionDelta < 0)
			{
				Memory.DispositionDelta = FMath::Min(0, Memory.DispositionDelta + FMath::CeilToInt(DecayAmount));
			}

			// Remove fully decayed mood memories
			if (Memory.DispositionDelta == 0)
			{
				Memories.RemoveAt(i);
			}
		}
	}
}
