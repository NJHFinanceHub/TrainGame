#include "SecretCarManager.h"

void USecretCarManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeRequirements();
}

bool USecretCarManager::CanAccessSecretCar(ESecretCar Car) const
{
    return Requirements.Contains(Car);
}

void USecretCarManager::DiscoverSecretCar(ESecretCar Car)
{
    if (!DiscoveryState.Contains(Car) || !DiscoveryState[Car])
    {
        DiscoveryState.Add(Car, true);
        OnSecretCarDiscovered.Broadcast(Car);
        CheckMilestones();
    }
}

bool USecretCarManager::IsDiscovered(ESecretCar Car) const
{
    const bool* Found = DiscoveryState.Find(Car);
    return Found && *Found;
}

int32 USecretCarManager::GetDiscoveredCount() const
{
    int32 Count = 0;
    for (const auto& Pair : DiscoveryState)
    {
        if (Pair.Value)
        {
            Count++;
        }
    }
    return Count;
}

bool USecretCarManager::AllCarsDiscovered() const
{
    return GetDiscoveredCount() >= 9;
}

FSecretCarRequirement USecretCarManager::GetRequirements(ESecretCar Car) const
{
    if (const FSecretCarRequirement* Found = Requirements.Find(Car))
    {
        return *Found;
    }
    return FSecretCarRequirement();
}

TArray<FText> USecretCarManager::GetUnmetRequirements(ESecretCar Car) const
{
    return TArray<FText>();
}

bool USecretCarManager::HasPathfinderTitle() const
{
    return GetDiscoveredCount() >= 3;
}

bool USecretCarManager::HasSixthSense() const
{
    return GetDiscoveredCount() >= 5;
}

bool USecretCarManager::HasMasterExplorer() const
{
    return GetDiscoveredCount() >= 7;
}

bool USecretCarManager::HasGhostOfTheTrain() const
{
    return GetDiscoveredCount() >= 9;
}

void USecretCarManager::InitializeRequirements()
{
}

void USecretCarManager::CheckMilestones()
{
    int32 Count = GetDiscoveredCount();
    if (Count == 3 || Count == 5 || Count == 7 || Count == 9)
    {
        OnSecretCarMilestone.Broadcast(Count);
    }
}
