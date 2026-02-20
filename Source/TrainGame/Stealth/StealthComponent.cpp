// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "StealthComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Combat/EnemyCharacter.h"
#include "TrainGame/Core/CombatTypes.h"
#include "GameFramework/Character.h"

UStealthComponent::UStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateMovementMode();
	UpdateComposure(DeltaTime);
	UpdateFootstepNoise(DeltaTime);
}

void UStealthComponent::GenerateNoise(float Radius, ESoundIntensity Intensity)
{
	// TODO: Apply environmental masking to radius
	// TODO: Broadcast noise event to all DetectionComponents in range
	float MaskedRadius = Radius - CalculateEnvironmentalMasking();
	if (MaskedRadius <= 0.f)
	{
		return; // Completely masked
	}

	FNoiseEvent Event;
	Event.Origin = GetOwner()->GetActorLocation();
	Event.Radius = MaskedRadius;
	Event.Intensity = Intensity;
	Event.Instigator = GetOwner();

	// TODO: Find all DetectionComponents in range and call ProcessNoiseEvent
}

float UStealthComponent::GetFootstepNoiseRadius() const
{
	// Base radius by surface type
	float BaseRadius = 0.f;
	switch (CurrentSurface)
	{
	case ESurfaceType::MetalGrating:	BaseRadius = 1200.f; break;
	case ESurfaceType::Carpet:			BaseRadius = 500.f; break;
	case ESurfaceType::WoodenPlanks:	BaseRadius = 800.f; break;
	case ESurfaceType::Water:			BaseRadius = 1000.f; break;
	case ESurfaceType::DirtGravel:		BaseRadius = 600.f; break;
	case ESurfaceType::IceFrost:		BaseRadius = 700.f; break;
	}

	// Crouch halves footstep noise
	if (CurrentMovement == EStealthMovement::CrouchWalk)
	{
		BaseRadius *= 0.5f;
	}

	return BaseRadius;
}

bool UStealthComponent::EnterHidingSpot(AActor* HidingSpot)
{
	// TODO: Validate hiding spot is available (not occupied by body)
	// TODO: Play enter animation (HidingTransitionTime)
	// TODO: Set player to hidden state

	if (!HidingSpot || bIsHiding)
	{
		return false;
	}

	bIsHiding = true;
	CurrentHidingSpot = HidingSpot;
	Composure = 100.f;
	OnEnteredHidingSpot.Broadcast();
	return true;
}

void UStealthComponent::ExitHidingSpot()
{
	if (!bIsHiding)
	{
		return;
	}

	// TODO: Play exit animation (HidingTransitionTime)
	bIsHiding = false;
	CurrentHidingSpot = nullptr;
	OnExitedHidingSpot.Broadcast();
}

FTakedownResult UStealthComponent::AttemptTakedown(AActor* Target, bool bLethal)
{
	FTakedownResult Result;

	if (!Target || !CanPerformTakedown(Target))
	{
		return Result;
	}

	AActor* Owner = GetOwner();
	if (!Owner) return Result;

	// Check distance
	float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	if (Dist > TakedownRange)
	{
		return Result;
	}

	// Check if behind target (dot product of target's forward and direction to attacker)
	FVector ToAttacker = (Owner->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(Target->GetActorForwardVector(), ToAttacker);
	if (DotProduct > -0.3f) // Must be approaching from behind (negative dot = behind)
	{
		return Result;
	}

	// Check if target is an EnemyCharacter with special takedown requirements
	AEnemyCharacter* EnemyTarget = Cast<AEnemyCharacter>(Target);
	if (EnemyTarget)
	{
		if (!EnemyTarget->CanBeTakenDown())
		{
			return Result;
		}

		// Apply the takedown via the enemy's own system
		EnemyTarget->ReceiveTakedown(bLethal, Owner);
	}
	else
	{
		// Generic target — apply damage via CombatComponent
		UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
		if (TargetCombat)
		{
			if (bLethal)
			{
				TargetCombat->ReceiveAttack(9999.f, EAttackDirection::Mid, EDamageType::StealthTakedown, Owner);
			}
			else
			{
				TargetCombat->ReceiveAttack(0.f, EAttackDirection::Mid, EDamageType::NonLethal, Owner);
			}
		}
	}

	Result.bSuccess = true;
	Result.bLethal = bLethal;
	Result.Method = ENonLethalMethod::StealthTakedown;

	// Lethal takedowns are slightly louder
	float NoiseRadius = bLethal ? TakedownNoiseRadius * 1.5f : TakedownNoiseRadius;
	Result.NoiseGenerated = NoiseRadius;

	if (bLethal)
	{
		KillCount++;
	}
	else
	{
		NonLethalTakedownCount++;
	}

	GenerateNoise(NoiseRadius, bLethal ? ESoundIntensity::Medium : ESoundIntensity::Low);
	OnTakedownCompleted.Broadcast(Result);
	return Result;
}

bool UStealthComponent::CanPerformTakedown(const AActor* Target) const
{
	if (!Target || bIsHiding || bIsDraggingBody) return false;

	const AActor* Owner = GetOwner();
	if (!Owner) return false;

	// Check distance
	float Dist = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	if (Dist > TakedownRange) return false;

	// Check if behind target
	FVector ToAttacker = (Owner->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	float DotProduct = FVector::DotProduct(Target->GetActorForwardVector(), ToAttacker);
	if (DotProduct > -0.3f) return false;

	// Check EnemyCharacter-specific requirements
	const AEnemyCharacter* EnemyTarget = Cast<const AEnemyCharacter>(Target);
	if (EnemyTarget)
	{
		if (!EnemyTarget->CanBeTakenDown()) return false;
		if (EnemyTarget->IsAlerted()) return false;
	}

	// Check target is alive
	UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
	if (TargetCombat && !TargetCombat->IsAlive()) return false;

	return true;
}

bool UStealthComponent::StartDragBody(AActor* Body)
{
	if (!Body || bIsDraggingBody || bIsHiding)
	{
		return false;
	}

	// TODO: Validate body is unconscious/dead
	// TODO: Attach body to player, reduce movement speed
	bIsDraggingBody = true;
	DraggedBody = Body;
	return true;
}

void UStealthComponent::StopDragBody()
{
	if (!bIsDraggingBody)
	{
		return;
	}

	// TODO: Detach body, restore movement speed
	bIsDraggingBody = false;
	DraggedBody = nullptr;
}

bool UStealthComponent::PlaceBodyInHidingSpot(AActor* Body, AActor* HidingSpot)
{
	// TODO: Validate hiding spot is not occupied
	// TODO: Place body in spot, mark spot as occupied
	// TODO: Stop dragging if this is the dragged body
	return false;
}

bool UStealthComponent::RestrainTarget(AActor* Target)
{
	// TODO: Check if target is unconscious
	// TODO: Check if player has restraint items in inventory
	// TODO: Apply restraint (prevents waking + alerting)
	return false;
}

float UStealthComponent::GetCurrentVisibility() const
{
	if (bIsHiding)
	{
		return 0.f;
	}

	// TODO: Calculate based on lighting, movement speed, disguise
	float Visibility = 1.f;

	switch (CurrentMovement)
	{
	case EStealthMovement::Stationary:	Visibility *= 0.3f; break;
	case EStealthMovement::CrouchWalk:	Visibility *= 0.6f; break;
	case EStealthMovement::Walk:		Visibility *= 1.0f; break;
	case EStealthMovement::Run:			Visibility *= 1.5f; break;
	case EStealthMovement::Sprint:		Visibility *= 2.0f; break;
	}

	return Visibility;
}

void UStealthComponent::UpdateMovementMode()
{
	// TODO: Query character movement component for speed
	// TODO: Map to EStealthMovement enum
}

void UStealthComponent::UpdateComposure(float DeltaTime)
{
	if (!bIsHiding)
	{
		Composure = 100.f;
		return;
	}

	// TODO: Check for NPCs within ComposureDrainDistance
	// TODO: Drain composure if NPCs are near
	// TODO: At 0%, generate involuntary noise
	// TODO: Broadcast composure changes
}

void UStealthComponent::UpdateFootstepNoise(float DeltaTime)
{
	if (CurrentMovement == EStealthMovement::Stationary || bIsHiding)
	{
		return;
	}

	// TODO: Generate periodic footstep noise based on movement speed
	// TODO: Query surface type from physics material
}

float UStealthComponent::CalculateEnvironmentalMasking() const
{
	// TODO: Query nearby environmental sound sources
	// Engine room = -1000cm, kitchen = -400cm, etc.
	// Also check train motion state (moving = -200cm global)
	return 0.f;
}
