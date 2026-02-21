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
    // Bunker Car - accessible from Tail, requires crawlspace
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone1_Tail;
        Req.bRequiresCrawlspaceAccess = true;
        Req.StatRequirements.Add(TEXT("Perception"), 4);
        Requirements.Add(ESecretCar::BunkerCar, Req);
    }

    // Ghost Car - abandoned car between zones, requires hearing rumors
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone2_ThirdClass;
        Req.NarrativeFlag = TEXT("heard_ghost_car_rumor");
        Req.StatRequirements.Add(TEXT("Cunning"), 5);
        Requirements.Add(ESecretCar::GhostCar, Req);
    }

    // Rebel Archive - hidden in working spine, faction-gated
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone4_WorkingSpine;
        Req.FactionRequirements.Add(TEXT("TheThaw"), 25);
        Requirements.Add(ESecretCar::RebelArchive, Req);
    }

    // Outside Lab - exterior access required, cold suit mandatory
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone3_SecondClass;
        Req.bRequiresColdSuit = true;
        Req.bRequiresRooftopAccess = true;
        Req.StatRequirements.Add(TEXT("Agility"), 6);
        Requirements.Add(ESecretCar::OutsideLab, Req);
    }

    // Stowaway Car - between third and second class
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone2_ThirdClass;
        Req.bRequiresCrawlspaceAccess = true;
        Req.StatRequirements.Add(TEXT("Perception"), 6);
        Requirements.Add(ESecretCar::StowawayCar, Req);
    }

    // Mirror Car - first class, requires specific collectible
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone5_FirstClass;
        Req.RequiredCollectibles.Add(TEXT("mirror_key_fragment"));
        Req.StatRequirements.Add(TEXT("Cunning"), 7);
        Requirements.Add(ESecretCar::MirrorCar, Req);
    }

    // Prototype Car - near engine, high stat requirements
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone6_Sanctum;
        Req.StatRequirements.Add(TEXT("Strength"), 7);
        Req.StatRequirements.Add(TEXT("Perception"), 7);
        Requirements.Add(ESecretCar::PrototypeCar, Req);
    }

    // Nursery - hidden in Sanctum, companion required
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone6_Sanctum;
        Req.RequiredCompanion = TEXT("Yona");
        Req.RequiredLoyalty = 50;
        Requirements.Add(ESecretCar::Nursery, Req);
    }

    // Dead Man's Switch - engine room, all-or-nothing
    {
        FSecretCarRequirement Req;
        Req.MinimumZone = ECollectibleZone::Zone7_Engine;
        Req.NarrativeFlag = TEXT("engine_access_granted");
        Req.StatRequirements.Add(TEXT("Cunning"), 8);
        Requirements.Add(ESecretCar::DeadMansSwitch, Req);
    }
}

void USecretCarManager::CheckMilestones()
{
    int32 Count = GetDiscoveredCount();
    if (Count == 3 || Count == 5 || Count == 7 || Count == 9)
    {
        OnSecretCarMilestone.Broadcast(Count);
    }
}
