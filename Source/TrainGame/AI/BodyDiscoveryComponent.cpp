// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BodyDiscoveryComponent.h"
#include "SEENPCCharacter.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"

UBodyDiscoveryComponent::UBodyDiscoveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;
}

void UBodyDiscoveryComponent::BeginPlay()
{
	Super::BeginPlay();

	DetectionComp = GetOwner()->FindComponentByClass<UDetectionComponent>();
}

void UBodyDiscoveryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ScanTimer += DeltaTime;
	if (ScanTimer >= ScanInterval)
	{
		ScanTimer = 0.f;
		ScanForBodies();
	}
}

void UBodyDiscoveryComponent::ScanForBodies()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector MyLocation = Owner->GetActorLocation();
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ASEENPCCharacter> It(World); It; ++It)
	{
		ASEENPCCharacter* OtherNPC = *It;
		if (!OtherNPC || OtherNPC == Owner)
		{
			continue;
		}

		// Skip already discovered bodies
		if (DiscoveredBodies.Contains(OtherNPC))
		{
			continue;
		}

		// Check if this NPC is incapacitated
		if (!OtherNPC->IsIncapacitated())
		{
			continue;
		}

		// Hidden bodies can't be discovered
		if (OtherNPC->GetBodyState() == EBodyState::Hidden)
		{
			continue;
		}

		// Distance check
		float Dist = FVector::Dist(MyLocation, OtherNPC->GetActorLocation());
		if (Dist > ScanRadius)
		{
			continue;
		}

		// Line of sight check
		if (bRequiresLineOfSight && DetectionComp)
		{
			if (!DetectionComp->HasLineOfSightTo(OtherNPC))
			{
				continue;
			}
		}

		// Body found!
		HandleBodyFound(OtherNPC, OtherNPC->GetBodyState());
	}
}

bool UBodyDiscoveryComponent::IsDiscoverableBody(AActor* Actor) const
{
	ASEENPCCharacter* NPC = Cast<ASEENPCCharacter>(Actor);
	if (!NPC)
	{
		return false;
	}

	return NPC->IsIncapacitated() && NPC->GetBodyState() != EBodyState::Hidden;
}

void UBodyDiscoveryComponent::HandleBodyFound(AActor* Body, EBodyState State)
{
	DiscoveredBodies.Add(Body);
	bHasDiscoveredBody = true;

	// Notify the detection component
	if (DetectionComp)
	{
		DetectionComp->NotifyBodyDiscovered(Body, State);
	}

	// Broadcast the discovery event
	FBodyDiscoveryEvent Event;
	Event.DiscoveredBody = Body;
	Event.Discoverer = GetOwner();
	Event.BodyState = State;
	Event.DiscoveryLocation = Body->GetActorLocation();

	OnBodyFound.Broadcast(Event);
}
