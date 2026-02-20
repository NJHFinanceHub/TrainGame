#include "SEENPCCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

ASEENPCCharacter::ASEENPCCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

	bUseControllerRotationYaw = false;
	CurrentHealth = MaxHealth;
}

void ASEENPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASEENPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentState)
	{
	case ESEENPCState::Patrolling:
		UpdatePatrol(DeltaTime);
		UpdateDetection(DeltaTime);
		break;
	case ESEENPCState::Idle:
		UpdateDetection(DeltaTime);
		break;
	case ESEENPCState::Suspicious:
		UpdateDetection(DeltaTime);
		break;
	case ESEENPCState::Combat:
		UpdateCombat(DeltaTime);
		break;
	default:
		break;
	}
}

void ASEENPCCharacter::SetState(ESEENPCState NewState)
{
	if (CurrentState == NewState) return;
	if (CurrentState == ESEENPCState::Dead) return;

	CurrentState = NewState;

	switch (NewState)
	{
	case ESEENPCState::Patrolling:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;
	case ESEENPCState::Suspicious:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * 0.5f;
		break;
	case ESEENPCState::Alerted:
	case ESEENPCState::Combat:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;
	case ESEENPCState::Fleeing:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed * 1.2f;
		break;
	default:
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;
	}
}

FSEENPCDialogueLine ASEENPCCharacter::GetRandomAmbientLine() const
{
	if (AmbientLines.Num() == 0)
	{
		return FSEENPCDialogueLine();
	}
	return AmbientLines[FMath::RandRange(0, AmbientLines.Num() - 1)];
}

void ASEENPCCharacter::AddPatrolPoint(FVector Point)
{
	PatrolPoints.Add(Point);
}

void ASEENPCCharacter::StartPatrol()
{
	if (PatrolPoints.Num() >= 2)
	{
		CurrentPatrolIndex = 0;
		bPatrolForward = true;
		SetState(ESEENPCState::Patrolling);
	}
}

void ASEENPCCharacter::StopPatrol()
{
	SetState(ESEENPCState::Idle);
}

bool ASEENPCCharacter::CanSeeActor(AActor* Target) const
{
	if (!Target) return false;

	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	float Distance = ToTarget.Size();

	if (Distance > SightRange) return false;

	ToTarget.Normalize();
	float DotProduct = FVector::DotProduct(GetActorForwardVector(), ToTarget);
	float AngleToTarget = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	if (AngleToTarget > SightAngle) return false;

	// Line trace for line of sight
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, GetActorLocation() + FVector(0, 0, 64),
		Target->GetActorLocation() + FVector(0, 0, 64), ECC_Visibility, Params))
	{
		return Hit.GetActor() == Target;
	}

	return true;
}

void ASEENPCCharacter::UpdatePatrol(float DeltaTime)
{
	if (PatrolPoints.Num() < 2) return;

	FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];
	FVector CurrentLocation = GetActorLocation();
	float DistToTarget = FVector::Dist2D(CurrentLocation, TargetPoint);

	if (DistToTarget < 100.0f)
	{
		PatrolWaitTimer += DeltaTime;
		if (PatrolWaitTimer >= PatrolWaitTime)
		{
			PatrolWaitTimer = 0.0f;
			if (bPatrolForward)
			{
				CurrentPatrolIndex++;
				if (CurrentPatrolIndex >= PatrolPoints.Num())
				{
					CurrentPatrolIndex = PatrolPoints.Num() - 2;
					bPatrolForward = false;
				}
			}
			else
			{
				CurrentPatrolIndex--;
				if (CurrentPatrolIndex < 0)
				{
					CurrentPatrolIndex = 1;
					bPatrolForward = true;
				}
			}
		}
	}
	else
	{
		FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal2D();
		AddMovementInput(Direction, 1.0f);
	}
}

void ASEENPCCharacter::UpdateDetection(float DeltaTime)
{
	// Find player
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return;

	bool bCanSee = CanSeeActor(PlayerPawn);
	float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
	bool bCanHear = DistToPlayer <= HearingRange;

	if (bCanSee)
	{
		float DistanceFactor = 1.0f - FMath::Clamp(DistToPlayer / SightRange, 0.0f, 1.0f);
		DetectionLevel += DetectionRate * DistanceFactor * DeltaTime;
	}
	else if (bCanHear)
	{
		float HearFactor = 1.0f - FMath::Clamp(DistToPlayer / HearingRange, 0.0f, 1.0f);
		DetectionLevel += DetectionRate * HearFactor * 0.5f * DeltaTime;
	}
	else
	{
		DetectionLevel = FMath::Max(0.0f, DetectionLevel - DetectionRate * 0.3f * DeltaTime);
	}

	DetectionLevel = FMath::Clamp(DetectionLevel, 0.0f, 1.0f);

	if (DetectionLevel >= 1.0f && CurrentState != ESEENPCState::Combat)
	{
		SetState(ESEENPCState::Combat);
	}
	else if (DetectionLevel >= 0.5f && CurrentState == ESEENPCState::Idle)
	{
		SetState(ESEENPCState::Suspicious);
	}
	else if (DetectionLevel < 0.3f && CurrentState == ESEENPCState::Suspicious)
	{
		SetState(ESEENPCState::Idle);
	}
}

void ASEENPCCharacter::UpdateCombat(float DeltaTime)
{
	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!PlayerPawn) return;

	float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

	if (DistToPlayer > AttackRange)
	{
		FVector Direction = (PlayerPawn->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		AddMovementInput(Direction, 1.0f);
	}

	AttackTimer += DeltaTime;
}
