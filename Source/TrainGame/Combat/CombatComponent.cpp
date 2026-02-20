// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "CombatComponent.h"
#include "ProjectileBase.h"
#include "TrainGame/Weapons/WeaponComponent.h"
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
	UpdateGrapple(DeltaTime);
}

// ============================================================================
// Melee Actions
// ============================================================================

FHitResult_Combat UCombatComponent::PerformAttack(EAttackDirection Direction)
{
	FHitResult_Combat Result;

	// Can't attack if not in a valid stance
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Dodging ||
		CurrentStance == ECombatStance::Grappled)
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
	EDamageType WeaponDamageType = EDamageType::Blunt; // Unarmed = blunt

	if (WeaponComp && WeaponComp->HasWeaponEquipped() && !WeaponComp->IsWeaponBroken())
	{
		const FWeaponStats& Weapon = WeaponComp->GetCurrentWeapon();

		// Ranged weapons shouldn't be used for melee attacks (use PerformRangedAttack)
		if (!Weapon.IsRanged())
		{
			Damage = Weapon.GetEffectiveDamage();
			StaminaCost = Weapon.StaminaCostPerSwing;
			Range = Weapon.Range;
			Speed = Weapon.AttackSpeed;
			WeaponDamageType = Weapon.GetDamageType();
		}
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
		Result.DamageType = WeaponDamageType;

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
			FHitResult_Combat TargetResult = TargetCombat->ReceiveAttack(FinalDamage, Direction, WeaponDamageType, GetOwner());
			Result.bBlocked = TargetResult.bBlocked;
			Result.bDodged = TargetResult.bDodged;
			Result.DamageDealt = TargetResult.DamageDealt;
		}
		else
		{
			Result.DamageDealt = FinalDamage;
		}

		// Apply weapon durability loss
		if (WeaponComp && WeaponComp->HasWeaponEquipped())
		{
			if (Result.bBlocked)
				WeaponComp->ApplyBlockDurabilityLoss();
			else
				WeaponComp->ApplyHitDurabilityLoss();
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
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Grappled)
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
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Grappled)
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

	// Apply damage resistance
	FinalDamage = ApplyDamageResistance(FinalDamage, DamageType);

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
				// Parry: stagger the attacker briefly (respecting their stagger resistance)
				if (FMath::FRand() > AttackerCombat->StaggerResistance)
				{
					AttackerCombat->SetStance(ECombatStance::Staggered);
					AttackerCombat->StaggerTimer = StaggerDuration * 0.5f;
				}
			}
		}
	}

	// Staggered targets take more damage
	if (CurrentStance == ECombatStance::Staggered)
	{
		FinalDamage *= 1.5f;
	}

	// Grappled targets take more damage
	if (CurrentStance == ECombatStance::Grappled)
	{
		FinalDamage *= 2.f;
	}

	Result.DamageDealt = FinalDamage;
	Result.bHit = true;

	ApplyDamage(FinalDamage, Attacker);
	OnHitReceived.Broadcast(Result);

	return Result;
}

// ============================================================================
// Ranged Actions
// ============================================================================

bool UCombatComponent::PerformRangedAttack(FVector AimDirection)
{
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Grappled)
	{
		return false;
	}

	if (AttackCooldownTimer > 0.f) return false;

	UWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComp || !WeaponComp->HasWeaponEquipped()) return false;

	const FWeaponStats& Weapon = WeaponComp->GetCurrentWeapon();
	if (!Weapon.IsRanged()) return false;
	if (!Weapon.HasAmmo()) return false;
	if (Weapon.IsBroken()) return false;

	float StaminaCost = Weapon.StaminaCostPerSwing;
	if (!StaminaState.CanPerformAction(StaminaCost))
	{
		OnStaminaDepleted.Broadcast();
		return false;
	}

	ConsumeStamina(StaminaCost);
	TimeSinceLastCombatAction = 0.f;

	// Set attack cooldown (ranged weapons are generally slow)
	AttackCooldownTimer = BaseAttackCooldown / Weapon.AttackSpeed;

	// Spawn projectile
	UWorld* World = GetWorld();
	if (World && ProjectileClass)
	{
		AActor* Owner = GetOwner();
		FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.f;
		FRotator SpawnRotation = AimDirection.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);

		AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(
			ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			float DamageMultiplier = bKronoleModeActive ? 1.5f : 1.f;
			Projectile->InitProjectile(Weapon, Owner, DamageMultiplier);
		}
	}

	// Consume ammo and apply durability
	WeaponComp->ConsumeAmmo();
	WeaponComp->ApplyHitDurabilityLoss();

	SetStance(ECombatStance::Attacking);
	SetStance(ECombatStance::Neutral);

	return true;
}

bool UCombatComponent::ThrowWeapon(FVector ThrowDirection)
{
	if (CurrentStance == ECombatStance::Staggered ||
		CurrentStance == ECombatStance::Downed ||
		CurrentStance == ECombatStance::Grappled)
	{
		return false;
	}

	UWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UWeaponComponent>();
	if (!WeaponComp || !WeaponComp->HasWeaponEquipped()) return false;

	const FWeaponStats& Weapon = WeaponComp->GetCurrentWeapon();

	// Create a thrown version of the weapon
	UWorld* World = GetWorld();
	if (World && ProjectileClass)
	{
		AActor* Owner = GetOwner();
		FVector SpawnLocation = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 80.f;
		FRotator SpawnRotation = ThrowDirection.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Owner;
		SpawnParams.Instigator = Cast<APawn>(Owner);

		AProjectileBase* Projectile = World->SpawnActor<AProjectileBase>(
			ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			// Thrown weapons do 1.5x damage but are consumed
			FWeaponStats ThrownStats = Weapon;
			ThrownStats.ProjectileSpeed = 3000.f;
			Projectile->InitProjectile(ThrownStats, Owner, 1.5f);
		}
	}

	// Unequip the thrown weapon (it's gone)
	WeaponComp->UnequipWeapon();
	TimeSinceLastCombatAction = 0.f;

	return true;
}

// ============================================================================
// Stealth Takedown Reception
// ============================================================================

FHitResult_Combat UCombatComponent::ReceiveStealthTakedown(AActor* Attacker, ETakedownType TakedownType, bool bLethal)
{
	FHitResult_Combat Result;
	Result.DamageType = bLethal ? EDamageType::Bladed : EDamageType::StealthTakedown;
	Result.bStealthTakedown = true;
	Result.bHit = true;

	if (!bCanBeTakenDown)
	{
		// Boss-type enemies resist takedowns — stagger them instead
		if (FMath::FRand() > StaggerResistance)
		{
			SetStance(ECombatStance::Staggered);
			StaggerTimer = StaggerDuration;
		}
		Result.DamageDealt = 0.f;
		return Result;
	}

	if (bLethal)
	{
		// Lethal takedown: instant kill
		Result.DamageDealt = CurrentHealth;
		ApplyDamage(CurrentHealth, Attacker);
	}
	else
	{
		// Non-lethal: put target down
		Result.DamageDealt = CurrentHealth;
		Result.DamageType = EDamageType::NonLethal;
		SetStance(ECombatStance::Downed);
		CurrentHealth = 0.f;
		// Non-lethal — enemy is unconscious, not dead
		// The StealthComponent handles body state management
	}

	OnStealthTakedown.Broadcast(Attacker, TakedownType);
	return Result;
}

bool UCombatComponent::BeginGrapple(AActor* InGrappler)
{
	if (CurrentStance == ECombatStance::Downed) return false;
	if (!bCanBeTakenDown) return false;

	// Can't grapple if target is aware and facing the grappler
	// (The StealthComponent/AI handles approach validation)

	Grappler = InGrappler;
	GrappleTimer = 0.f;
	GrappleEscapeProgress = 0;
	SetStance(ECombatStance::Grappled);
	OnGrappled.Broadcast();

	return true;
}

bool UCombatComponent::AttemptGrappleEscape()
{
	if (CurrentStance != ECombatStance::Grappled) return false;

	GrappleEscapeProgress++;

	// Each escape attempt reduces remaining grapple time
	float EscapeContribution = GrappleEscapeTime / 5.f; // ~5 mashes to escape
	GrappleTimer += EscapeContribution;

	if (GrappleTimer >= GrappleEscapeTime)
	{
		// Escaped!
		Grappler = nullptr;
		GrappleTimer = 0.f;
		GrappleEscapeProgress = 0;
		SetStance(ECombatStance::Neutral);
		OnGrappleEscaped.Broadcast();
		return true;
	}

	return false;
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
// Damage Resistance
// ============================================================================

void UCombatComponent::SetDamageResistance(EDamageType Type, float Resistance)
{
	DamageResistances.Add(Type, FMath::Clamp(Resistance, 0.f, 1.f));
}

void UCombatComponent::SetStaggerResistance(float Resistance)
{
	StaggerResistance = FMath::Clamp(Resistance, 0.f, 1.f);
}

void UCombatComponent::SetMaxHealth(float NewMaxHealth)
{
	float HealthRatio = MaxHealth > 0.f ? CurrentHealth / MaxHealth : 1.f;
	MaxHealth = NewMaxHealth;
	CurrentHealth = MaxHealth * HealthRatio;
}

void UCombatComponent::Heal(float Amount)
{
	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
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

void UCombatComponent::UpdateGrapple(float DeltaTime)
{
	if (CurrentStance != ECombatStance::Grappled) return;

	// If grappler is dead or gone, escape automatically
	if (!Grappler || !Grappler->IsValidLowLevel())
	{
		Grappler = nullptr;
		SetStance(ECombatStance::Neutral);
		OnGrappleEscaped.Broadcast();
		return;
	}

	// NPC auto-escape after grapple time (AI doesn't mash buttons)
	GrappleTimer += DeltaTime * 0.3f; // Slow auto-escape for NPCs
	if (GrappleTimer >= GrappleEscapeTime)
	{
		Grappler = nullptr;
		SetStance(ECombatStance::Neutral);
		OnGrappleEscaped.Broadcast();
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

float UCombatComponent::ApplyDamageResistance(float Damage, EDamageType Type) const
{
	const float* Resistance = DamageResistances.Find(Type);
	if (Resistance)
	{
		return Damage * (1.f - FMath::Clamp(*Resistance, 0.f, 1.f));
	}
	return Damage;
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
