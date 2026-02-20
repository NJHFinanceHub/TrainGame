#include "ClimbingComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

UClimbingComponent::UClimbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UClimbingComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (ClimbState)
	{
	case EClimbState::Climbing:
		UpdateClimbing(DeltaTime);
		break;
	case EClimbState::Mantling:
		UpdateMantling(DeltaTime);
		break;
	default:
		break;
	}
}

bool UClimbingComponent::TryStartClimb()
{
	if (IsClimbing() || !OwnerCharacter.IsValid())
	{
		return false;
	}

	FHitResult Hit;
	EClimbableType DetectedType;
	if (!TraceForClimbable(Hit, DetectedType))
	{
		return false;
	}

	// Pipes require the skill tree unlock
	if (DetectedType == EClimbableType::Pipe && !bPipeClimbUnlocked)
	{
		return false;
	}

	CurrentSurfaceType = DetectedType;
	ClimbSurfaceNormal = Hit.ImpactNormal;
	ClimbSurfaceLocation = Hit.ImpactPoint;

	// Attach character to surface
	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->SetMovementMode(MOVE_Flying);
	Movement->StopMovementImmediately();
	Movement->GravityScale = 0.0f;

	// Snap to face the climbing surface
	FRotator FaceRotation = (-ClimbSurfaceNormal).Rotation();
	FaceRotation.Pitch = 0.0f;
	FaceRotation.Roll = 0.0f;
	OwnerCharacter->SetActorRotation(FaceRotation);

	SetClimbState(EClimbState::Climbing);
	SetComponentTickEnabled(true);

	return true;
}

void UClimbingComponent::StopClimb()
{
	if (!IsClimbing() || !OwnerCharacter.IsValid())
	{
		return;
	}

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->SetMovementMode(MOVE_Falling);
	Movement->GravityScale = 1.0f;

	CurrentSurfaceType = EClimbableType::None;
	SetClimbState(EClimbState::NotClimbing);
	SetComponentTickEnabled(false);
}

bool UClimbingComponent::TryMantle()
{
	if (IsClimbing() || !OwnerCharacter.IsValid())
	{
		return false;
	}

	FVector LedgeTop;
	float LedgeHeight;
	if (!TraceForMantleLedge(LedgeTop, LedgeHeight))
	{
		return false;
	}

	// Start mantle interpolation
	MantleStartLocation = OwnerCharacter->GetActorLocation();
	float CapsuleHalfHeight = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	MantleEndLocation = LedgeTop + FVector(0.0f, 0.0f, CapsuleHalfHeight);
	MantleTimer = 0.0f;

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->SetMovementMode(MOVE_Flying);
	Movement->StopMovementImmediately();
	Movement->GravityScale = 0.0f;

	CurrentSurfaceType = LedgeHeight <= MantleLowHeight ? EClimbableType::LedgeLow : EClimbableType::LedgeHigh;
	SetClimbState(EClimbState::Mantling);
	SetComponentTickEnabled(true);

	return true;
}

void UClimbingComponent::ClimbInput(float VerticalAxis, float HorizontalAxis)
{
	if (ClimbState != EClimbState::Climbing || !OwnerCharacter.IsValid())
	{
		return;
	}

	float Speed = GetClimbSpeedForSurface() * ClimbSpeedMultiplier;

	// Vertical movement along the surface
	FVector UpMovement = FVector::UpVector * VerticalAxis * Speed;

	// Horizontal movement along the surface (perpendicular to normal and up)
	FVector Right = FVector::CrossProduct(ClimbSurfaceNormal, FVector::UpVector).GetSafeNormal();
	FVector HorizontalMovement = Right * HorizontalAxis * Speed * 0.5f;

	UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
	Movement->Velocity = UpMovement + HorizontalMovement;
}

EClimbableType UClimbingComponent::DetectClimbable() const
{
	FHitResult Hit;
	EClimbableType Type;
	if (TraceForClimbable(Hit, Type))
	{
		return Type;
	}
	return EClimbableType::None;
}

void UClimbingComponent::SetClimbState(EClimbState NewState)
{
	if (ClimbState != NewState)
	{
		ClimbState = NewState;
		OnClimbStateChanged.Broadcast(ClimbState, CurrentSurfaceType);
	}
}

void UClimbingComponent::UpdateClimbing(float DeltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		StopClimb();
		return;
	}

	// Check if still near the surface
	FHitResult Hit;
	EClimbableType Type;
	if (!TraceForClimbable(Hit, Type))
	{
		// Reached top or bottom — dismount
		StopClimb();
		return;
	}

	// Update surface tracking
	ClimbSurfaceNormal = Hit.ImpactNormal;
	ClimbSurfaceLocation = Hit.ImpactPoint;
}

void UClimbingComponent::UpdateMantling(float DeltaTime)
{
	if (!OwnerCharacter.IsValid())
	{
		StopClimb();
		return;
	}

	MantleTimer += DeltaTime;
	float Alpha = FMath::Clamp(MantleTimer / MantleDuration, 0.0f, 1.0f);

	// Smooth step interpolation
	float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

	// Move through an arc: up first, then forward
	FVector MidPoint = FMath::Lerp(MantleStartLocation, MantleEndLocation, 0.5f);
	MidPoint.Z = MantleEndLocation.Z + 20.0f; // Arc above the target

	FVector CurrentPos;
	if (Alpha < 0.5f)
	{
		CurrentPos = FMath::Lerp(MantleStartLocation, MidPoint, SmoothAlpha * 2.0f);
	}
	else
	{
		CurrentPos = FMath::Lerp(MidPoint, MantleEndLocation, (SmoothAlpha - 0.5f) * 2.0f);
	}

	OwnerCharacter->SetActorLocation(CurrentPos);

	if (Alpha >= 1.0f)
	{
		// Mantle complete — restore normal movement
		UCharacterMovementComponent* Movement = OwnerCharacter->GetCharacterMovement();
		Movement->SetMovementMode(MOVE_Walking);
		Movement->GravityScale = 1.0f;

		CurrentSurfaceType = EClimbableType::None;
		SetClimbState(EClimbState::NotClimbing);
		SetComponentTickEnabled(false);

		OnMantleComplete.Broadcast();
	}
}

float UClimbingComponent::GetClimbSpeedForSurface() const
{
	switch (CurrentSurfaceType)
	{
	case EClimbableType::Ladder:	return LadderClimbSpeed;
	case EClimbableType::Pipe:		return PipeClimbSpeed;
	case EClimbableType::BunkBed:	return BunkClimbSpeed;
	default:						return LadderClimbSpeed;
	}
}

bool UClimbingComponent::TraceForClimbable(FHitResult& OutHit, EClimbableType& OutType) const
{
	if (!OwnerCharacter.IsValid())
	{
		return false;
	}

	FVector Start = OwnerCharacter->GetActorLocation();
	FVector Forward = OwnerCharacter->GetActorForwardVector();
	FVector End = Start + Forward * DetectionRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	// Sphere trace to detect climbable surfaces
	FCollisionShape Shape = FCollisionShape::MakeSphere(DetectionRadius);

	if (!GetWorld()->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, ECC_Visibility, Shape, Params))
	{
		return false;
	}

	AActor* HitActor = OutHit.GetActor();
	if (!HitActor)
	{
		return false;
	}

	// Determine climbable type from actor tags
	if (HitActor->ActorHasTag("Ladder"))
	{
		OutType = EClimbableType::Ladder;
		return true;
	}
	if (HitActor->ActorHasTag("Pipe"))
	{
		OutType = EClimbableType::Pipe;
		return true;
	}
	if (HitActor->ActorHasTag("BunkBed") || HitActor->ActorHasTag("Bunk"))
	{
		OutType = EClimbableType::BunkBed;
		return true;
	}

	return false;
}

bool UClimbingComponent::TraceForMantleLedge(FVector& OutLedgeTop, float& OutHeight) const
{
	if (!OwnerCharacter.IsValid())
	{
		return false;
	}

	FVector CharLocation = OwnerCharacter->GetActorLocation();
	FVector Forward = OwnerCharacter->GetActorForwardVector();

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter.Get());

	// Forward trace to find wall
	FHitResult WallHit;
	FVector WallTraceStart = CharLocation;
	FVector WallTraceEnd = CharLocation + Forward * DetectionRange;

	if (!GetWorld()->LineTraceSingleByChannel(WallHit, WallTraceStart, WallTraceEnd, ECC_Visibility, Params))
	{
		return false;
	}

	// Trace down from above to find the top of the ledge
	float MaxMantleCheck = MantleHighHeight + 20.0f;
	FVector TopTraceStart = WallHit.ImpactPoint + FVector(0, 0, MaxMantleCheck) + Forward * 10.0f;
	FVector TopTraceEnd = WallHit.ImpactPoint + Forward * 10.0f;

	FHitResult TopHit;
	if (!GetWorld()->LineTraceSingleByChannel(TopHit, TopTraceStart, TopTraceEnd, ECC_Visibility, Params))
	{
		return false;
	}

	OutLedgeTop = TopHit.ImpactPoint;
	OutHeight = OutLedgeTop.Z - CharLocation.Z + OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	// Check height is within mantle range
	if (OutHeight < 10.0f || OutHeight > MantleHighHeight)
	{
		return false;
	}

	// Check there's enough room on top to stand
	FVector StandCheck = OutLedgeTop + FVector(0, 0, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(
		OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
		OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
	);

	if (GetWorld()->OverlapBlockingTestByChannel(StandCheck, FQuat::Identity, ECC_Pawn, CapsuleShape, Params))
	{
		return false; // Not enough room on top
	}

	return true;
}
