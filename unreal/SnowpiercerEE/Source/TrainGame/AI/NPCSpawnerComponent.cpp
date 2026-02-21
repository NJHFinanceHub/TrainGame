// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "NPCSpawnerComponent.h"
#include "NPCAIController.h"
#include "NPCScheduleComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UNPCSpawnerComponent::UNPCSpawnerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNPCSpawnerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache spawn rows for this car
	if (SpawnDataTable)
	{
		TArray<FNPCSpawnRow*> AllRows;
		SpawnDataTable->GetAllRows<FNPCSpawnRow>(TEXT("NPCSpawner"), AllRows);

		for (const FNPCSpawnRow* Row : AllRows)
		{
			if (Row && Row->CarTag == CarTag)
			{
				CachedSpawnRows.Add(*Row);
			}
		}
	}

	if (bSpawnOnBeginPlay)
	{
		SpawnNPCs();
	}
}

void UNPCSpawnerComponent::SpawnNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (const FNPCSpawnRow& Row : CachedSpawnRows)
	{
		for (int32 i = 0; i < Row.SpawnCount && SpawnedNPCs.Num() < MaxNPCs; ++i)
		{
			APawn* SpawnedNPC = SpawnSingleNPC(Row, i);
			if (SpawnedNPC)
			{
				SpawnedNPCs.Add(SpawnedNPC);
				ConfigureNPCAI(SpawnedNPC, Row);
			}
		}
	}
}

void UNPCSpawnerComponent::DespawnAll()
{
	for (APawn* NPC : SpawnedNPCs)
	{
		if (NPC && IsValid(NPC))
		{
			NPC->Destroy();
		}
	}
	SpawnedNPCs.Empty();
}

void UNPCSpawnerComponent::RespawnNPC(int32 SpawnIndex)
{
	if (!CachedSpawnRows.IsValidIndex(0)) return;

	// Find the appropriate row — for simplicity, use the first matching row
	const FNPCSpawnRow& Row = CachedSpawnRows[0];
	APawn* NewNPC = SpawnSingleNPC(Row, SpawnIndex);
	if (NewNPC)
	{
		SpawnedNPCs.Add(NewNPC);
		ConfigureNPCAI(NewNPC, Row);
	}
}

int32 UNPCSpawnerComponent::GetActiveNPCCount() const
{
	int32 Count = 0;
	for (const APawn* NPC : SpawnedNPCs)
	{
		if (NPC && IsValid(NPC))
		{
			Count++;
		}
	}
	return Count;
}

APawn* UNPCSpawnerComponent::SpawnSingleNPC(const FNPCSpawnRow& Row, int32 SpawnPointIndex)
{
	UWorld* World = GetWorld();
	if (!World || !Row.NPCClass) return nullptr;

	FVector SpawnLoc = GetSpawnLocation(Row, SpawnPointIndex);
	FRotator SpawnRot = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	return World->SpawnActor<APawn>(Row.NPCClass, SpawnLoc, SpawnRot, SpawnParams);
}

FVector UNPCSpawnerComponent::GetSpawnLocation(const FNPCSpawnRow& Row, int32 SpawnPointIndex) const
{
	// Try tagged spawn points first
	if (Row.SpawnPointTags.IsValidIndex(SpawnPointIndex))
	{
		TArray<AActor*> TaggedActors = FindTaggedActors(Row.SpawnPointTags[SpawnPointIndex]);
		if (TaggedActors.Num() > 0)
		{
			return TaggedActors[0]->GetActorLocation();
		}
	}

	// Fallback: random point on nav mesh near the car
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation NavLoc;
		AActor* Owner = GetOwner();
		FVector Origin = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;

		if (NavSys->GetRandomReachablePointInRadius(Origin, 500.f, NavLoc))
		{
			return NavLoc.Location;
		}
	}

	// Last resort: owner location with offset
	AActor* Owner = GetOwner();
	FVector Base = Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
	return Base + FVector(FMath::RandRange(-200.f, 200.f), FMath::RandRange(-100.f, 100.f), 0.f);
}

TArray<AActor*> UNPCSpawnerComponent::FindTaggedActors(FName Tag) const
{
	TArray<AActor*> Result;
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, AllActors);

	// Filter to actors within reasonable range of this car
	AActor* Owner = GetOwner();
	if (!Owner) return AllActors;

	float MaxRange = 2000.f;
	for (AActor* Actor : AllActors)
	{
		if (Actor && FVector::Dist(Actor->GetActorLocation(), Owner->GetActorLocation()) < MaxRange)
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

void UNPCSpawnerComponent::ConfigureNPCAI(APawn* SpawnedPawn, const FNPCSpawnRow& Row)
{
	if (!SpawnedPawn) return;

	// Configure schedule component if present
	UNPCScheduleComponent* ScheduleComp = SpawnedPawn->FindComponentByClass<UNPCScheduleComponent>();
	if (ScheduleComp && Row.Schedule.Num() > 0)
	{
		ScheduleComp->SetSchedule(Row.Schedule);
	}

	// Configure AI controller
	ANPCAIController* AIController = Cast<ANPCAIController>(SpawnedPawn->GetController());
	if (!AIController)
	{
		// Auto-possess if controller not yet assigned
		SpawnedPawn->SpawnDefaultController();
		AIController = Cast<ANPCAIController>(SpawnedPawn->GetController());
	}

	if (AIController)
	{
		if (Row.Schedule.Num() > 0)
		{
			AIController->SetSchedule(Row.Schedule);
		}

		// Resolve patrol waypoint locations from tags
		if (Row.PatrolWaypointTags.Num() > 0)
		{
			TArray<FVector> Waypoints;
			for (const FName& WaypointTag : Row.PatrolWaypointTags)
			{
				TArray<AActor*> TaggedActors = FindTaggedActors(WaypointTag);
				for (AActor* Actor : TaggedActors)
				{
					Waypoints.Add(Actor->GetActorLocation());
				}
			}
			AIController->SetPatrolWaypoints(Waypoints);
		}
	}
}
