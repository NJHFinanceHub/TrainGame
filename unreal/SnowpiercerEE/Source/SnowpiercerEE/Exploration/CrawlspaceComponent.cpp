#include "CrawlspaceComponent.h"
#include "EngineUtils.h"

UCrawlspaceComponent::UCrawlspaceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCrawlspaceComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bInCrawlspace)
    {
        UpdateCarTracking();
    }
}

void UCrawlspaceComponent::EnterCrawlspace(int32 CarNumber)
{
    bInCrawlspace = true;
    CurrentCarNumber = CarNumber;
    OnCrawlspaceEntered.Broadcast(CarNumber);
}

void UCrawlspaceComponent::ExitCrawlspace()
{
    bInCrawlspace = false;
    OnCrawlspaceExited.Broadcast();
}

void UCrawlspaceComponent::SetCrawlSpeed(ECrawlSpeed Speed)
{
    CurrentSpeed = Speed;
}

float UCrawlspaceComponent::GetCurrentNoiseRadius() const
{
    float NoiseMultiplier = 1.0f;
    switch (CurrentSpeed)
    {
    case ECrawlSpeed::Slow: NoiseMultiplier = 0.5f; break;
    case ECrawlSpeed::Normal: NoiseMultiplier = 1.0f; break;
    case ECrawlSpeed::Fast: NoiseMultiplier = 2.0f; break;
    case ECrawlSpeed::Squeeze: NoiseMultiplier = 0.25f; break;
    }

    return FMath::Max(0.0f, BaseNoiseRadius * NoiseMultiplier - TrainMotionMasking);
}

float UCrawlspaceComponent::GetCurrentStaminaCost() const
{
    switch (CurrentSpeed)
    {
    case ECrawlSpeed::Slow: return BaseStaminaCost * 0.5f;
    case ECrawlSpeed::Fast: return BaseStaminaCost * 2.5f;
    case ECrawlSpeed::Squeeze: return BaseStaminaCost * 3.0f;
    default: return BaseStaminaCost;
    }
}

float UCrawlspaceComponent::GetCurrentMoveSpeed() const
{
    switch (CurrentSpeed)
    {
    case ECrawlSpeed::Slow: return 0.5f;
    case ECrawlSpeed::Normal: return 1.0f;
    case ECrawlSpeed::Fast: return 1.8f;
    case ECrawlSpeed::Squeeze: return 0.3f;
    default: return 1.0f;
    }
}

bool UCrawlspaceComponent::IsNearObservationGrate() const
{
    return false;
}

void UCrawlspaceComponent::BeginEavesdrop()
{
}

bool UCrawlspaceComponent::TryHackJunctionBox(int32 CunningScore)
{
    return CunningScore >= 5;
}

ECrawlspaceHazard UCrawlspaceComponent::GetCurrentHazard() const
{
    return ECrawlspaceHazard::None;
}

float UCrawlspaceComponent::ProcessHazard(ECrawlspaceHazard Hazard, int32 AgilityScore, int32 PerceptionScore)
{
    float Damage = 0.0f;

    switch (Hazard)
    {
    case ECrawlspaceHazard::DriveShaft:
        // Agility check to avoid damage
        Damage = (AgilityScore >= 6) ? 0.0f : 30.0f;
        break;
    case ECrawlspaceHazard::SteamVent:
        // Perception to spot, agility to dodge
        Damage = (PerceptionScore >= 5) ? 0.0f : 15.0f;
        break;
    case ECrawlspaceHazard::Electrical:
        Damage = (AgilityScore >= 7) ? 5.0f : 25.0f;
        break;
    case ECrawlspaceHazard::LooseGrating:
        // Generates noise on failure
        if (AgilityScore < 5)
        {
            GenerateNoise(BaseNoiseRadius * 3.0f);
            Damage = 10.0f;
        }
        break;
    case ECrawlspaceHazard::IceFormation:
        Damage = 5.0f; // Always some cold damage
        if (AgilityScore < 4) Damage += 15.0f;
        break;
    default:
        break;
    }

    if (Damage > 0.0f)
    {
        OnHazardTriggered.Broadcast(Hazard);
    }
    return Damage;
}

void UCrawlspaceComponent::GenerateNoise(float Radius)
{
    if (Radius <= 0.0f) return;

    // Alert nearby NPCs within noise radius
    AActor* Owner = GetOwner();
    if (!Owner) return;

    TArray<AActor*> OverlappingActors;
    // Use a simple distance check for NPCs in range
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        APawn* NPC = *It;
        if (NPC == Owner) continue;

        float Dist = FVector::Dist(Owner->GetActorLocation(), NPC->GetActorLocation());
        if (Dist <= Radius * 100.0f) // Convert meters to cm
        {
            // NPC detection system will handle the alert via hearing range
        }
    }
}

void UCrawlspaceComponent::UpdateCarTracking()
{
    // Track which car the player is under based on X position
    // Each car is 10000 units (100m) long
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float XPos = Owner->GetActorLocation().X;
    int32 NewCarNumber = FMath::FloorToInt32(XPos / 10000.0f);
    if (NewCarNumber != CurrentCarNumber)
    {
        CurrentCarNumber = NewCarNumber;
    }
}
