#include "CrawlspaceComponent.h"

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
    return 0.0f;
}

void UCrawlspaceComponent::GenerateNoise(float Radius)
{
}

void UCrawlspaceComponent::UpdateCarTracking()
{
}
