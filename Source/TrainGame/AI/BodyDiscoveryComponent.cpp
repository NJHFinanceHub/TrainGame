// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BodyDiscoveryComponent.h"
#include "SEEAIController.h"
#include "JackbootAIController.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

UBodyDiscoveryComponent::UBodyDiscoveryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void UBodyDiscoveryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBodyDiscoveryComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ScanTimer += DeltaTime;
	if (ScanTimer >= ScanInterval)
	{
		ScanTimer = 0.f;
		ScanForBodies();
	}
}

bool UBodyDiscoveryComponent::ScanForBodies()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector MyLocation = Owner->GetActorLocation();
	bool bFoundNew = false;

	// Scan for characters with body states (unconscious/dead)
	for (TActorIterator<ACharacter> It(World); It; ++It)
	{
		ACharacter* Character = *It;
		if (!Character || Character == Owner || Character->IsPendingKillPending())
		{
			continue;
		}

		// Skip if too far
		float Distance = FVector::Dist(MyLocation, Character->GetActorLocation());
		if (Distance > ScanRange)
		{
			continue;
		}

		// Skip if already known
		if (IsBodyAlreadyKnown(Character))
		{
			continue;
		}

		// Check if this character is "down" — we look for a character whose
		// movement mode indicates ragdoll/incapacitation, or check via interface.
		// For now, we check if the character is in ragdoll (no movement component active)
		// or has been tagged as a body.
		if (!Character->ActorHasTag(TEXT("Body")))
		{
			continue;
		}

		// Line of sight check
		if (bRequiresLineOfSight && !HasLineOfSightToBody(Character))
		{
			continue;
		}

		// Determine body state from tags
		EBodyState BodyState = EBodyState::Unconscious;
		if (Character->ActorHasTag(TEXT("Dead")))
		{
			BodyState = EBodyState::Dead;
		}
		else if (Character->ActorHasTag(TEXT("Stunned")))
		{
			BodyState = EBodyState::Stunned;
		}

		ReportBody(Character, BodyState);
		bFoundNew = true;
	}

	return bFoundNew;
}

void UBodyDiscoveryComponent::ReportBody(AActor* Body, EBodyState BodyState)
{
	if (!Body || IsBodyAlreadyKnown(Body))
	{
		return;
	}

	KnownBodies.Add(Body);

	FBodyDiscoveryReport Report;
	Report.Body = Body;
	Report.BodyState = BodyState;
	Report.DiscoveryLocation = Body->GetActorLocation();
	Report.Discoverer = GetOwner();
	Report.GameTimeDiscovered = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	DiscoveredBodies.Add(Report);

	// Notify the AI controller on our owner
	AActor* Owner = GetOwner();
	if (Owner)
	{
		APawn* OwnerPawn = Cast<APawn>(Owner);
		if (OwnerPawn)
		{
			// Notify detection component
			UDetectionComponent* DetComp = OwnerPawn->FindComponentByClass<UDetectionComponent>();
			if (DetComp)
			{
				DetComp->NotifyBodyDiscovered(Body, BodyState);
			}

			// If Jackboot, escalate alert
			AJackbootAIController* JackbootAI = Cast<AJackbootAIController>(OwnerPawn->GetController());
			if (JackbootAI)
			{
				EJackbootAlertLevel NewLevel = (BodyState == EBodyState::Dead)
					? EJackbootAlertLevel::Red
					: EJackbootAlertLevel::Orange;
				JackbootAI->RaiseAlert(NewLevel, Body->GetActorLocation());
			}
		}
	}

	// Broadcast delegate
	OnBodyFound.Broadcast(Report);

	// Propagate alert to nearby NPCs
	PropagateBodyAlert(Report);
}

// --- Private ---

void UBodyDiscoveryComponent::PropagateBodyAlert(const FBodyDiscoveryReport& Report)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FVector MyLocation = Owner->GetActorLocation();

	// Alert all NPCs within propagation radius
	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* OtherPawn = *It;
		if (!OtherPawn || OtherPawn == Owner || OtherPawn->IsPendingKillPending())
		{
			continue;
		}

		float Distance = FVector::Dist(MyLocation, OtherPawn->GetActorLocation());
		if (Distance > AlertPropagationRadius)
		{
			continue;
		}

		// Notify detection component on the other NPC
		UDetectionComponent* OtherDetComp = OtherPawn->FindComponentByClass<UDetectionComponent>();
		if (OtherDetComp)
		{
			OtherDetComp->NotifyBodyDiscovered(Report.Body, Report.BodyState);
		}
	}
}

bool UBodyDiscoveryComponent::IsBodyAlreadyKnown(AActor* Body) const
{
	return KnownBodies.Contains(Body);
}

bool UBodyDiscoveryComponent::HasLineOfSightToBody(AActor* Body) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Body)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FHitResult HitResult;
	FVector Start = Owner->GetActorLocation() + FVector(0, 0, 60.f); // Eye height offset
	FVector End = Body->GetActorLocation();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);
	QueryParams.AddIgnoredActor(Body);

	bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End,
		ECollisionChannel::ECC_Visibility, QueryParams);

	return !bHit; // No obstruction = has line of sight
}
