// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BodyDiscoveryComponent.h"
#include "JackbootAIController.h"
#include "CrowdNPCController.h"
#include "NPCAIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

// Static body registry
TArray<UBodyDiscoveryComponent::FBodyEntry> UBodyDiscoveryComponent::BodyRegistry;

UBodyDiscoveryComponent::UBodyDiscoveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Stagger scans
}

void UBodyDiscoveryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ScanTimer -= DeltaTime;
	if (ScanTimer <= 0.f)
	{
		ScanTimer = ScanInterval;
		ScanForBodies();
	}
}

// --- Static Body Registration ---

void UBodyDiscoveryComponent::RegisterBody(AActor* Body, EBodyState State)
{
	if (!Body) return;

	// Check if already registered
	for (FBodyEntry& Entry : BodyRegistry)
	{
		if (Entry.Body.Get() == Body)
		{
			Entry.State = State;
			return;
		}
	}

	FBodyEntry NewEntry;
	NewEntry.Body = Body;
	NewEntry.State = State;
	BodyRegistry.Add(NewEntry);
}

void UBodyDiscoveryComponent::UnregisterBody(AActor* Body)
{
	BodyRegistry.RemoveAll([Body](const FBodyEntry& Entry)
	{
		return Entry.Body.Get() == Body || !Entry.Body.IsValid();
	});
}

void UBodyDiscoveryComponent::UpdateBodyState(AActor* Body, EBodyState NewState)
{
	for (FBodyEntry& Entry : BodyRegistry)
	{
		if (Entry.Body.Get() == Body)
		{
			Entry.State = NewState;
			return;
		}
	}
}

TArray<AActor*> UBodyDiscoveryComponent::GetAllRegisteredBodies()
{
	TArray<AActor*> Result;

	// Clean up stale entries while we're at it
	BodyRegistry.RemoveAll([](const FBodyEntry& Entry)
	{
		return !Entry.Body.IsValid();
	});

	for (const FBodyEntry& Entry : BodyRegistry)
	{
		if (Entry.Body.IsValid())
		{
			Result.Add(Entry.Body.Get());
		}
	}

	return Result;
}

// --- Discovery ---

bool UBodyDiscoveryComponent::HasDiscoveredBody(AActor* Body) const
{
	return DiscoveredBodies.Contains(Body);
}

void UBodyDiscoveryComponent::ScanForBodies()
{
	// Clean stale entries from registry
	BodyRegistry.RemoveAll([](const FBodyEntry& Entry)
	{
		return !Entry.Body.IsValid();
	});

	AActor* Owner = GetOwner();
	if (!Owner) return;

	for (const FBodyEntry& Entry : BodyRegistry)
	{
		AActor* Body = Entry.Body.Get();
		if (!Body) continue;

		// Skip hidden bodies unless we can detect them
		if (Entry.State == EBodyState::Hidden && !bCanFindHiddenBodies) continue;

		// Skip already discovered
		if (DiscoveredBodies.Contains(Body)) continue;

		// Range check
		float Dist = FVector::Dist(Owner->GetActorLocation(), Body->GetActorLocation());
		if (Dist > DiscoveryRange) continue;

		// Vision check
		if (CanSeeBody(Body))
		{
			DiscoveredBodies.Add(Body);
			OnBodyFound.Broadcast(Body, Entry.State, Owner);
			ReactToBody(Body, Entry.State);
		}
	}
}

bool UBodyDiscoveryComponent::CanSeeBody(AActor* Body) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Body) return false;

	// Direction check (vision cone)
	FVector ToBody = (Body->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
	FVector Forward = Owner->GetActorForwardVector();

	float DotProduct = FVector::DotProduct(Forward, ToBody);
	float ConeThreshold = FMath::Cos(FMath::DegreesToRadians(DiscoveryVisionAngle));

	if (DotProduct < ConeThreshold) return false;

	// Line of sight trace
	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.AddIgnoredActor(Body);

	UWorld* World = GetWorld();
	if (!World) return false;

	// Trace from NPC eyes to body
	FVector TraceStart = Owner->GetActorLocation() + FVector(0.f, 0.f, 80.f); // Eye height
	FVector TraceEnd = Body->GetActorLocation();

	bool bBlocked = World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	return !bBlocked;
}

void UBodyDiscoveryComponent::ReactToBody(AActor* Body, EBodyState State)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	APawn* OwnerPawn = Cast<APawn>(Owner);
	if (!OwnerPawn) return;

	AController* Controller = OwnerPawn->GetController();
	if (!Controller) return;

	// Jackboot reaction: raise alert, call backup
	if (AJackbootAIController* JackbootAI = Cast<AJackbootAIController>(Controller))
	{
		JackbootAI->OnBodyDiscovered(Body, State);
		return;
	}

	// Generic NPC reaction: investigate or flee
	if (ANPCAIController* NPCAI = Cast<ANPCAIController>(Controller))
	{
		if (State == EBodyState::Dead)
		{
			NPCAI->SetAIState(ENPCAIState::Fleeing);
		}
		else
		{
			NPCAI->SetInvestigationLocation(Body->GetActorLocation());
			NPCAI->SetAIState(ENPCAIState::Investigating);
		}
		return;
	}

	// Crowd NPC reaction: flee
	if (ACrowdNPCController* CrowdAI = Cast<ACrowdNPCController>(Controller))
	{
		CrowdAI->TriggerFlee(Body->GetActorLocation());
	}
}
