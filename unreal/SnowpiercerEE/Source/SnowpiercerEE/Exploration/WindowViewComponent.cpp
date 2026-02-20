#include "WindowViewComponent.h"

UWindowViewComponent::UWindowViewComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWindowViewComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWindowViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateFrostState(DeltaTime);
    CheckLandmarkVisibility();
}

void UWindowViewComponent::WipeFrost()
{
    if (bCanWipeFrost)
    {
        bFrostWiped = true;
        FrostTimer = FrostClearDuration;
    }
}

EThawStage UWindowViewComponent::GetCurrentThawStage() const
{
    return EThawStage::DeepWinter;
}

ERouteSegment UWindowViewComponent::GetCurrentRouteSegment() const
{
    return ERouteSegment::Northern;
}

bool UWindowViewComponent::IsLandmarkVisible(FName LandmarkID) const
{
    return VisibleLandmarks.Contains(LandmarkID);
}

void UWindowViewComponent::BeginTelescopeView()
{
    if (bHasTelescope)
    {
        bInTelescopeView = true;
    }
}

void UWindowViewComponent::EndTelescopeView()
{
    bInTelescopeView = false;
}

TArray<FText> UWindowViewComponent::ProcessTelescopePerceptionCheck(int32 PerceptionScore) const
{
    return TArray<FText>();
}

float UWindowViewComponent::GetExteriorTemperature() const
{
    return -67.0f;
}

bool UWindowViewComponent::CanSeeExterior() const
{
    if (WindowType == ESEEWindowType::Sealed)
    {
        return false;
    }

    if (WindowType == ESEEWindowType::Passenger && !bFrostWiped)
    {
        return false;
    }

    return true;
}

void UWindowViewComponent::CheckLandmarkVisibility()
{
    if (!CanSeeExterior()) return;

    // Landmarks visible depend on route segment and thaw stage
    ERouteSegment Segment = GetCurrentRouteSegment();
    EThawStage Thaw = GetCurrentThawStage();

    // Check each registered landmark against current conditions
    // Landmark visibility is determined by car number, route, and thaw stage
    // In a full implementation, this would query a landmark data table
    // For now, landmarks in VisibleLandmarks are always visible when exterior is viewable
    for (const FName& Landmark : VisibleLandmarks)
    {
        OnLandmarkVisible.Broadcast(Landmark);
    }
}

void UWindowViewComponent::UpdateFrostState(float DeltaTime)
{
    if (bFrostWiped)
    {
        FrostTimer -= DeltaTime;
        if (FrostTimer <= 0.0f)
        {
            bFrostWiped = false;
            FrostTimer = 0.0f;
        }
    }
}
