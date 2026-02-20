#include "CollectibleComponent.h"
#include "CollectibleJournalSubsystem.h"
#include "Engine/GameInstance.h"

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

void UCollectibleComponent::ActivateCollectible()
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
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn) return;

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    float DetectionRadius = GetDetectionRadius(5); // Default PER 5

    if (Distance <= DetectionRadius)
    {
        OnPlayerProximity.Broadcast(CollectibleRowName, Distance);

        // Glow intensity increases as player gets closer
        if (bAlwaysShowGlow || Distance < DetectionRadius * 0.5f)
        {
            // Visual feedback handled by Blueprint/material
        }
    }
}

void UCollectibleComponent::ApplyPickupEffects(AActor* Collector)
{
    if (!Collector) return;

    // Register with journal subsystem
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (GI)
    {
        UCollectibleJournalSubsystem* Journal = GI->GetSubsystem<UCollectibleJournalSubsystem>();
        if (Journal)
        {
            Journal->RegisterCollectible(CollectibleRowName);
        }
    }

    TriggerCompanionResonance(Collector);

    // Destroy the collectible actor after pickup
    GetOwner()->SetActorHiddenInGame(true);
    GetOwner()->SetActorEnableCollision(false);
    SetComponentTickEnabled(false);
}

void UCollectibleComponent::TriggerCompanionResonance(AActor* Collector)
{
    if (!CachedData || CachedData->ResonanceCompanion.IsNone()) return;

    // Companion resonance grants bonus loyalty when the matching companion is present
    // Implementation: check if companion actor with matching name is in party
    // For now, broadcast the event for external systems to handle
}
