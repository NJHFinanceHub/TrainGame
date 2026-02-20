// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCSpawnerComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

UNPCSpawnerComponent::UNPCSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 2.0f; // Check every 2 seconds
}

void UNPCSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoSpawn && !bDistanceManaged)
	{
		SpawnAllNPCs();
	}
}

void UNPCSpawnerComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDistanceManaged)
	{
		CheckPlayerDistance();
	}
}

void UNPCSpawnerComponent::SpawnAllNPCs()
{
	if (!SpawnTable || !GetOwner())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Read all rows from the spawn table
	TArray<FNPCSpawnEntry*> Rows;
	SpawnTable->GetAllRows<FNPCSpawnEntry>(TEXT("NPCSpawner"), Rows);

	int32 SpawnPointIndex = 0;

	for (const FNPCSpawnEntry* Entry : Rows)
	{
		if (!Entry || Entry->NPCClass.IsNull())
		{
			continue;
		}

		// Load the class synchronously (would be async in shipping)
		UClass* NPCClass = Entry->NPCClass.LoadSynchronous();
		if (!NPCClass)
		{
			continue;
		}

		for (int32 i = 0; i < Entry->Count; ++i)
		{
			// Pick a spawn location
			FVector SpawnLocation;
			if (SpawnPoints.Num() > 0)
			{
				SpawnLocation = SpawnPoints[SpawnPointIndex % SpawnPoints.Num()];
				SpawnPointIndex++;
			}
			else
			{
				// Default: offset from owner with some random spread
				FVector OwnerLoc = GetOwner()->GetActorLocation();
				SpawnLocation = OwnerLoc + FVector(
					FMath::RandRange(-300.f, 300.f),
					FMath::RandRange(-150.f, 150.f),
					0.f
				);
			}

			AActor* SpawnedNPC = SpawnSingleNPC(*Entry, SpawnLocation);
			if (SpawnedNPC)
			{
				SpawnedNPCs.Add(SpawnedNPC);
			}
		}
	}
}

void UNPCSpawnerComponent::DespawnAllNPCs()
{
	for (AActor* NPC : SpawnedNPCs)
	{
		if (NPC && !NPC->IsPendingKillPending())
		{
			NPC->Destroy();
		}
	}
	SpawnedNPCs.Empty();
}

void UNPCSpawnerComponent::CheckPlayerDistance()
{
	UWorld* World = GetWorld();
	if (!World || !GetOwner())
	{
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0);
	if (!Player)
	{
		return;
	}

	float Distance = FVector::Dist(Player->GetActorLocation(), GetOwner()->GetActorLocation());
	bool bNowInRange = Distance <= ActivationDistance;

	if (bNowInRange && !bPlayerInRange)
	{
		// Player entered range — spawn NPCs
		bPlayerInRange = true;
		if (bAutoSpawn && SpawnedNPCs.Num() == 0)
		{
			SpawnAllNPCs();
		}
	}
	else if (!bNowInRange && bPlayerInRange)
	{
		// Player left range — despawn NPCs
		bPlayerInRange = false;
		DespawnAllNPCs();
	}
}

AActor* UNPCSpawnerComponent::SpawnSingleNPC(const FNPCSpawnEntry& Entry, FVector Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	UClass* NPCClass = Entry.NPCClass.LoadSynchronous();
	if (!NPCClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);

	return World->SpawnActor<AActor>(NPCClass, Location, SpawnRotation, SpawnParams);
}
