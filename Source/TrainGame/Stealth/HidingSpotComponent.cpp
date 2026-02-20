// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "HidingSpotComponent.h"

UHidingSpotComponent::UHidingSpotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHidingSpotComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UHidingSpotComponent::Occupy(AActor* Occupant)
{
	if (!Occupant || !IsAvailable())
	{
		return false;
	}

	// TODO: If bRequiresSkillCheck, validate player stat against SkillCheckDC
	// TODO: Play enter animation on the hiding spot actor

	CurrentOccupant = Occupant;
	return true;
}

void UHidingSpotComponent::Vacate()
{
	// TODO: Play exit animation
	CurrentOccupant = nullptr;
}

bool UHidingSpotComponent::StoreBody(AActor* Body)
{
	if (!Body || HasStoredBody())
	{
		return false;
	}

	// A body occupies the spot — can't share with a living occupant
	if (IsOccupied())
	{
		return false;
	}

	// TODO: Play body placement animation
	// TODO: Hide the body actor visually
	StoredBody = Body;
	return true;
}

AActor* UHidingSpotComponent::RetrieveBody()
{
	AActor* Body = StoredBody;
	StoredBody = nullptr;
	// TODO: Make body visible again
	return Body;
}
