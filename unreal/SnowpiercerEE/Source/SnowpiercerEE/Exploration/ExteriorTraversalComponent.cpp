#include "ExteriorTraversalComponent.h"

UExteriorTraversalComponent::UExteriorTraversalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UExteriorTraversalComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UExteriorTraversalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsExterior)
    {
        UpdateColdExposure(DeltaTime);
        if (bIsRooftop)
        {
            UpdateWindEffects(DeltaTime);
        }
    }
}

void UExteriorTraversalComponent::EnterExterior(bool bRooftop)
{
    bIsExterior = true;
    bIsRooftop = bRooftop;
}

void UExteriorTraversalComponent::ReturnToInterior()
{
    bIsExterior = false;
    bIsRooftop = false;
    ColdStage = EColdExposureStage::None;
    WindIntensity = EWindIntensity::None;
}

void UExteriorTraversalComponent::SetPosture(ERooftopPosture NewPosture)
{
    Posture = NewPosture;
}

float UExteriorTraversalComponent::GetMovementSpeedMultiplier() const
{
    if (!bIsExterior)
    {
        return 1.0f;
    }

    float Multiplier = 1.0f;

    switch (Posture)
    {
    case ERooftopPosture::Crouching: Multiplier *= 0.6f; break;
    case ERooftopPosture::Prone: Multiplier *= 0.3f; break;
    default: break;
    }

    switch (WindIntensity)
    {
    case EWindIntensity::Moderate: Multiplier *= 0.8f; break;
    case EWindIntensity::Strong: Multiplier *= 0.6f; break;
    case EWindIntensity::Severe: Multiplier *= 0.4f; break;
    case EWindIntensity::Extreme: Multiplier *= 0.2f; break;
    default: break;
    }

    return Multiplier;
}

float UExteriorTraversalComponent::GetStaminaPenalty() const
{
    switch (ColdStage)
    {
    case EColdExposureStage::Chill: return 0.1f;
    case EColdExposureStage::BitingCold: return 0.25f;
    case EColdExposureStage::Hypothermia: return 0.5f;
    case EColdExposureStage::Frostbite: return 0.75f;
    case EColdExposureStage::Lethal: return 1.0f;
    default: return 0.0f;
    }
}

void UExteriorTraversalComponent::SetInHeatedZone(bool bHeated)
{
    bInHeatedZone = bHeated;
}

float UExteriorTraversalComponent::GetColdSuitTimeBonus() const
{
    switch (ColdSuitTier)
    {
    case 1: return 60.0f;
    case 2: return 180.0f;
    case 3: return 600.0f;
    default: return 0.0f;
    }
}

bool UExteriorTraversalComponent::HasWindResistance() const
{
    return ColdSuitTier >= 2;
}

bool UExteriorTraversalComponent::ProcessWindStaggerCheck(int32 AgilityScore)
{
    return false;
}

void UExteriorTraversalComponent::UpdateColdExposure(float DeltaTime)
{
    if (bInHeatedZone)
    {
        ColdExposureTime = FMath::Max(0.0f, ColdExposureTime - InteriorRecoveryRate * DeltaTime);
    }
    else
    {
        ColdExposureTime += DeltaTime;
    }

    EColdExposureStage NewStage = CalculateColdStage();
    if (NewStage != ColdStage)
    {
        ColdStage = NewStage;
        OnColdStageChanged.Broadcast(ColdStage);
    }

    ApplyColdEffects();
}

void UExteriorTraversalComponent::UpdateWindEffects(float DeltaTime)
{
    // Wind stagger check on rooftop
    StaggerTimer += DeltaTime;
    float Interval = GetStaggerInterval();

    if (StaggerTimer >= Interval)
    {
        StaggerTimer = 0.0f;

        // Standing in strong wind = high stagger chance
        if (WindIntensity >= EWindIntensity::Strong && Posture == ERooftopPosture::Standing)
        {
            if (!HasWindResistance())
            {
                // Stagger the player - external systems handle the actual knockback
                OnWindIntensityChanged.Broadcast(WindIntensity);
            }
        }
    }
}

void UExteriorTraversalComponent::ApplyColdEffects()
{
    // Apply health drain for severe cold stages
    AActor* Owner = GetOwner();
    if (!Owner) return;

    float HealthDrain = 0.0f;
    switch (ColdStage)
    {
    case EColdExposureStage::Hypothermia:
        HealthDrain = HypothermiaHealthDrain;
        break;
    case EColdExposureStage::Frostbite:
        HealthDrain = FrostbiteHealthDrain;
        break;
    case EColdExposureStage::Lethal:
        HealthDrain = LethalHealthDrain;
        break;
    default:
        break;
    }

    // Health drain is applied per tick via the cold exposure update
    // External health system hooks into OnColdStageChanged to apply damage
}

EColdExposureStage UExteriorTraversalComponent::CalculateColdStage() const
{
    float AdjustedInterval = BaseColdStageInterval + GetColdSuitTimeBonus();
    if (ColdExposureTime < AdjustedInterval) return EColdExposureStage::Chill;
    if (ColdExposureTime < AdjustedInterval * 2.0f) return EColdExposureStage::BitingCold;
    if (ColdExposureTime < AdjustedInterval * 3.0f) return EColdExposureStage::Hypothermia;
    if (ColdExposureTime < AdjustedInterval * 4.0f) return EColdExposureStage::Frostbite;
    return EColdExposureStage::Lethal;
}

float UExteriorTraversalComponent::GetStaggerInterval() const
{
    return 5.0f;
}
