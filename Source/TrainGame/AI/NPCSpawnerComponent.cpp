// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCSpawnerComponent.h"
#include "SEENPCCharacter.h"
#include "SEENPCController.h"
#include "NPCScheduleComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

UNPCSpawnerComponent::UNPCSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNPCSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawn)
	{
		SpawnAllNPCs();
	}
}

void UNPCSpawnerComponent::SpawnAllNPCs()
{
	if (!SpawnTable)
	{
		return;
	}

	TArray<FNPCSpawnEntry*> AllRows;
	SpawnTable->GetAllRows<FNPCSpawnEntry>(TEXT("NPCSpawner"), AllRows);

	for (const FNPCSpawnEntry* Entry : AllRows)
	{
		if (!Entry)
		{
			continue;
		}

		for (int32 i = 0; i < Entry->Count; ++i)
		{
			FVector SpawnLocation = ResolveSpawnLocation(*Entry);

			// Add slight offset for multiple spawns at same location
			if (i > 0)
			{
				SpawnLocation += FVector(FMath::RandRange(-100.f, 100.f), FMath::RandRange(-100.f, 100.f), 0.f);
			}

			ASEENPCCharacter* NPC = SpawnNPC(*Entry, SpawnLocation);
			if (NPC)
			{
				SpawnedNPCs.Add(NPC);
			}
		}
	}
}

void UNPCSpawnerComponent::DespawnAllNPCs()
{
	for (ASEENPCCharacter* NPC : SpawnedNPCs)
	{
		if (NPC && IsValid(NPC))
		{
			NPC->Destroy();
		}
	}

	SpawnedNPCs.Empty();
	bHasDespawned = true;
}

void UNPCSpawnerComponent::RespawnNPCs()
{
	if (!bHasDespawned)
	{
		return;
	}

	bHasDespawned = false;
	SpawnAllNPCs();
}

ASEENPCCharacter* UNPCSpawnerComponent::SpawnNPC(const FNPCSpawnEntry& Entry, FVector SpawnLocation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Determine class to spawn
	UClass* NPCClass = Entry.NPCClass.IsValid() ? Entry.NPCClass.Get() : nullptr;
	if (!NPCClass)
	{
		// Try to load the soft reference
		NPCClass = Entry.NPCClass.LoadSynchronous();
	}

	if (!NPCClass)
	{
		// Fallback to default NPC character
		NPCClass = ASEENPCCharacter::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FRotator SpawnRotation = FRotator::ZeroRotator;

	ASEENPCCharacter* NPC = World->SpawnActor<ASEENPCCharacter>(NPCClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!NPC)
	{
		return nullptr;
	}

	// Configure the NPC
	NPC->Archetype = Entry.Archetype;
	NPC->HomeZone = Entry.HomeZone;

	// Set up schedule
	if (Entry.Schedule.Num() > 0)
	{
		UNPCScheduleComponent* ScheduleComp = NPC->GetScheduleComponent();
		if (ScheduleComp)
		{
			ScheduleComp->SetSchedule(Entry.Schedule);
		}
	}

	// Configure detection component
	UDetectionComponent* DetectionComp = NPC->GetDetectionComponent();
	if (DetectionComp)
	{
		// Set radio access based on spawn data
		// (bHasRadio is set in the DetectionComponent's properties at spawn)
	}

	return NPC;
}

FVector UNPCSpawnerComponent::ResolveSpawnLocation(const FNPCSpawnEntry& Entry) const
{
	if (Entry.SpawnLocationTag.IsNone())
	{
		// Default to the owning actor's location
		return GetOwner()->GetActorLocation();
	}

	// Find actor with matching tag
	UWorld* World = GetWorld();
	if (!World)
	{
		return GetOwner()->GetActorLocation();
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->Tags.Contains(Entry.SpawnLocationTag))
		{
			return Actor->GetActorLocation();
		}
	}

	// Tag not found, fall back to owner location
	return GetOwner()->GetActorLocation();
}
