// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	StaminaState.CurrentStamina = StaminaState.MaxStamina;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsAlive()) return;

	// Update timers
	if (AttackCooldownTimer > 0.f) AttackCooldownTimer -= DeltaTime;
	if (DodgeCooldownTimer > 0.f)
	{
		DodgeCooldownTimer -= DeltaTime;
		if (DodgeCooldownTimer <= 0.f) bDodgeOnCooldown = false;
	}

	// I-frame tracking
	if (bInIFrames)
	{
		IFrameTimer -= DeltaTime;
		if (IFrameTimer <= 0.f) bInIFrames = false;
	}

	// Stagger recovery
	if (CurrentStance == ECombatStance::Staggered)
	{
		StaggerTimer -= DeltaTime;
		if (StaggerTimer <= 0.f)
		{
			SetStance(ECombatStance::Neutral);
		}
	}

	// Blocking stamina drain
	if (CurrentStance == ECombatStance::Blocking)
	{
		ConsumeStamina(BlockStaminaDrainPerSecond * DeltaTime);
		if (StaminaState.CurrentStamina <= 0.f)
		{
			// Guard broken - stagger
			StopBlock();
			SetStance(ECombatStance::Staggered);
			StaggerTimer = StaggerDuration;
		}
	}

	// Combat action tracking for fatigue recovery
	TimeSinceLastCombatAction += DeltaTime;

	RegenerateStamina(DeltaTime);
	UpdateFatigue(DeltaTime);
	UpdateKronoleMode(DeltaTime);
}

// ============================================================================
// Actions
// ============================================================================

FHitResult_Combat UCombatComponent::PerformAttack(EAttackDirection Direction)
{
	FHitResult_Combat Result;

	// Can't attack if not in a valid stance
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Dodging)
	{
		return Result;
	}

	// Attack cooldown
	if (AttackCooldownTimer > 0.f) return Result;

	// Get weapon stats (from WeaponComponent if available, otherwise unarmed)
	UWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UWeaponComponent>();
	float Damage = UnarmedDamage;
	float StaminaCost = 10.f;
	float Range = 150.f;
	float Speed = 1.f;
	float DurabilityLoss = 0.f;

	if (WeaponComp)
	{
		// WeaponComponent provides stats - for now use unarmed as fallback
		// This will be connected when WeaponComponent is attached
	}

	// Check stamina
	if (!StaminaState.CanPerformAction(StaminaCost))
	{
		OnStaminaDepleted.Broadcast();
		return Result;
	}

	// Consume stamina
	ConsumeStamina(StaminaCost);
	StaminaState.FatigueLevel = FMath::Min(StaminaState.FatigueLevel + FatiguePerAttack, StaminaState.MaxFatigue);
	TimeSinceLastCombatAction = 0.f;

	// Set attack cooldown
	AttackCooldownTimer = BaseAttackCooldown / Speed;

	// Set stance
	SetStance(ECombatStance::Attacking);

	// Perform melee trace
	AActor* HitTarget = PerformMeleeTrace(Range);

	if (HitTarget)
	{
		Result.bHit = true;
		Result.AttackDirection = Direction;
		Result.DamageType = EDamageType::Physical;

		// Check for critical hit
		Result.bCritical = FMath::FRand() < CriticalHitChance;
		float FinalDamage = Damage;
		if (Result.bCritical)
		{
			FinalDamage *= CriticalHitMultiplier;
		}

		// Apply Kronole mode damage bonus
		if (bKronoleModeActive)
		{
			FinalDamage *= 1.5f;
		}

		// Send damage to target's combat component
		UCombatComponent* TargetCombat = HitTarget->FindComponentByClass<UCombatComponent>();
		if (TargetCombat)
		{
			FHitResult_Combat TargetResult = TargetCombat->ReceiveAttack(FinalDamage, Direction, EDamageType::Physical, GetOwner());
			Result.bBlocked = TargetResult.bBlocked;
			Result.bDodged = TargetResult.bDodged;
			Result.DamageDealt = TargetResult.DamageDealt;
		}
		else
		{
			Result.DamageDealt = FinalDamage;
		}

		OnHitLanded.Broadcast(Result);
	}

	// Return to neutral after attack (animation system would handle timing)
	SetStance(ECombatStance::Neutral);

	return Result;
}

void UCombatComponent::StartBlock(EBlockDirection Direction)
{
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed)
	{
		return;
	}

	CurrentBlockDirection = Direction;
	SetStance(ECombatStance::Blocking);
	TimeSinceLastCombatAction = 0.f;
}

void UCombatComponent::StopBlock()
{
	if (CurrentStance == ECombatStance::Blocking)
	{
		SetStance(ECombatStance::Neutral);
	}
}

bool UCombatComponent::PerformDodge(FVector DodgeDirection)
{
	if (bDodgeOnCooldown) return false;
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed)
	{
		return false;
	}
	if (!StaminaState.CanPerformAction(DodgeStaminaCost)) return false;

	ConsumeStamina(DodgeStaminaCost);
	TimeSinceLastCombatAction = 0.f;

	// Activate i-frames
	bInIFrames = true;
	IFrameTimer = DodgeIFrameDuration;

	// Set dodge cooldown
	bDodgeOnCooldown = true;
	DodgeCooldownTimer = DodgeCooldown;

	SetStance(ECombatStance::Dodging);

	// Apply dodge movement (in tight train corridors, this is a sidestep)
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar)
	{
		DodgeDirection.Normalize();
		OwnerChar->LaunchCharacter(DodgeDirection * DodgeDistance, true, false);
	}

	// Return to neutral after dodge (animation handles timing)
	SetStance(ECombatStance::Neutral);

	return true;
}

FHitResult_Combat UCombatComponent::ReceiveAttack(float IncomingDamage, EAttackDirection Direction, EDamageType DamageType, AActor* Attacker)
{
	FHitResult_Combat Result;
	Result.AttackDirection = Direction;
	Result.DamageType = DamageType;

	// Dodged (in i-frames)
	if (bInIFrames)
	{
		Result.bDodged = true;
		Result.DamageDealt = 0.f;
		return Result;
	}

	float FinalDamage = IncomingDamage;

	// Blocking
	if (CurrentStance == ECombatStance::Blocking)
	{
		float Reduction = CalculateBlockReduction(Direction);
		FinalDamage *= (1.f - Reduction);
		Result.bBlocked = true;

		// Blocking costs stamina
		ConsumeStamina(BlockStaminaCost);

		// If perfect block (correct direction), can stagger the attacker
		if (DoesBlockMatchAttack(Direction) && Attacker)
		{
			UCombatComponent* AttackerCombat = Attacker->FindComponentByClass<UCombatComponent>();
			if (AttackerCombat)
			{
				// Parry: stagger the attacker briefly
				AttackerCombat->SetStance(ECombatStance::Staggered);
				AttackerCombat->StaggerTimer = StaggerDuration * 0.5f;
			}
		}
	}

	// Staggered targets take more damage
	if (CurrentStance == ECombatStance::Staggered)
	{
		FinalDamage *= 1.5f;
	}

	Result.DamageDealt = FinalDamage;
	Result.bHit = true;

	ApplyDamage(FinalDamage, Attacker);
	OnHitReceived.Broadcast(Result);

	return Result;
}

// ============================================================================
// Kronole Mode
// ============================================================================

bool UCombatComponent::ActivateKronoleMode()
{
	if (bKronoleModeActive) return false;
	if (KronoleCooldownTimer > 0.f) return false;
	if (CurrentHealth < KronoleMinHealthRequired) return false;

	bKronoleModeActive = true;
	KronoleModeTimer = 0.f;

	// Apply time dilation to the world (player perceives everything in slow-mo)
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, KronoleTimeDilation);
	}

	SetStance(ECombatStance::KronoleMode);
	OnKronoleModeActivated.Broadcast();

	return true;
}

void UCombatComponent::DeactivateKronoleMode()
{
	if (!bKronoleModeActive) return;

	bKronoleModeActive = false;
	KronoleCooldownTimer = KronoleCooldown;

	// Restore normal time
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	}

	SetStance(ECombatStance::Neutral);
	OnKronoleModeDeactivated.Broadcast();
}

// ============================================================================
// Private
// ============================================================================

void UCombatComponent::SetStance(ECombatStance NewStance)
{
	if (CurrentStance != NewStance)
	{
		CurrentStance = NewStance;
		OnStanceChanged.Broadcast(NewStance);
	}
}

void UCombatComponent::ConsumeStamina(float Amount)
{
	float OldStamina = StaminaState.CurrentStamina;
	StaminaState.CurrentStamina = FMath::Max(0.f, StaminaState.CurrentStamina - Amount);

	if (OldStamina != StaminaState.CurrentStamina)
	{
		OnStaminaChanged.Broadcast(StaminaState.CurrentStamina);
	}

	if (StaminaState.CurrentStamina <= 0.f)
	{
		OnStaminaDepleted.Broadcast();
	}
}

void UCombatComponent::RegenerateStamina(float DeltaTime)
{
	// Don't regenerate while blocking or in combat stance
	if (CurrentStance == ECombatStance::Blocking ||
		CurrentStance == ECombatStance::Attacking)
	{
		return;
	}

	float EffectiveMax = StaminaState.GetEffectiveMaxStamina();
	if (StaminaState.CurrentStamina < EffectiveMax)
	{
		// Slower regen during Kronole mode
		float RegenRate = StaminaState.StaminaRegenRate;
		if (bKronoleModeActive) RegenRate *= 0.5f;

		StaminaState.CurrentStamina = FMath::Min(
			StaminaState.CurrentStamina + RegenRate * DeltaTime,
			EffectiveMax
		);
		OnStaminaChanged.Broadcast(StaminaState.CurrentStamina);
	}
}

void UCombatComponent::UpdateFatigue(float DeltaTime)
{
	// Recover fatigue when out of combat for a while
	if (TimeSinceLastCombatAction > FatigueRecoveryDelay && StaminaState.FatigueLevel > 0.f)
	{
		StaminaState.FatigueLevel = FMath::Max(0.f,
			StaminaState.FatigueLevel - FatigueRecoveryRate * DeltaTime);
	}
}

void UCombatComponent::UpdateKronoleMode(float DeltaTime)
{
	// Kronole cooldown
	if (KronoleCooldownTimer > 0.f)
	{
		KronoleCooldownTimer -= DeltaTime;
	}

	if (!bKronoleModeActive) return;

	// Kronole mode drains health using real time (not dilated)
	float RealDeltaTime = DeltaTime / KronoleTimeDilation;
	KronoleModeTimer += RealDeltaTime;

	// Health cost
	ApplyDamage(KronoleHealthCostPerSecond * RealDeltaTime, nullptr);

	// Auto-deactivate at max duration or low health
	if (KronoleModeTimer >= KronoleMaxDuration || CurrentHealth <= KronoleMinHealthRequired)
	{
		DeactivateKronoleMode();
	}
}

void UCombatComponent::ApplyDamage(float Damage, AActor* DamageSource)
{
	if (Damage <= 0.f) return;

	CurrentHealth = FMath::Max(0.f, CurrentHealth - Damage);

	if (CurrentHealth <= 0.f)
	{
		Die(DamageSource);
	}
}

void UCombatComponent::Die(AActor* Killer)
{
	if (bKronoleModeActive)
	{
		DeactivateKronoleMode();
	}

	SetStance(ECombatStance::Downed);
	OnDeath.Broadcast(Killer);
}

float UCombatComponent::CalculateBlockReduction(EAttackDirection AttackDir) const
{
	if (DoesBlockMatchAttack(AttackDir))
	{
		return BlockDamageReduction;
	}
	return PartialBlockReduction;
}

bool UCombatComponent::DoesBlockMatchAttack(EAttackDirection AttackDir) const
{
	// High attacks matched by high block
	// Mid/Left/Right attacks matched by mid block
	// Low attacks matched by low block
	switch (AttackDir)
	{
		case EAttackDirection::High:
			return CurrentBlockDirection == EBlockDirection::High;
		case EAttackDirection::Low:
			return CurrentBlockDirection == EBlockDirection::Low;
		default:
			return CurrentBlockDirection == EBlockDirection::Mid;
	}
}

AActor* UCombatComponent::PerformMeleeTrace(float TraceRange) const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	FVector End = Start + Forward * TraceRange;

	// Sphere trace for melee (wider than line trace to account for weapon swing arc)
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(50.f),
		Params
	);

	if (bHit && HitResult.GetActor())
	{
		return HitResult.GetActor();
	}

	return nullptr;
}
