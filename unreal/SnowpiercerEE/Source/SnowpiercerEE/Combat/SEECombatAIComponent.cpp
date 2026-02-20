#include "SEECombatAIComponent.h"
#include "SEECombatComponent.h"

USEECombatAIComponent::USEECombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USEECombatAIComponent::BeginPlay()
{
    Super::BeginPlay();

    CombatComponent = GetOwner()->FindComponentByClass<USEECombatComponent>();
}

void USEECombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!CurrentTarget || !CombatComponent || CombatComponent->bIsDead)
    {
        return;
    }

    UpdateState(DeltaTime);
}

void USEECombatAIComponent::SetTarget(AActor* Target)
{
    CurrentTarget = Target;
    if (Target)
    {
        SetState(ESEECombatAIState::Approaching);
    }
    else
    {
        SetState(ESEECombatAIState::Idle);
    }
}

void USEECombatAIComponent::ClearTarget()
{
    CurrentTarget = nullptr;
    SetState(ESEECombatAIState::Idle);
}

void USEECombatAIComponent::NotifyDamageReceived(float DamageAmount)
{
    if (DamageAmount >= StaggerDamageThreshold)
    {
        StaggerTimeRemaining = StaggerDuration;
        SetState(ESEECombatAIState::Staggered);
    }
}

FVector USEECombatAIComponent::GetDesiredMovementDirection() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return FVector::ZeroVector;
    }

    FVector ToTarget = CurrentTarget->GetActorLocation() - GetOwner()->GetActorLocation();
    float Distance = ToTarget.Size();

    if (Distance < KINDA_SMALL_NUMBER)
    {
        return FVector::ZeroVector;
    }

    FVector Direction = ToTarget / Distance;

    switch (CurrentState)
    {
    case ESEECombatAIState::Approaching:
        return Direction;
    case ESEECombatAIState::Retreating:
        return -Direction;
    case ESEECombatAIState::Flanking:
        return FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    default:
        // Move toward preferred range
        if (Distance > PreferredRange * 1.2f) return Direction;
        if (Distance < PreferredRange * 0.5f) return -Direction;
        return FVector::ZeroVector;
    }
}

bool USEECombatAIComponent::WantsToAttack() const
{
    return bWantsToAttack;
}

ESEEAttackDirection USEECombatAIComponent::GetChosenAttackDirection() const
{
    return ChosenDirection;
}

float USEECombatAIComponent::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
    {
        return MAX_FLT;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

void USEECombatAIComponent::UpdateState(float DeltaTime)
{
    bWantsToAttack = false;

    // Handle stagger
    if (CurrentState == ESEECombatAIState::Staggered)
    {
        StaggerTimeRemaining -= DeltaTime;
        if (StaggerTimeRemaining <= 0.0f)
        {
            SetState(ESEECombatAIState::Approaching);
        }
        return;
    }

    // Check retreat threshold
    if (CombatComponent && CombatComponent->GetHealthPercent() <= RetreatHealthThreshold
        && Behavior != ESEECombatAIBehavior::Berserker)
    {
        SetState(ESEECombatAIState::Retreating);
        return;
    }

    float Distance = GetDistanceToTarget();

    // Attack cooldown
    AttackCooldownRemaining = FMath::Max(0.0f, AttackCooldownRemaining - DeltaTime);

    if (Distance <= PreferredRange)
    {
        if (AttackCooldownRemaining <= 0.0f)
        {
            // Choose attack direction
            int32 Rand = FMath::RandRange(0, 2);
            ChosenDirection = static_cast<ESEEAttackDirection>(Rand);

            bWantsToAttack = true;
            AttackCooldownRemaining = AttackCooldown;
            SetState(ESEECombatAIState::Attacking);
        }
        else
        {
            // Between attacks: block or idle based on behavior
            if (Behavior == ESEECombatAIBehavior::Defensive || Behavior == ESEECombatAIBehavior::Balanced)
            {
                SetState(ESEECombatAIState::Blocking);
            }
        }
    }
    else
    {
        SetState(ESEECombatAIState::Approaching);
    }
}

void USEECombatAIComponent::SetState(ESEECombatAIState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        OnStateChanged.Broadcast(NewState);
    }
}
