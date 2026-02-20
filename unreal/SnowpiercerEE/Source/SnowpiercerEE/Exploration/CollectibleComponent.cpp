#include "CollectibleComponent.h"

UCollectibleComponent::UCollectibleComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCollectibleComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCollectibleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateProximityDetection(DeltaTime);
}

bool UCollectibleComponent::TryPickup(AActor* Collector)
{
    if (!Collector || !bIsActivated || !MeetsStatRequirements(Collector))
    {
        return false;
    }

    ApplyPickupEffects(Collector);
    OnPickedUp.Broadcast(CollectibleRowName, Collector);
    return true;
}

void UCollectibleComponent::Activate()
{
    bIsActivated = true;
}

float UCollectibleComponent::GetDetectionRadius(int32 PerceptionScore) const
{
    if (DetectionRadiusOverride > 0.0f)
    {
        return DetectionRadiusOverride;
    }

    return 200.0f + PerceptionScore * 50.0f;
}

bool UCollectibleComponent::MeetsStatRequirements(AActor* Collector) const
{
    return Collector != nullptr;
}

void UCollectibleComponent::UpdateProximityDetection(float DeltaTime)
{
}

void UCollectibleComponent::ApplyPickupEffects(AActor* Collector)
{
}

void UCollectibleComponent::TriggerCompanionResonance(AActor* Collector)
{
}
