// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "DeckAccessPoint.h"
#include "TransportDeckSubsystem.h"
#include "EngineUtils.h"

ADeckAccessPoint::ADeckAccessPoint()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADeckAccessPoint::BeginPlay()
{
	Super::BeginPlay();

	// Register with subsystem
	UWorld* World = GetWorld();
	if (World)
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub)
		{
			DeckSub->RegisterAccessPoint(Config);
		}
	}
}

bool ADeckAccessPoint::TryUse(AActor* User, bool bGoingDown)
{
	if (!User)
	{
		return false;
	}

	if (IsLocked())
	{
		return false;
	}

	GenerateAccessNoise();
	return true;
}

bool ADeckAccessPoint::TryUnlock(int32 CunningScore)
{
	if (!Config.bLocked || bUnlocked)
	{
		return true;
	}

	if (Config.LockDifficulty <= 0 || CunningScore >= Config.LockDifficulty)
	{
		bUnlocked = true;

		// Update subsystem registration
		UWorld* World = GetWorld();
		if (World)
		{
			UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
			if (DeckSub)
			{
				FDeckAccessConfig UnlockedConfig = Config;
				UnlockedConfig.bLocked = false;
				DeckSub->RegisterAccessPoint(UnlockedConfig);
			}
		}

		return true;
	}

	// Failed unlock generates noise
	GenerateAccessNoise();
	return false;
}

void ADeckAccessPoint::Discover()
{
	bDiscovered = true;
}

void ADeckAccessPoint::GenerateAccessNoise()
{
	if (Config.NoiseRadius <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<APawn> It(World); It; ++It)
	{
		APawn* NPC = *It;
		float Dist = FVector::Dist(GetActorLocation(), NPC->GetActorLocation());
		if (Dist <= Config.NoiseRadius * 100.0f)
		{
			// Detection system handles alert propagation
		}
	}
}
