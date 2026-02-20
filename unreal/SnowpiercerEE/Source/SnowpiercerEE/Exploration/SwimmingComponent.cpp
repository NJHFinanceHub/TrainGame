#include "SwimmingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USwimmingComponent::USwimmingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	CurrentBreath = MaxBreath;
}

void USwimmingComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
	CurrentBreath = GetEffectiveMaxBreath();
}

void USwimmingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (SwimState)
	{
	case ESwimState::SurfaceSwim:
		UpdateSurfaceSwim(DeltaTime);
		break;
	case ESwimState::Underwater:
		UpdateUnderwater(DeltaTime);
		break;
	default:
		break;
	}
}

void USwimmingComponent::EnterWater(float InWaterSurfaceZ)
{
	if (IsInWater() || !OwnerCharacter.IsValid())
	{
		return;
	}

	WaterSurfaceHeight = InWaterSurfaceZ;

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->SetMovementMode(MOVE_Swimming);

	SetSwimState(ESwimState::SurfaceSwim);
	SetComponentTickEnabled(true);
}

void USwimmingComponent::ExitWater()
{
	if (!IsInWater() || !OwnerCharacter.IsValid())
	{
		return;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->SetMovementMode(MOVE_Walking);

	SetSwimState(ESwimState::NotSwimming);
	SetComponentTickEnabled(false);
}

void USwimmingComponent::StartDive()
{
	if (SwimState != ESwimState::SurfaceSwim || !OwnerCharacter.IsValid())
	{
		return;
	}

	SetSwimState(ESwimState::Underwater);
}

void USwimmingComponent::Surface()
{
	if (SwimState != ESwimState::Underwater || !OwnerCharacter.IsValid())
	{
		return;
	}

	SetSwimState(ESwimState::SurfaceSwim);
}

void USwimmingComponent::SwimInput(FVector Direction)
{
	if (!IsInWater() || !OwnerCharacter.IsValid())
	{
		return;
	}

	float Speed = (SwimState == ESwimState::Underwater) ? UnderwaterSwimSpeed : SurfaceSwimSpeed;
	Speed *= SwimSpeedMultiplier;

	FVector SwimVelocity = Direction.GetSafeNormal() * Speed;

	// On surface, constrain to water plane
	if (SwimState == ESwimState::SurfaceSwim)
	{
		SwimVelocity.Z = 0.0f;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->Velocity = SwimVelocity;
}

void USwimmingComponent::SetSwimState(ESwimState NewState)
{
	if (SwimState != NewState)
	{
		SwimState = NewState;
		OnSwimStateChanged.Broadcast(SwimState);
	}
}

void USwimmingComponent::UpdateSurfaceSwim(float DeltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	// Recover breath at surface
	float EffectiveMax = GetEffectiveMaxBreath();
	if (CurrentBreath < EffectiveMax)
	{
		CurrentBreath = FMath::Min(EffectiveMax, CurrentBreath + BreathRecoveryRate * DeltaTime);
		OnBreathChanged.Broadcast(CurrentBreath / EffectiveMax);
	}

	// Buoyancy: keep player at water surface height
	FVector Location = OwnerCharacter->GetActorLocation();
	if (Location.Z < WaterSurfaceHeight)
	{
		FVector BuoyancyVel = FVector(0, 0, BuoyancyForce * DeltaTime);
		OwnerCharacter->GetCharacterMovement()->Velocity += BuoyancyVel;
	}
}

void USwimmingComponent::UpdateUnderwater(float DeltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		return;
	}

	float EffectiveMax = GetEffectiveMaxBreath();

	// Drain breath
	CurrentBreath = FMath::Max(0.0f, CurrentBreath - BreathDrainRate * DeltaTime);
	OnBreathChanged.Broadcast(CurrentBreath / EffectiveMax);

	// Drowning damage when out of breath
	if (CurrentBreath <= 0.0f)
	{
		OnDrowning.Broadcast();
	}

	// Check if player has risen above water surface → auto surface
	FVector Location = OwnerCharacter->GetActorLocation();
	if (Location.Z >= WaterSurfaceHeight)
	{
		Surface();
	}
}

float USwimmingComponent::GetEffectiveMaxBreath() const
{
	return bExtendedSwimUnlocked ? MaxBreath * 2.0f : MaxBreath;
}
