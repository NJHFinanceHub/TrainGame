#include "SEECombatComponent.h"
#include "SEEWeaponBase.h"
#include "SEEHealthComponent.h"
#include "SEEStatsComponent.h"
#include "SEECharacter.h"
#include "TrainGame/Economy/ArmorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

USEECombatComponent::USEECombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Replicated so its Server RPCs (ServerLightAttack/ServerHeavyAttack) route.
	SetIsReplicatedByDefault(true);

	// 3-hit light combo with escalating damage
	ComboDamageMultipliers = { 1.0f, 1.1f, 1.3f };
}

void USEECombatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Never leave the world stuck in hit-stop dilation if we go away mid-freeze
	if (UWorld* World = GetWorld())
	{
		if (World->GetTimerManager().IsTimerActive(HitStopTimer))
		{
			World->GetTimerManager().ClearTimer(HitStopTimer);
			UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
		}
	}

	Super::EndPlay(EndPlayReason);
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
			if (ASEECharacter* OwnerChar = GetOwnerSEECharacter())
			{
				OwnerChar->SetInvulnerable(false);
			}
			if (CurrentState == ESEECombatState::Dodging)
			{
				SetCombatState(ESEECombatState::Idle);
			}
		}
	}

	// Dodge cooldown
	DodgeCooldownRemaining = FMath::Max(0.0f, DodgeCooldownRemaining - DeltaTime);

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
	AActor* Owner = GetOwner();
	const bool bAuthority = !Owner || Owner->HasAuthority();

	if (bAuthority)
	{
		// Host / standalone: authority runs the real swing directly (single-player path intact).
		ExecuteLightAttack();
	}
	else
	{
		// Owning client: immediate local cosmetic feedback, then ask the server to
		// run the authoritative sweep + damage.
		PlayLocalSwingFeedback();
		ServerLightAttack();
	}
}

void USEECombatComponent::HeavyAttack()
{
	AActor* Owner = GetOwner();
	const bool bAuthority = !Owner || Owner->HasAuthority();

	if (bAuthority)
	{
		ExecuteHeavyAttack();
	}
	else
	{
		PlayLocalSwingFeedback();
		ServerHeavyAttack();
	}
}

// --- Server RPCs (authoritative attack execution) ---

bool USEECombatComponent::ServerLightAttack_Validate() { return true; }
void USEECombatComponent::ServerLightAttack_Implementation()
{
	ExecuteLightAttack();
}

bool USEECombatComponent::ServerHeavyAttack_Validate() { return true; }
void USEECombatComponent::ServerHeavyAttack_Implementation()
{
	ExecuteHeavyAttack();
}

void USEECombatComponent::ExecuteLightAttack()
{
	if (!CanAttack()) return;
	if (EquippedWeapon && EquippedWeapon->IsBroken()) return;

	// Chain the next combo step if pressed inside the window after the previous active phase
	if (ComboTimer > 0.0f && ComboCount > 0)
	{
		ComboCount = FMath::Min(ComboCount + 1, ComboDamageMultipliers.Num());
	}
	else
	{
		ComboCount = 1;
	}

	// Light attacks stay available at zero stamina but still drain it
	if (ASEECharacter* OwnerChar = GetOwnerSEECharacter())
	{
		OwnerChar->ConsumeStamina(EquippedWeapon ? EquippedWeapon->StaminaCostLight : LightStaminaCost);
	}

	BeginAttack(false);
}

void USEECombatComponent::ExecuteHeavyAttack()
{
	if (!CanAttack()) return;
	if (EquippedWeapon && EquippedWeapon->IsBroken()) return;

	// Heavy attacks require the full stamina cost up front
	const float StaminaCost = EquippedWeapon ? EquippedWeapon->StaminaCostHeavy : HeavyStaminaCost;
	if (ASEECharacter* OwnerChar = GetOwnerSEECharacter())
	{
		if (!OwnerChar->HasStamina(StaminaCost)) return;
		OwnerChar->ConsumeStamina(StaminaCost);
	}

	ComboCount = 0;
	ComboTimer = 0.0f;

	BeginAttack(true);
}

void USEECombatComponent::PlayLocalSwingFeedback()
{
	// Immediate swing whoosh on the owning client (damage stays server-side).
	EnsureFoleyLoaded();
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (World && SwingSound && Owner)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SwingSound, Owner->GetActorLocation(), 0.45f);
	}
}

void USEECombatComponent::BeginAttack(bool bHeavy)
{
	bInCombat = true;
	CombatExitTimer = 0.0f;
	bPendingHeavyAttack = bHeavy;

	if (bHeavy)
	{
		PendingDamageMultiplier = EquippedWeapon ? EquippedWeapon->HeavyDamageMultiplier : HeavyAttackMultiplier;
	}
	else
	{
		const int32 ComboIndex = FMath::Clamp(ComboCount - 1, 0, ComboDamageMultipliers.Num() - 1);
		PendingDamageMultiplier = ComboDamageMultipliers.IsValidIndex(ComboIndex) ? ComboDamageMultipliers[ComboIndex] : 1.0f;
	}

	ComboTimer = 0.0f; // the chain window reopens once the hit's active phase fires
	SetCombatState(ESEECombatState::Attacking);

	// Forward lunge so swings carry momentum down the corridor
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		const float Lunge = bHeavy ? HeavyLungeImpulse : LightLungeImpulse;
		OwnerChar->LaunchCharacter(OwnerChar->GetActorForwardVector() * Lunge, false, false);
	}

	// Timer-driven active frame (no montages — windup is pure code timing)
	const float AttackSpeed = EquippedWeapon ? FMath::Max(EquippedWeapon->GetAttackSpeed(), 0.1f) : 1.0f;
	const float Windup = (bHeavy ? HeavyAttackWindup : LightAttackWindup) / AttackSpeed;
	GetWorld()->GetTimerManager().SetTimer(AttackWindupTimer, this, &USEECombatComponent::OnAttackWindupComplete, Windup, false);
}

void USEECombatComponent::OnAttackWindupComplete()
{
	// Interrupted (staggered, dodged, etc.) during windup — the swing whiffs entirely
	if (CurrentState != ESEECombatState::Attacking) return;

	// Soft lock: nudge light swings toward the nearest frontal enemy before sweeping
	if (!bPendingHeavyAttack)
	{
		ApplyTargetAssist();
	}

	PerformWeaponTrace(PendingDamageMultiplier);

	if (EquippedWeapon)
	{
		EquippedWeapon->DegradeDurability(bPendingHeavyAttack ? 2.0f : 1.0f);
	}

	// Our swing was parried — the defender staggered us mid-trace; don't overwrite it
	if (CurrentState == ESEECombatState::Staggered)
	{
		ComboCount = 0;
		ComboTimer = 0.0f;
		return;
	}

	// Recovery time based on weapon speed
	const float AttackSpeed = EquippedWeapon ? FMath::Max(EquippedWeapon->GetAttackSpeed(), 0.1f) : 1.0f;
	float RecoveryTime = (bPendingHeavyAttack ? 0.8f : 0.4f) / AttackSpeed;

	const bool bComboFinisher = !bPendingHeavyAttack && ComboCount >= ComboDamageMultipliers.Num();
	if (bComboFinisher)
	{
		RecoveryTime += ComboFinisherExtraRecovery;
	}

	StateTimer = RecoveryTime;
	SetCombatState(ESEECombatState::Recovering);

	if (bPendingHeavyAttack || bComboFinisher)
	{
		// Heavies and finishers reset the chain
		ComboCount = 0;
		ComboTimer = 0.0f;
	}
	else
	{
		// Chain window opens now that the hit's active phase has fired
		ComboTimer = ComboWindow;
	}
}

void USEECombatComponent::StartBlock()
{
	if (CurrentState == ESEECombatState::Staggered ||
		CurrentState == ESEECombatState::Dodging ||
		CurrentState == ESEECombatState::Attacking) return;

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
	if (CurrentState == ESEECombatState::Staggered || CurrentState == ESEECombatState::Attacking) return;
	if (DodgeCooldownRemaining > 0.0f) return;

	// Dodge requires the full stamina cost
	ASEECharacter* OwnerSEEChar = GetOwnerSEECharacter();
	if (OwnerSEEChar)
	{
		if (!OwnerSEEChar->HasStamina(DodgeStaminaCost)) return;
		OwnerSEEChar->ConsumeStamina(DodgeStaminaCost);
	}

	bInCombat = true;
	CombatExitTimer = 0.0f;
	DodgeCooldownRemaining = DodgeCooldown;

	SetCombatState(ESEECombatState::Dodging);
	bDodgeIFramesActive = true;
	DodgeTimer = DodgeIFrameDuration;

	// Apply dodge impulse — input direction, or backward when standing still
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		FVector DodgeDir = Direction.GetSafeNormal2D();
		if (DodgeDir.IsNearlyZero())
		{
			DodgeDir = -OwnerChar->GetActorForwardVector().GetSafeNormal2D();
		}
		OwnerChar->LaunchCharacter(DodgeDir * DodgeDistance, true, false);
	}

	if (OwnerSEEChar)
	{
		OwnerSEEChar->SetInvulnerable(true);
		OwnerSEEChar->AddCameraFOVImpulse(DodgeFOVPulse);
	}
}

float USEECombatComponent::ProcessIncomingDamage(float BaseDamage, AActor* Attacker)
{
	return ProcessIncomingHit(BaseDamage, Attacker, false);
}

float USEECombatComponent::ProcessIncomingHit(float BaseDamage, AActor* Attacker, bool bBreaksBlock)
{
	bInCombat = true;
	CombatExitTimer = 0.0f;

	AActor* Owner = GetOwner();
	ASEECharacter* OwnerSEEChar = GetOwnerSEECharacter();

	// Dodge i-frames = no damage
	if (bDodgeIFramesActive || (OwnerSEEChar && OwnerSEEChar->IsInvulnerable())) return 0.0f;

	// Facing check — block and parry only cover the frontal arc
	bool bFrontal = true;
	if (Owner && Attacker)
	{
		const FVector ToAttacker = (Attacker->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
		const float FacingDot = FVector::DotProduct(Owner->GetActorForwardVector().GetSafeNormal2D(), ToAttacker);
		bFrontal = FacingDot >= FMath::Cos(FMath::DegreesToRadians(BlockArcDegrees * 0.5f));
	}

	// Parry = block raised within the window before the hit: negate damage, punish the attacker
	if (CurrentState == ESEECombatState::Parrying && bFrontal)
	{
		OnParrySuccess.Broadcast();
		if (Attacker && Owner)
		{
			if (USEECombatComponent* AttackerCombat = Attacker->FindComponentByClass<USEECombatComponent>())
			{
				AttackerCombat->SetCombatState(ESEECombatState::Staggered);
				AttackerCombat->StateTimer = StaggerDuration;
			}
			else if (ACharacter* AttackerChar = Cast<ACharacter>(Attacker))
			{
				// No compatible combat component — shove the attacker back instead
				const FVector PushDir = (Attacker->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
				AttackerChar->LaunchCharacter(PushDir * ParryKnockbackImpulse + FVector(0.0f, 0.0f, 60.0f), false, false);
			}
		}
		TriggerHitStop();
		return 0.0f;
	}

	// Block = reduced frontal damage; heavies smash straight through
	float DamageThrough = BaseDamage;
	if (CurrentState == ESEECombatState::Blocking && bFrontal)
	{
		if (bBreaksBlock)
		{
			OnBlockBroken.Broadcast();
			SetCombatState(ESEECombatState::Staggered);
			StateTimer = BlockBreakStaggerDuration;
		}
		else
		{
			// Blocking a hit chips stamina
			if (OwnerSEEChar)
			{
				OwnerSEEChar->ConsumeStamina(BlockStaminaDrainRate);
			}
			DamageThrough = BaseDamage * (1.0f - BlockDamageReduction);
		}
	}

	// Worn armor absorbs part of whatever got through, wearing down in return
	if (Owner)
	{
		if (UArmorComponent* Armor = Owner->FindComponentByClass<UArmorComponent>())
		{
			const float Reduction = FMath::Clamp(Armor->GetTotalDamageReduction(), 0.0f, 0.9f);
			DamageThrough *= (1.0f - Reduction);
			Armor->ApplyHitToArmor(EArmorSlot::Torso, BaseDamage * ArmorWearPerHit);
		}
	}

	return DamageThrough;
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

	EnsureFoleyLoaded();
	UWorld* World = GetWorld();
	if (World && SwingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SwingSound, Owner->GetActorLocation(), 0.45f);
	}

	// Weapon trace from character forward
	const FVector Start = Owner->GetActorLocation() + FVector(0, 0, 48.0f);
	const FVector End = Start + Owner->GetActorForwardVector() * TraceRange;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	if (EquippedWeapon) Params.AddIgnoredActor(EquippedWeapon);

	// Primary: a fat sphere sweep down the swing arc.
	TArray<FHitResult> Hits;
	if (World)
	{
		World->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
			ECC_Pawn, FCollisionShape::MakeSphere(MeleeSweepRadius), Params);
	}

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == Owner) continue;
		if (ApplyMeleeHitTo(HitActor, FinalDamage))
		{
			return; // landed on a damageable target — melee hits one
		}
	}

	// Fallback: the sweep can miss a capsule whose Pawn-channel response isn't Block
	// (crowd NPCs, odd presets). Object-type overlap finds every pawn reliably; pick
	// the nearest one inside a frontal cone and hit it. This is what makes melee
	// actually connect in the corridors.
	if (World)
	{
		const FVector OwnerLoc = Owner->GetActorLocation();
		const FVector Forward = Owner->GetActorForwardVector().GetSafeNormal2D();
		const float Reach = TraceRange + MeleeSweepRadius;

		TArray<FOverlapResult> Overlaps;
		FCollisionObjectQueryParams ObjParams(ECC_Pawn);
		World->OverlapMultiByObjectType(Overlaps, OwnerLoc, FQuat::Identity,
			ObjParams, FCollisionShape::MakeSphere(Reach), Params);

		AActor* Best = nullptr;
		float BestDistSq = TNumericLimits<float>::Max();
		for (const FOverlapResult& Ov : Overlaps)
		{
			AActor* Cand = Ov.GetActor();
			if (!Cand || Cand == Owner) continue;
			if (!Cand->FindComponentByClass<USEEHealthComponent>()) continue;
			const FVector To = (Cand->GetActorLocation() - OwnerLoc);
			const FVector To2D = To.GetSafeNormal2D();
			if (FVector::DotProduct(Forward, To2D) < 0.35f) continue; // ~70 deg frontal cone
			const float DistSq = To.SizeSquared2D();
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				Best = Cand;
			}
		}
		if (Best)
		{
			ApplyMeleeHitTo(Best, FinalDamage);
		}
	}
}

bool USEECombatComponent::ApplyMeleeHitTo(AActor* HitActor, float FinalDamage)
{
	AActor* Owner = GetOwner();
	if (!HitActor || HitActor == Owner) return false;

	// A damageable target must have a health component (the AI controller adds one
	// to every adopted NPC on possess).
	USEEHealthComponent* TargetHealth = HitActor->FindComponentByClass<USEEHealthComponent>();
	if (!TargetHealth) return false;

	// Route through the target's combat component first (block/parry), then health.
	float ProcessedDamage = FinalDamage;
	if (USEECombatComponent* TargetCombat = HitActor->FindComponentByClass<USEECombatComponent>())
	{
		ProcessedDamage = TargetCombat->ProcessIncomingHit(FinalDamage, Owner, bPendingHeavyAttack);
	}

	if (ProcessedDamage > 0.0f)
	{
		const ESEEDamageType DmgType = EquippedWeapon ? EquippedWeapon->GetDamageType() : ESEEDamageType::Blunt;
		TargetHealth->TakeDamage(ProcessedDamage, DmgType, Owner);

		if (ACharacter* VictimChar = Cast<ACharacter>(HitActor))
		{
			const FVector Away = (HitActor->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal2D();
			const float Knockback = bPendingHeavyAttack ? HeavyHitKnockback : LightHitKnockback;
			const float UpKick = bPendingHeavyAttack ? 120.0f : 40.0f;
			VictimChar->LaunchCharacter(Away * Knockback + FVector(0.0f, 0.0f, UpKick), false, false);
		}
	}

	// Feedback: hit sound at the victim + a timestamp the HUD reads for the hitmarker
	if (UWorld* World = GetWorld())
	{
		LastHitLandedTime = World->GetTimeSeconds();
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(World, HitSound, HitActor->GetActorLocation(), 0.8f);
		}
	}

	TriggerHitStop();
	OnAttackHit.Broadcast(HitActor, ProcessedDamage);
	return true;
}

float USEECombatComponent::GetTimeSinceHitLanded() const
{
	const UWorld* World = GetWorld();
	if (!World) return 1000.0f;
	return World->GetTimeSeconds() - LastHitLandedTime;
}

void USEECombatComponent::EnsureFoleyLoaded()
{
	if (bFoleyLoaded) return;
	bFoleyLoaded = true;
	HitSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Combat/SFX_MeleeHit_01.SFX_MeleeHit_01"));
	SwingSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/Combat/SFX_MeleeSwing_01.SFX_MeleeSwing_01"));
}

void USEECombatComponent::ApplyTargetAssist()
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	UWorld* World = GetWorld();
	if (!OwnerChar || !World) return;

	FCollisionQueryParams Params(FName(TEXT("SEETargetAssist")), false, OwnerChar);
	if (EquippedWeapon) Params.AddIgnoredActor(EquippedWeapon);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, OwnerChar->GetActorLocation(), FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(TargetAssistRange), Params);

	const FVector Forward = OwnerChar->GetActorForwardVector().GetSafeNormal2D();
	const float MinDot = FMath::Cos(FMath::DegreesToRadians(TargetAssistConeHalfAngle));

	// Nearest living pawn inside the frontal cone
	AActor* BestTarget = nullptr;
	float BestDistSq = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OwnerChar) continue;
		if (!Candidate->IsA<APawn>()) continue;

		if (USEEHealthComponent* CandidateHealth = Candidate->FindComponentByClass<USEEHealthComponent>())
		{
			if (CandidateHealth->IsDead()) continue;
		}

		const FVector ToCandidate = Candidate->GetActorLocation() - OwnerChar->GetActorLocation();
		if (FVector::DotProduct(Forward, ToCandidate.GetSafeNormal2D()) < MinDot) continue;

		const float DistSq = ToCandidate.SizeSquared2D();
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			BestTarget = Candidate;
		}
	}

	if (!BestTarget) return;

	// Nudge yaw toward the target so the swing connects naturally
	const FVector ToBest = BestTarget->GetActorLocation() - OwnerChar->GetActorLocation();
	const float DesiredYaw = ToBest.Rotation().Yaw;
	const float CurrentYaw = OwnerChar->GetActorRotation().Yaw;
	const float YawDelta = FMath::Clamp(FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw),
		-TargetAssistMaxYawCorrection, TargetAssistMaxYawCorrection);
	if (FMath::IsNearlyZero(YawDelta, 0.1f)) return;

	FRotator NewRotation = OwnerChar->GetActorRotation();
	NewRotation.Yaw += YawDelta;
	OwnerChar->SetActorRotation(NewRotation);

	// Keep control rotation in sync so controller yaw doesn't immediately undo the assist
	if (AController* Controller = OwnerChar->GetController())
	{
		FRotator ControlRotation = Controller->GetControlRotation();
		ControlRotation.Yaw += YawDelta;
		Controller->SetControlRotation(ControlRotation);
	}
}

void USEECombatComponent::TriggerHitStop()
{
	UWorld* World = GetWorld();
	if (!World || HitStopDuration <= 0.0f) return;
	if (World->GetTimerManager().IsTimerActive(HitStopTimer)) return; // already mid hit-stop

	UGameplayStatics::SetGlobalTimeDilation(World, HitStopTimeDilation);

	// Timers advance in dilated time — scale the duration so the freeze lasts HitStopDuration real seconds
	const float DilatedDuration = FMath::Max(HitStopDuration * HitStopTimeDilation, 0.001f);
	World->GetTimerManager().SetTimer(HitStopTimer, this, &USEECombatComponent::EndHitStop, DilatedDuration, false);
}

void USEECombatComponent::EndHitStop()
{
	if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
	}
}

ASEECharacter* USEECombatComponent::GetOwnerSEECharacter() const
{
	return Cast<ASEECharacter>(GetOwner());
}
