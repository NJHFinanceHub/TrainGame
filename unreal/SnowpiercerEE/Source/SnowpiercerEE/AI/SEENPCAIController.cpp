// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCAIController.h"

#include "SEENPCHealthComponent.h"
#include "SnowpiercerEE/SEECombatComponent.h"
#include "SnowpiercerEE/SEEJackbootCharacter.h"
#include "SnowpiercerEE/SEECivilianCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "TimerManager.h"

namespace
{
	constexpr float GPerceptionInterval = 0.25f;
	constexpr float GPathRetryInterval = 1.0f;
	constexpr float GStuckSpeedThreshold = 25.0f;
	constexpr float GStuckTimeBeforeFallback = 0.8f;
	constexpr float GDirectMoveBurst = 2.0f;
	constexpr float GCorpseLifetime = 20.0f;
}

ASEENPCAIController::ASEENPCAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASEENPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!InPawn) return;

	HomeLocation = InPawn->GetActorLocation();

	// Pawn-class defaults for C++-derived NPCs that auto-possess via
	// AIControllerClass (the brain subsystem overrides these for BP pawns).
	if (!bConfigured)
	{
		if (InPawn->IsA<ASEEJackbootCharacter>())
		{
			bHostile = true;
			MaxHealth = 150.0f;
			AttackDamage = 20.0f;
			if (DialogueEntryNode.IsNone())
			{
				DialogueEntryNode = TEXT("Guard_01");
			}
		}
		else if (InPawn->IsA<ASEECivilianCharacter>())
		{
			bHostile = false;
			MaxHealth = 60.0f;
			if (DialogueEntryNode.IsNone())
			{
				DialogueEntryNode = TEXT("Injured_01");
			}
		}
	}

	ApplyConfiguration();

	// Smooth turn-in-place toward the controller's focus instead of snapping.
	if (ACharacter* PawnChar = Cast<ACharacter>(InPawn))
	{
		PawnChar->bUseControllerRotationYaw = false;
		if (UCharacterMovementComponent* Move = PawnChar->GetCharacterMovement())
		{
			Move->bUseControllerDesiredRotation = true;
			Move->bOrientRotationToMovement = false;
			Move->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
		}
	}

	GetWorldTimerManager().SetTimer(PerceptionTimerHandle, this,
		&ASEENPCAIController::UpdatePerception, GPerceptionInterval, true,
		FMath::FRandRange(0.0f, GPerceptionInterval));

	EnterIdle();
}

void ASEENPCAIController::OnUnPossess()
{
	GetWorldTimerManager().ClearTimer(PerceptionTimerHandle);
	GetWorldTimerManager().ClearTimer(WanderTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);

	if (PawnHealth)
	{
		PawnHealth->OnDamageTaken.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDamaged);
		PawnHealth->OnDowned.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDeath);
		PawnHealth->OnDeath.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDeath);
		PawnHealth = nullptr;
	}

	Super::OnUnPossess();
}

void ASEENPCAIController::ApplyConfiguration()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Attach a health component if the pawn has none (the placed BP_NPC_*
	// blueprints derive from plain ACharacter and carry no components).
	USEEHealthComponent* Health = ControlledPawn->FindComponentByClass<USEEHealthComponent>();
	if (!Health)
	{
		USEENPCHealthComponent* NewHealth = NewObject<USEENPCHealthComponent>(
			ControlledPawn, USEENPCHealthComponent::StaticClass(), TEXT("SEENPCHealth"));
		NewHealth->InitHealth(MaxHealth);
		NewHealth->RegisterComponent();
		Health = NewHealth;
	}
	else if (USEENPCHealthComponent* NPCHealth = Cast<USEENPCHealthComponent>(Health))
	{
		// Re-applied configuration (subsystem pass after possession) — keep max in sync.
		if (!NPCHealth->IsDead() && NPCHealth->GetMaxHealth() != MaxHealth)
		{
			NPCHealth->InitHealth(MaxHealth);
		}
	}

	if (PawnHealth != Health)
	{
		if (PawnHealth)
		{
			PawnHealth->OnDamageTaken.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDamaged);
			PawnHealth->OnDowned.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDeath);
			PawnHealth->OnDeath.RemoveDynamic(this, &ASEENPCAIController::HandlePawnDeath);
		}
		PawnHealth = Health;
		PawnHealth->OnDamageTaken.AddDynamic(this, &ASEENPCAIController::HandlePawnDamaged);
		// Zero health downs the component before it kills it — for NPCs both mean dead.
		PawnHealth->OnDowned.AddDynamic(this, &ASEENPCAIController::HandlePawnDeath);
		PawnHealth->OnDeath.AddDynamic(this, &ASEENPCAIController::HandlePawnDeath);
	}

	SetMoveSpeed(WalkSpeed);
}

void ASEENPCAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BrainState == ESEENPCBrainState::Dead || !GetPawn()) return;

	if (PathRetryCooldown > 0.0f)
	{
		PathRetryCooldown -= DeltaTime;
	}

	switch (BrainState)
	{
	case ESEENPCBrainState::Chase:
		UpdateChase(DeltaTime);
		break;

	case ESEENPCBrainState::ReturnHome:
		UpdateReturnHome();
		UpdateDirectMove(DeltaTime);
		break;

	default:
		// Idle/Wander movement is timer-driven; only the direct-move fallback ticks.
		UpdateDirectMove(DeltaTime);
		break;
	}
}

// ---------------------------------------------------------------------------
// Perception
// ---------------------------------------------------------------------------

void ASEENPCAIController::UpdatePerception()
{
	if (BrainState == ESEENPCBrainState::Dead) return;

	APawn* ControlledPawn = GetPawn();
	APawn* Player = GetPlayerPawn();
	if (!ControlledPawn || !Player) return;

	const bool bPlayerAlive = IsPlayerAlive(Player);

	if (bHostile)
	{
		if (BrainState == ESEENPCBrainState::Chase)
		{
			if (!bPlayerAlive)
			{
				StartReturnHome();
				return;
			}

			// Already aggroed: track by sight without the frontal-cone restriction.
			if (CanSeePlayer(Player, /*bUseCone=*/false))
			{
				TimeSinceSeenPlayer = 0.0f;
			}
			else
			{
				TimeSinceSeenPlayer += GPerceptionInterval;
				if (TimeSinceSeenPlayer >= LoseSightDuration)
				{
					StartReturnHome();
				}
			}
		}
		else if (bPlayerAlive && CanSeePlayer(Player, /*bUseCone=*/true))
		{
			StartChase();
		}
	}
	else
	{
		// Friendlies: face the player when nearby (and always while talking).
		const float DistSq = FVector::DistSquared(
			ControlledPawn->GetActorLocation(), Player->GetActorLocation());
		const bool bPlayerClose = DistSq <= FMath::Square(FacePlayerRange);

		if (bInDialogue || bPlayerClose)
		{
			SetFocus(Player);
			if (bInDialogue)
			{
				StopMovement();
				bDirectMoveActive = false;
			}
		}
		else
		{
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}

bool ASEENPCAIController::CanSeePlayer(const APawn* Player, bool bUseCone) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !Player) return false;

	const FVector PawnLoc = ControlledPawn->GetActorLocation();
	const FVector PlayerLoc = Player->GetActorLocation();
	const FVector ToPlayer = PlayerLoc - PawnLoc;

	if (ToPlayer.SizeSquared() > FMath::Square(SightRange)) return false;

	if (bUseCone)
	{
		const float CosHalfCone = FMath::Cos(FMath::DegreesToRadians(SightConeDegrees * 0.5f));
		const float FacingDot = FVector::DotProduct(
			ControlledPawn->GetActorForwardVector().GetSafeNormal2D(),
			ToPlayer.GetSafeNormal2D());
		if (FacingDot < CosHalfCone) return false;
	}

	// Occlusion: eye-height line trace. The player's capsule ignores the
	// visibility channel, so any blocking hit that is not the player means a
	// wall/prop sits in between.
	FVector EyeLoc;
	FRotator EyeRot;
	ControlledPawn->GetActorEyesViewPoint(EyeLoc, EyeRot);

	FCollisionQueryParams Params(FName(TEXT("SEENPCSight")), false, ControlledPawn);
	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, EyeLoc, PlayerLoc + FVector(0.0f, 0.0f, 40.0f), ECC_Visibility, Params);

	return !bHit || Hit.GetActor() == Player;
}

// ---------------------------------------------------------------------------
// State transitions
// ---------------------------------------------------------------------------

void ASEENPCAIController::StartChase()
{
	if (BrainState == ESEENPCBrainState::Chase || BrainState == ESEENPCBrainState::Dead) return;

	BrainState = ESEENPCBrainState::Chase;
	TimeSinceSeenPlayer = 0.0f;
	StuckTime = 0.0f;
	PathRetryCooldown = 0.0f;
	bDirectMoveActive = false;
	bInDialogue = false;

	GetWorldTimerManager().ClearTimer(WanderTimerHandle);
	SetMoveSpeed(ChaseSpeed);

	if (APawn* Player = GetPlayerPawn())
	{
		SetFocus(Player);
	}
}

void ASEENPCAIController::StartReturnHome()
{
	if (BrainState == ESEENPCBrainState::Dead) return;

	BrainState = ESEENPCBrainState::ReturnHome;
	bWindingUp = false;
	bDirectMoveActive = false;
	StuckTime = 0.0f;
	PathRetryCooldown = 0.0f;

	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	ClearFocus(EAIFocusPriority::Gameplay);
	SetMoveSpeed(WalkSpeed);
	RequestMove(HomeLocation, nullptr, 80.0f);
}

void ASEENPCAIController::EnterIdle()
{
	if (BrainState == ESEENPCBrainState::Dead) return;

	BrainState = ESEENPCBrainState::Idle;
	bDirectMoveActive = false;
	StopMovement();
	SetMoveSpeed(WalkSpeed);

	// Only friendlies pace around; hostiles hold their post.
	if (!bHostile)
	{
		ScheduleNextWander();
	}
}

// ---------------------------------------------------------------------------
// Movement
// ---------------------------------------------------------------------------

void ASEENPCAIController::UpdateChase(float DeltaTime)
{
	ACharacter* PawnChar = GetPawnCharacter();
	APawn* Player = GetPlayerPawn();
	if (!PawnChar || !Player) return;

	const float Dist = FVector::Dist2D(PawnChar->GetActorLocation(), Player->GetActorLocation());

	SetFocus(Player);

	// In range: stop and swing on cooldown (windup gives the player a dodge window).
	if (Dist <= AttackRange)
	{
		StopMovement();
		bDirectMoveActive = false;
		StuckTime = 0.0f;

		if (!bWindingUp && GetWorld()->GetTimeSeconds() >= NextAttackTime)
		{
			BeginAttackWindup();
		}
		return;
	}

	if (bWindingUp) return; // committed to the swing, no repositioning

	// Direct-movement fallback drives the pawn straight at the player.
	if (bDirectMoveActive)
	{
		DirectMoveGoal = Player->GetActorLocation();
		UpdateDirectMove(DeltaTime);
		return;
	}

	// (Re)issue a pathed move periodically.
	if (PathRetryCooldown <= 0.0f)
	{
		PathRetryCooldown = GPathRetryInterval;
		RequestMove(Player->GetActorLocation(), Player, ChaseAcceptanceRadius);
	}

	// Stall detection: pathing reported success but the pawn is not moving
	// (missing/partial navmesh). Burst into direct movement.
	if (PawnChar->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		StuckTime += DeltaTime;
		if (StuckTime >= GStuckTimeBeforeFallback)
		{
			StuckTime = 0.0f;
			StopMovement();
			bDirectMoveActive = true;
			DirectMoveGoal = Player->GetActorLocation();
			DirectMoveTimeLeft = GDirectMoveBurst;
		}
	}
	else
	{
		StuckTime = 0.0f;
	}
}

void ASEENPCAIController::UpdateReturnHome()
{
	ACharacter* PawnChar = GetPawnCharacter();
	if (!PawnChar) return;

	const float Dist = FVector::Dist2D(PawnChar->GetActorLocation(), HomeLocation);
	if (Dist <= 100.0f)
	{
		EnterIdle();
		return;
	}

	if (bDirectMoveActive) return; // UpdateDirectMove steers

	if (PathRetryCooldown <= 0.0f)
	{
		PathRetryCooldown = GPathRetryInterval;
		RequestMove(HomeLocation, nullptr, 80.0f);
	}

	if (PawnChar->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		StuckTime += GetWorld()->GetDeltaSeconds();
		if (StuckTime >= GStuckTimeBeforeFallback)
		{
			StuckTime = 0.0f;
			StopMovement();
			bDirectMoveActive = true;
			DirectMoveGoal = HomeLocation;
			DirectMoveTimeLeft = GDirectMoveBurst;
		}
	}
	else
	{
		StuckTime = 0.0f;
	}
}

void ASEENPCAIController::UpdateDirectMove(float DeltaTime)
{
	if (!bDirectMoveActive) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		bDirectMoveActive = false;
		return;
	}

	DirectMoveTimeLeft -= DeltaTime;

	const FVector ToGoal = DirectMoveGoal - ControlledPawn->GetActorLocation();
	const float StopDist = (BrainState == ESEENPCBrainState::Chase) ? AttackRange * 0.8f : 70.0f;

	if (DirectMoveTimeLeft <= 0.0f || ToGoal.Size2D() <= StopDist)
	{
		bDirectMoveActive = false;
		PathRetryCooldown = 0.0f; // try pathfinding again right away
		return;
	}

	ControlledPawn->AddMovementInput(ToGoal.GetSafeNormal2D(), 1.0f);
	if (BrainState != ESEENPCBrainState::Chase)
	{
		SetFocalPoint(DirectMoveGoal);
	}
}

void ASEENPCAIController::RequestMove(const FVector& GoalLocation, AActor* GoalActor, float AcceptanceRadius)
{
	EPathFollowingRequestResult::Type Result;
	if (GoalActor)
	{
		Result = MoveToActor(GoalActor, AcceptanceRadius, /*bStopOnOverlap=*/true,
			/*bUsePathfinding=*/true, /*bCanStrafe=*/true);
	}
	else
	{
		Result = MoveToLocation(GoalLocation, AcceptanceRadius, /*bStopOnOverlap=*/true,
			/*bUsePathfinding=*/true, /*bProjectDestinationToNavigation=*/true, /*bCanStrafe=*/true);
	}

	// No navmesh (or unreachable goal): straight-line corridor fallback.
	if (Result == EPathFollowingRequestResult::Failed)
	{
		bDirectMoveActive = true;
		DirectMoveGoal = GoalActor ? GoalActor->GetActorLocation() : GoalLocation;
		DirectMoveTimeLeft = GDirectMoveBurst;
	}
}

void ASEENPCAIController::SetMoveSpeed(float Speed) const
{
	if (const ACharacter* PawnChar = GetPawnCharacter())
	{
		if (UCharacterMovementComponent* Move = PawnChar->GetCharacterMovement())
		{
			Move->MaxWalkSpeed = Speed;
		}
	}
}

// ---------------------------------------------------------------------------
// Attacks
// ---------------------------------------------------------------------------

void ASEENPCAIController::BeginAttackWindup()
{
	bWindingUp = true;
	GetWorldTimerManager().SetTimer(AttackWindupTimerHandle, this,
		&ASEENPCAIController::DeliverAttack, AttackWindupTime, false);
}

void ASEENPCAIController::DeliverAttack()
{
	bWindingUp = false;
	NextAttackTime = GetWorld()->GetTimeSeconds() + FMath::FRandRange(AttackIntervalMin, AttackIntervalMax);

	if (BrainState != ESEENPCBrainState::Chase) return;

	APawn* ControlledPawn = GetPawn();
	APawn* Player = GetPlayerPawn();
	if (!ControlledPawn || !Player || !IsPlayerAlive(Player)) return;

	// The player can step out during the windup — generous lunge reach, but a miss is a miss.
	const float Dist = FVector::Dist2D(ControlledPawn->GetActorLocation(), Player->GetActorLocation());
	if (Dist > AttackRange * 1.3f) return;
	if (!CanSeePlayer(Player, /*bUseCone=*/false)) return;

	// Mirror the player's own melee pipeline in reverse: route through the
	// victim's combat component (block/parry/dodge i-frames), then health.
	float DamageThrough = AttackDamage;
	if (USEECombatComponent* PlayerCombat = Player->FindComponentByClass<USEECombatComponent>())
	{
		DamageThrough = PlayerCombat->ProcessIncomingHit(AttackDamage, ControlledPawn, /*bBreaksBlock=*/false);
	}

	if (DamageThrough > 0.0f)
	{
		if (USEEHealthComponent* PlayerHealth = Player->FindComponentByClass<USEEHealthComponent>())
		{
			PlayerHealth->TakeDamage(DamageThrough, ESEEDamageType::Blunt, ControlledPawn);
		}

		if (ACharacter* PlayerChar = Cast<ACharacter>(Player))
		{
			const FVector Away = (Player->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal2D();
			PlayerChar->LaunchCharacter(Away * HitKnockbackImpulse + FVector(0.0f, 0.0f, 40.0f), false, false);
		}
	}
}

// ---------------------------------------------------------------------------
// Friendly wander
// ---------------------------------------------------------------------------

void ASEENPCAIController::ScheduleNextWander()
{
	if (WanderRadius <= 0.0f) return; // stationary NPC (merchant)

	GetWorldTimerManager().SetTimer(WanderTimerHandle, this,
		&ASEENPCAIController::OnWanderTimer,
		FMath::FRandRange(WanderIntervalMin, WanderIntervalMax), false);
}

void ASEENPCAIController::OnWanderTimer()
{
	ScheduleNextWander();

	if (BrainState != ESEENPCBrainState::Idle && BrainState != ESEENPCBrainState::Wander) return;
	if (bInDialogue) return;

	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	// Don't pace around while the player is standing next to us.
	if (APawn* Player = GetPlayerPawn())
	{
		if (FVector::DistSquared(ControlledPawn->GetActorLocation(), Player->GetActorLocation())
			<= FMath::Square(FacePlayerRange))
		{
			return;
		}
	}

	BrainState = ESEENPCBrainState::Wander;

	const FVector2D Offset = FMath::RandPointInCircle(WanderRadius);
	const FVector Goal = HomeLocation + FVector(Offset.X, Offset.Y, 0.0f);
	RequestMove(Goal, nullptr, 60.0f);
}

// ---------------------------------------------------------------------------
// Damage / death
// ---------------------------------------------------------------------------

void ASEENPCAIController::HandlePawnDamaged(float Damage, ESEEDamageType DamageType, AActor* DamageInstigator)
{
	if (BrainState == ESEENPCBrainState::Dead) return;

	// Taking damage is instant aggro for hostiles, regardless of facing.
	if (bHostile)
	{
		StartChase();
	}
}

void ASEENPCAIController::HandlePawnDeath()
{
	if (BrainState == ESEENPCBrainState::Dead) return;
	BrainState = ESEENPCBrainState::Dead;

	GetWorldTimerManager().ClearTimer(PerceptionTimerHandle);
	GetWorldTimerManager().ClearTimer(WanderTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackWindupTimerHandle);
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);

	ACharacter* PawnChar = GetPawnCharacter();
	if (PawnChar)
	{
		PawnChar->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (UCharacterMovementComponent* Move = PawnChar->GetCharacterMovement())
		{
			Move->StopMovementImmediately();
			Move->DisableMovement();
		}

		USkeletalMeshComponent* Mesh = PawnChar->GetMesh();
		if (Mesh && Mesh->GetSkeletalMeshAsset())
		{
			Mesh->SetCollisionProfileName(TEXT("Ragdoll"));
			Mesh->SetSimulatePhysics(true);
		}
		else
		{
			// No visible mesh to ragdoll — collapse the capsule out of sight.
			PawnChar->SetActorHiddenInGame(true);
		}

		PawnChar->SetLifeSpan(GCorpseLifetime);
	}

	UnPossess();
	SetLifeSpan(GCorpseLifetime + 1.0f);
}

// ---------------------------------------------------------------------------
// Dialogue hooks
// ---------------------------------------------------------------------------

bool ASEENPCAIController::CanStartDialogue() const
{
	if (DialogueEntryNode.IsNone()) return false;
	if (BrainState == ESEENPCBrainState::Dead || BrainState == ESEENPCBrainState::Chase) return false;
	if (bWindingUp) return false;
	return true;
}

void ASEENPCAIController::SetInDialogue(bool bNowInDialogue)
{
	if (BrainState == ESEENPCBrainState::Dead) return;

	bInDialogue = bNowInDialogue;

	if (bInDialogue)
	{
		StopMovement();
		bDirectMoveActive = false;
		if (APawn* Player = GetPlayerPawn())
		{
			SetFocus(Player);
		}
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
		if (BrainState == ESEENPCBrainState::Wander)
		{
			BrainState = ESEENPCBrainState::Idle;
		}
	}
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

ACharacter* ASEENPCAIController::GetPawnCharacter() const
{
	return Cast<ACharacter>(GetPawn());
}

APawn* ASEENPCAIController::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

bool ASEENPCAIController::IsPlayerAlive(const APawn* Player) const
{
	if (!Player) return false;
	if (const USEEHealthComponent* Health = Player->FindComponentByClass<USEEHealthComponent>())
	{
		return !Health->IsDead();
	}
	return true;
}
