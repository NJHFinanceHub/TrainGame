#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"
#include "SEEHealthComponent.h"
#include "SEEStatsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USEECombatComponent::USEECombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USEECombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// State timers
	if (CurrentState == ESEECombatState::Staggered || CurrentState == ESEECombatState::Recovering)
	{
		StateTimer -= DeltaTime;
		if (StateTimer <= 0.0f)
		{
			SetCombatState(ESEECombatState::Idle);
		}
	}

	// Parry window
	if (ParryTimer > 0.0f)
	{
		ParryTimer -= DeltaTime;
		if (ParryTimer <= 0.0f && CurrentState == ESEECombatState::Parrying)
		{
			SetCombatState(ESEECombatState::Blocking);
		}
	}

	// Dodge i-frames
	if (bDodgeIFramesActive)
	{
		DodgeTimer -= DeltaTime;
		if (DodgeTimer <= 0.0f)
		{
			bDodgeIFramesActive = false;
			SetCombatState(ESEECombatState::Idle);
		}
	}

	// Combo timer
	if (ComboTimer > 0.0f)
	{
		ComboTimer -= DeltaTime;
		if (ComboTimer <= 0.0f)
		{
			ComboCount = 0;
		}
	}

	// Combat exit timer (5 seconds out of combat)
	if (bInCombat)
	{
		CombatExitTimer += DeltaTime;
		if (CombatExitTimer >= 5.0f)
		{
			bInCombat = false;
		}
	}
}

void USEECombatComponent::LightAttack()
{
	if (!CanAttack()) return;
	if (EquippedWeapon && EquippedWeapon->IsBroken()) return;

	bInCombat = true;
	CombatExitTimer = 0.0f;
	ComboCount = FMath::Min(ComboCount + 1, 3);
	ComboTimer = ComboWindow;

	SetCombatState(ESEECombatState::Attacking);

	float DamageMultiplier = 1.0f + (ComboCount - 1) * 0.15f;
	PerformWeaponTrace(DamageMultiplier);

	if (EquippedWeapon)
	{
		EquippedWeapon->DegradeDurability(1.0f);
	}

	// Recovery time based on weapon speed
	float RecoveryTime = EquippedWeapon ? (0.4f / EquippedWeapon->GetAttackSpeed()) : 0.3f;
	StateTimer = RecoveryTime;
	SetCombatState(ESEECombatState::Recovering);
}

void USEECombatComponent::HeavyAttack()
{
	if (!CanAttack()) return;
	if (EquippedWeapon && EquippedWeapon->IsBroken()) return;

	bInCombat = true;
	CombatExitTimer = 0.0f;
	ComboCount = 0;

	SetCombatState(ESEECombatState::Attacking);

	float DamageMultiplier = EquippedWeapon ? EquippedWeapon->HeavyDamageMultiplier : 2.0f;
	PerformWeaponTrace(DamageMultiplier);

	if (EquippedWeapon)
	{
		EquippedWeapon->DegradeDurability(2.0f);
	}

	float RecoveryTime = EquippedWeapon ? (0.8f / EquippedWeapon->GetAttackSpeed()) : 0.6f;
	StateTimer = RecoveryTime;
	SetCombatState(ESEECombatState::Recovering);
}

void USEECombatComponent::StartBlock()
{
	if (CurrentState == ESEECombatState::Staggered || CurrentState == ESEECombatState::Dodging) return;

	ParryTimer = ParryWindowDuration;
	SetCombatState(ESEECombatState::Parrying);
}

void USEECombatComponent::StopBlock()
{
	if (CurrentState == ESEECombatState::Blocking || CurrentState == ESEECombatState::Parrying)
	{
		ParryTimer = 0.0f;
		SetCombatState(ESEECombatState::Idle);
	}
}

void USEECombatComponent::Dodge(FVector Direction)
{
	if (CurrentState == ESEECombatState::Staggered) return;

	bInCombat = true;
	CombatExitTimer = 0.0f;

	SetCombatState(ESEECombatState::Dodging);
	bDodgeIFramesActive = true;
	DodgeTimer = DodgeIFrameDuration;

	// Apply dodge impulse
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		FVector DodgeDir = Direction.IsNearlyZero() ? -OwnerChar->GetActorForwardVector() : Direction.GetSafeNormal();
		OwnerChar->LaunchCharacter(DodgeDir * DodgeDistance, true, false);
	}
}

float USEECombatComponent::ProcessIncomingDamage(float BaseDamage, AActor* Attacker)
{
	bInCombat = true;
	CombatExitTimer = 0.0f;

	// Dodge i-frames = no damage
	if (bDodgeIFramesActive) return 0.0f;

	// Parry = counter-attack window, no damage
	if (CurrentState == ESEECombatState::Parrying)
	{
		OnParrySuccess.Broadcast();
		// Stagger the attacker
		if (Attacker)
		{
			if (USEECombatComponent* AttackerCombat = Attacker->FindComponentByClass<USEECombatComponent>())
			{
				AttackerCombat->SetCombatState(ESEECombatState::Staggered);
				AttackerCombat->StateTimer = StaggerDuration;
			}
		}
		return 0.0f;
	}

	// Block = reduced damage, stamina drain
	if (CurrentState == ESEECombatState::Blocking)
	{
		return BaseDamage * (1.0f - BlockDamageReduction);
	}

	return BaseDamage;
}

void USEECombatComponent::EquipWeapon(ASEEWeaponBase* Weapon)
{
	if (EquippedWeapon)
	{
		UnequipWeapon();
	}

	EquippedWeapon = Weapon;
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToComponent(
			Cast<ACharacter>(GetOwner())->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("weapon_r"));
	}
}

void USEECombatComponent::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		EquippedWeapon = nullptr;
	}
}

bool USEECombatComponent::CanAttack() const
{
	return CurrentState == ESEECombatState::Idle ||
		   (CurrentState == ESEECombatState::Recovering && ComboTimer > 0.0f);
}

void USEECombatComponent::SetCombatState(ESEECombatState NewState)
{
	if (CurrentState == NewState) return;
	CurrentState = NewState;
	OnCombatStateChanged.Broadcast(NewState);
}

void USEECombatComponent::PerformWeaponTrace(float DamageMultiplier)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	float TraceRange = EquippedWeapon ? EquippedWeapon->GetWeaponRange() : UnarmedRange;
	float BaseDamage = EquippedWeapon ? EquippedWeapon->GetBaseDamage() : UnarmedDamage;

	// Get strength modifier from stats
	float StrengthMod = 1.0f;
	if (USEEStatsComponent* Stats = Owner->FindComponentByClass<USEEStatsComponent>())
	{
		StrengthMod = 1.0f + Stats->GetStatModifier(ESEEStat::Strength);
	}

	// Get injury modifier from health
	float InjuryMod = 1.0f;
	if (USEEHealthComponent* Health = Owner->FindComponentByClass<USEEHealthComponent>())
	{
		InjuryMod = Health->GetMeleeDamageModifier();
	}

	float FinalDamage = BaseDamage * DamageMultiplier * StrengthMod * InjuryMod;

	// Weapon trace from character forward
	FVector Start = Owner->GetActorLocation() + FVector(0, 0, 64.0f);
	FVector End = Start + Owner->GetActorForwardVector() * TraceRange;

	// Sweep for width
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (EquippedWeapon) Params.AddIgnoredActor(EquippedWeapon);

	TArray<FHitResult> Hits;
	float SweepRadius = 30.0f;
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(SweepRadius), Params);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner) continue;

		// Apply damage through combat component first (for block/parry), then health
		float ProcessedDamage = FinalDamage;
		if (USEECombatComponent* TargetCombat = HitActor->FindComponentByClass<USEECombatComponent>())
		{
			ProcessedDamage = TargetCombat->ProcessIncomingDamage(FinalDamage, Owner);
		}

		if (ProcessedDamage > 0.0f)
		{
			if (USEEHealthComponent* TargetHealth = HitActor->FindComponentByClass<USEEHealthComponent>())
			{
				ESEEDamageType DmgType = EquippedWeapon ? EquippedWeapon->GetDamageType() : ESEEDamageType::Blunt;
				TargetHealth->TakeDamage(ProcessedDamage, DmgType, Owner);
			}
		}

		OnAttackHit.Broadcast(HitActor, ProcessedDamage);
		break; // Only hit first target in melee
	}
}
