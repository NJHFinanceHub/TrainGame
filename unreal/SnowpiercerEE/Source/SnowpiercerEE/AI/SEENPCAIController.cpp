// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCAIController.h"

#include "SEENPCHealthComponent.h"
#include "SnowpiercerEE/SEECombatComponent.h"
#include "SnowpiercerEE/SEEFactionManager.h"
#include "SnowpiercerEE/SEEJackbootCharacter.h"
#include "SnowpiercerEE/SEECivilianCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
		UpdateReturnHome(DeltaTime);
		break;

	case ESEENPCBrainState::Wander:
		UpdateWander(DeltaTime);
		break;

	default:
		// Idle is timer-driven; only a leftover direct-move fallback ticks.
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
		else if (GetFocusActor() == Player)
		{
			// Only clear focus *we* put on the player — wander goals and
			// ambient turns park focal points in the same Gameplay slot.
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

	// Navigation known-broken: steer directly for the WHOLE chase (not 2s
	// bursts). The goal and timer are refreshed every tick so the direct move
	// never expires until nav is re-tested.
	if (IsNavBlocked())
	{
		bDirectMoveActive = true;
		DirectMoveGoal = Player->GetActorLocation();
		DirectMoveTimeLeft = 1.0f; // refreshed each tick — effectively endless
		DirectMoveStopDist = AttackRange * 0.8f;
		UpdateDirectMove(DeltaTime);
		return;
	}

	// Direct-movement fallback (stall burst) drives the pawn straight at the player.
	if (bDirectMoveActive)
	{
		DirectMoveGoal = Player->GetActorLocation();
		UpdateDirectMove(DeltaTime);
		return;
	}

	// (Re)issue a pathed move periodically. A Failed result inside RequestMove
	// flips bNavUnavailable and starts direct steering immediately.
	if (PathRetryCooldown <= 0.0f)
	{
		PathRetryCooldown = GPathRetryInterval;
		RequestMove(Player->GetActorLocation(), Player, ChaseAcceptanceRadius);
		if (bDirectMoveActive) return; // request failed, already steering
	}

	// Stall detection: pathing reported success but the pawn is not moving
	// (partial navmesh / blocked path). Burst into direct movement.
	if (PawnChar->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		StuckTime += DeltaTime;
		if (StuckTime >= GStuckTimeBeforeFallback)
		{
			StuckTime = 0.0f;
			StopMovement();
			StartDirectMove(Player->GetActorLocation(), GDirectMoveBurst, AttackRange * 0.8f);
		}
	}
	else
	{
		StuckTime = 0.0f;
	}
}

void ASEENPCAIController::UpdateReturnHome(float DeltaTime)
{
	ACharacter* PawnChar = GetPawnCharacter();
	if (!PawnChar) return;

	const float Dist = FVector::Dist2D(PawnChar->GetActorLocation(), HomeLocation);
	if (Dist <= 100.0f)
	{
		EnterIdle();
		return;
	}

	if (bDirectMoveActive)
	{
		UpdateDirectMove(DeltaTime);
		return;
	}

	// Nav known-broken: walk the whole way home on direct steering.
	if (IsNavBlocked())
	{
		StartDirectMove(HomeLocation, 30.0f, 70.0f);
		UpdateDirectMove(DeltaTime);
		return;
	}

	if (PathRetryCooldown <= 0.0f)
	{
		PathRetryCooldown = GPathRetryInterval;
		RequestMove(HomeLocation, nullptr, 80.0f);
		if (bDirectMoveActive) return;
	}

	if (PawnChar->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		StuckTime += DeltaTime;
		if (StuckTime >= GStuckTimeBeforeFallback)
		{
			StuckTime = 0.0f;
			StopMovement();
			StartDirectMove(HomeLocation, GDirectMoveBurst, 70.0f);
		}
	}
	else
	{
		StuckTime = 0.0f;
	}
}

void ASEENPCAIController::UpdateWander(float DeltaTime)
{
	ACharacter* PawnChar = GetPawnCharacter();
	if (!PawnChar)
	{
		BrainState = ESEENPCBrainState::Idle;
		return;
	}

	WanderElapsed += DeltaTime;

	// Direct steering leg: UpdateDirectMove arrives/expires/stalls and drops
	// back to Idle on its own.
	if (bDirectMoveActive)
	{
		UpdateDirectMove(DeltaTime);
		return;
	}

	// Pathed leg in flight.
	const float Dist = FVector::Dist2D(PawnChar->GetActorLocation(), ActiveWanderGoal);
	if (Dist <= WanderArriveRadius || WanderElapsed >= WanderMoveTimeout)
	{
		StopMovement();
		BrainState = ESEENPCBrainState::Idle;
		return;
	}

	// Stall on a "successful" pathed move (partial navmesh): finish the leg
	// with direct steering instead of standing still.
	if (PawnChar->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		StuckTime += DeltaTime;
		if (StuckTime >= GStuckTimeBeforeFallback)
		{
			StuckTime = 0.0f;
			StopMovement();
			StartDirectMove(ActiveWanderGoal,
				FMath::Max(WanderMoveTimeout - WanderElapsed, 1.0f), WanderArriveRadius);
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

	// Stall detection (same idea chase uses): pushing into a wall/prop without
	// progress. Chasers keep shoving — corridors run along +X and the player
	// may free the lane — but wanderers/returners give up and re-plan later.
	bool bStalled = false;
	if (ControlledPawn->GetVelocity().Size2D() < GStuckSpeedThreshold)
	{
		DirectStuckTime += DeltaTime;
		bStalled = (DirectStuckTime >= GStuckTimeBeforeFallback)
			&& (BrainState != ESEENPCBrainState::Chase);
	}
	else
	{
		DirectStuckTime = 0.0f;
	}

	if (DirectMoveTimeLeft <= 0.0f || ToGoal.Size2D() <= DirectMoveStopDist || bStalled)
	{
		bDirectMoveActive = false;
		DirectStuckTime = 0.0f;
		PathRetryCooldown = 0.0f; // try pathfinding again right away

		// A wander leg ends here either way — idle until the next timer.
		if (BrainState == ESEENPCBrainState::Wander)
		{
			StopMovement();
			BrainState = ESEENPCBrainState::Idle;
		}
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

	if (Result == EPathFollowingRequestResult::Failed)
	{
		// No navmesh (degenerate bounds volume) or unreachable goal: remember
		// that nav is down so every consumer steers directly, and start the
		// straight-line fallback for this request immediately.
		if (!bNavUnavailable)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("SEENPCBrain: %s pathed move FAILED (no usable navmesh?) — direct steering for %.0fs"),
				*GetNameSafe(GetPawn()), NavRetestInterval);
		}
		bNavUnavailable = true;
		NextNavRetestTime = GetWorld()->GetTimeSeconds() + NavRetestInterval;

		const FVector Goal = GoalActor ? GoalActor->GetActorLocation() : GoalLocation;
		const bool bChasing = (BrainState == ESEENPCBrainState::Chase);
		StartDirectMove(Goal,
			bChasing ? GDirectMoveBurst : WanderMoveTimeout,
			bChasing ? AttackRange * 0.8f : WanderArriveRadius);
	}
	else
	{
		// A path was actually built — navigation works again.
		bNavUnavailable = false;
	}
}

void ASEENPCAIController::StartDirectMove(const FVector& Goal, float Duration, float StopDist)
{
	bDirectMoveActive = true;
	DirectMoveGoal = Goal;
	DirectMoveTimeLeft = Duration;
	DirectMoveStopDist = StopDist;
	DirectStuckTime = 0.0f;
}

bool ASEENPCAIController::IsNavBlocked() const
{
	return bNavUnavailable && GetWorld()->GetTimeSeconds() < NextNavRetestTime;
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

	// Don't pace around while the player is standing next to us — perception
	// already keeps us facing them.
	if (APawn* Player = GetPlayerPawn())
	{
		if (FVector::DistSquared(ControlledPawn->GetActorLocation(), Player->GetActorLocation())
			<= FMath::Square(FacePlayerRange))
		{
			return;
		}
	}

	// Ambient life: sometimes just turn in place instead of walking.
	if (FMath::FRand() < AmbientTurnChance)
	{
		BrainState = ESEENPCBrainState::Idle;
		DoAmbientTurn();
		return;
	}

	FVector Goal;
	if (!TryPickWanderPoint(Goal))
	{
		// Hemmed in by props — at least look around.
		BrainState = ESEENPCBrainState::Idle;
		DoAmbientTurn();
		return;
	}

	BrainState = ESEENPCBrainState::Wander;
	ActiveWanderGoal = Goal;
	WanderElapsed = 0.0f;
	StuckTime = 0.0f;
	SetMoveSpeed(WalkSpeed); // unhurried shuffle; StartChase restores ChaseSpeed
	ClearFocus(EAIFocusPriority::Gameplay);
	SetFocalPoint(Goal);

	if (IsNavBlocked())
	{
		// Skip the doomed pathed request and steer straight away.
		StartDirectMove(Goal, WanderMoveTimeout, WanderArriveRadius);
	}
	else
	{
		// Failed requests flip to direct steering inside RequestMove.
		RequestMove(Goal, nullptr, 60.0f);
	}
}

bool ASEENPCAIController::TryPickWanderPoint(FVector& OutGoal) const
{
	const ACharacter* PawnChar = GetPawnCharacter();
	UWorld* World = GetWorld();
	if (!PawnChar || !World) return false;

	const UCapsuleComponent* Capsule = PawnChar->GetCapsuleComponent();
	const float CapsuleRadius = Capsule ? Capsule->GetScaledCapsuleRadius() : 35.0f;
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 90.0f;
	const FVector PawnLoc = PawnChar->GetActorLocation();

	FCollisionQueryParams Params(FName(TEXT("SEEWanderPick")), false, PawnChar);

	for (int32 Attempt = 0; Attempt < 6; ++Attempt)
	{
		const FVector2D Offset = FMath::RandPointInCircle(WanderRadius);
		FVector Goal = HomeLocation + FVector(Offset.X, Offset.Y, 0.0f);

		// Stay inside the car interior (walls sit at |Y| ~ half car width).
		Goal.Y = FMath::Clamp(Goal.Y, -WanderYClamp + CapsuleRadius, WanderYClamp - CapsuleRadius);

		// Floor check: trace down through the goal column (floors sit at z=0,
		// so a generous window around the pawn's own height is plenty).
		FHitResult Floor;
		const FVector FloorStart(Goal.X, Goal.Y, PawnLoc.Z + 100.0f);
		const FVector FloorEnd(Goal.X, Goal.Y, PawnLoc.Z - 400.0f);
		if (!World->LineTraceSingleByChannel(Floor, FloorStart, FloorEnd, ECC_Visibility, Params))
		{
			continue; // hole / off the car — pick again
		}
		Goal.Z = Floor.ImpactPoint.Z + HalfHeight;

		// Wall check: capsule-radius sphere sweep at chest height along the
		// straight approach. If something blocks, stop short of it instead of
		// grinding into a crate; if it is right in our face, pick elsewhere.
		FHitResult Wall;
		const FVector SweepEnd(Goal.X, Goal.Y, PawnLoc.Z);
		const bool bBlocked = World->SweepSingleByChannel(Wall, PawnLoc, SweepEnd,
			FQuat::Identity, ECC_Visibility,
			FCollisionShape::MakeSphere(CapsuleRadius), Params);
		if (bBlocked)
		{
			if (Wall.Distance < CapsuleRadius + 100.0f)
			{
				continue;
			}
			const FVector Dir = (SweepEnd - PawnLoc).GetSafeNormal2D();
			const FVector Short = PawnLoc + Dir * (Wall.Distance - CapsuleRadius);
			Goal.X = Short.X;
			Goal.Y = Short.Y;
		}

		// A leg shorter than the arrive radius is not worth standing up for.
		if (FVector::Dist2D(Goal, PawnLoc) <= WanderArriveRadius)
		{
			continue;
		}

		OutGoal = Goal;
		return true;
	}

	return false;
}

void ASEENPCAIController::DoAmbientTurn()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn) return;

	APawn* Player = GetPlayerPawn();

	// Prefer facing a nearby fellow NPC so idle clusters read as conversations.
	const ACharacter* Neighbor = nullptr;
	float BestDistSq = FMath::Square(AmbientFaceNPCRange);
	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		const ACharacter* Other = *It;
		if (!IsValid(Other) || Other == ControlledPawn || Other == Player) continue;
		const float DistSq = FVector::DistSquared(
			Other->GetActorLocation(), ControlledPawn->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Neighbor = Other;
		}
	}

	if (Neighbor && FMath::FRand() < 0.6f)
	{
		SetFocalPoint(Neighbor->GetActorLocation());
	}
	else
	{
		// Small random yaw drift, expressed as a focal point: SetControlRotation
		// would be stomped by UpdateControlRotation (it re-derives the control
		// rotation from pawn orientation whenever no focal point is set).
		const float DriftYaw = ControlledPawn->GetActorRotation().Yaw
			+ FMath::FRandRange(-AmbientYawDriftMax, AmbientYawDriftMax);
		const FVector DriftDir = FRotator(0.0f, DriftYaw, 0.0f).Vector();
		SetFocalPoint(ControlledPawn->GetActorLocation() + DriftDir * 400.0f);
	}
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

	USEEFactionManager::NotifyNPCKilled(this, GetPawn()); // faction kill ripple (classifies jackboot/merchant/civilian)

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
	if (USEEFactionManager::ShouldPawnRefuseDialogue(GetPawn())) return false; // standing too low — they turn their back
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
