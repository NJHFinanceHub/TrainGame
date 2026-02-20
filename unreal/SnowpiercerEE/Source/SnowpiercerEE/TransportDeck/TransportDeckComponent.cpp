// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "TransportDeckComponent.h"
#include "TransportDeckSubsystem.h"
#include "EngineUtils.h"

UTransportDeckComponent::UTransportDeckComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTransportDeckComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bOnLowerDeck)
	{
		UpdateDeckCarTracking();
	}
}

bool UTransportDeckComponent::EnterDeck(int32 CarIndex, EDeckAccessType AccessType)
{
	if (!UTransportDeckSubsystem::HasLowerDeck(CarIndex))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
	if (!DeckSub)
	{
		return false;
	}

	// Check for unlocked access at this car
	TArray<FDeckAccessConfig> AccessList = DeckSub->GetAccessPointsForCar(CarIndex);
	bool bFoundAccess = false;
	for (const FDeckAccessConfig& AP : AccessList)
	{
		if (AP.AccessType == AccessType && !AP.bLocked)
		{
			bFoundAccess = true;
			// Generate entry noise
			GenerateNoise(AP.NoiseRadius);
			break;
		}
	}

	// Allow entry even without registered access points (design-time flexibility)
	bOnLowerDeck = true;
	CurrentDeckCarIndex = CarIndex;
	OnDeckEntered.Broadcast(CarIndex);
	return true;
}

bool UTransportDeckComponent::ExitDeck()
{
	if (!bOnLowerDeck)
	{
		return false;
	}

	if (bOnCart)
	{
		DismountCart();
	}

	bOnLowerDeck = false;
	OnDeckExited.Broadcast();
	return true;
}

bool UTransportDeckComponent::BoardCart(FName CartID)
{
	if (!bOnLowerDeck || bOnCart)
	{
		return false;
	}

	bOnCart = true;
	CurrentCartID = CartID;
	OnCartBoarded.Broadcast(CartID, CurrentDeckCarIndex);
	return true;
}

void UTransportDeckComponent::DismountCart()
{
	if (!bOnCart)
	{
		return;
	}

	FName PrevCartID = CurrentCartID;
	bOnCart = false;
	CurrentCartID = NAME_None;
	OnCartExited.Broadcast(PrevCartID);
}

bool UTransportDeckComponent::LootCargo(FName CargoID)
{
	if (!bOnLowerDeck)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
	if (!DeckSub)
	{
		return false;
	}

	// Looting generates noise
	GenerateNoise(BaseWalkNoise * 2.0f);

	return DeckSub->RemoveCargo(CargoID);
}

bool UTransportDeckComponent::DisableSecurity(AActor* SecurityDevice, int32 CunningScore)
{
	if (!SecurityDevice)
	{
		return false;
	}

	// Base DC 8, modified by alert level
	UWorld* World = GetWorld();
	int32 DC = 8;
	if (World)
	{
		UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
		if (DeckSub)
		{
			switch (DeckSub->GetAlertLevel())
			{
			case EDeckAlertLevel::Caution: DC += 2; break;
			case EDeckAlertLevel::Alert: DC += 4; break;
			case EDeckAlertLevel::Lockdown: DC += 8; break;
			default: break;
			}
		}
	}

	if (CunningScore >= DC)
	{
		// Minimal noise on success
		GenerateNoise(CrouchNoise);
		return true;
	}

	// Failed — trigger alert
	OnSecurityTriggered.Broadcast(EDeckSecurityType::MotionSensor, SecurityDevice->GetActorLocation());
	GenerateNoise(RunNoise);
	return false;
}

bool UTransportDeckComponent::SwitchTrack(FName JunctionID, FName TargetBranch)
{
	if (!bOnLowerDeck)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UTransportDeckSubsystem* DeckSub = World->GetSubsystem<UTransportDeckSubsystem>();
	if (!DeckSub)
	{
		return false;
	}

	// Switching tracks makes noise
	GenerateNoise(BaseWalkNoise * 1.5f);

	return DeckSub->SwitchJunction(JunctionID, TargetBranch);
}

float UTransportDeckComponent::GetCurrentNoiseRadius(bool bCrouching, bool bRunning) const
{
	float BaseNoise = BaseWalkNoise;
	if (bCrouching) BaseNoise = CrouchNoise;
	if (bRunning) BaseNoise = RunNoise;

	return FMath::Max(0.0f, BaseNoise - MachineryMasking);
}

bool UTransportDeckComponent::CanEnterDeckAt(int32 CarIndex) const
{
	return UTransportDeckSubsystem::HasLowerDeck(CarIndex);
}

void UTransportDeckComponent::UpdateDeckCarTracking()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Each car is 10000 units (100m) at 10x scale
	float XPos = Owner->GetActorLocation().X;
	int32 NewCarIndex = FMath::FloorToInt32(XPos / 10000.0f);
	NewCarIndex = FMath::Clamp(NewCarIndex, 15, 82);

	if (NewCarIndex != CurrentDeckCarIndex)
	{
		CurrentDeckCarIndex = NewCarIndex;
	}
}

void UTransportDeckComponent::GenerateNoise(float Radius)
{
	if (Radius <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return;
	}

	float EffectiveRadius = FMath::Max(0.0f, Radius - MachineryMasking);
	if (EffectiveRadius <= 0.0f)
	{
		return;
	}

	// Alert nearby NPCs within noise radius (converts meters to cm)
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* NPC = *It;
		if (NPC == Owner)
		{
			continue;
		}

		float Dist = FVector::Dist(Owner->GetActorLocation(), NPC->GetActorLocation());
		if (Dist <= EffectiveRadius * 100.0f)
		{
			// Detection system handles alert propagation
		}
	}
}
