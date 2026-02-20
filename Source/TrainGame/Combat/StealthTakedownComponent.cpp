// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "StealthTakedownComponent.h"
#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UStealthTakedownComponent::UStealthTakedownComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStealthTakedownComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStealthTakedownComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CooldownTimer > 0.f)
	{
		CooldownTimer -= DeltaTime;
	}

	if (bIsTakingDown)
	{
		TakedownTimer -= DeltaTime;

		// Check if victim escaped or died during takedown
		if (!CurrentVictim || !IsValid(CurrentVictim))
		{
			CancelTakedown();
			return;
		}

		UCombatComponent* VictimCombat = CurrentVictim->FindComponentByClass<UCombatComponent>();
		if (!VictimCombat || !VictimCombat->IsAlive())
		{
			CancelTakedown();
			return;
		}

		if (TakedownTimer <= 0.f)
		{
			CompleteTakedown();
		}
	}
}

bool UStealthTakedownComponent::AttemptTakedown(EStealthTakedownType Type)
{
	if (bIsTakingDown) return false;
	if (CooldownTimer > 0.f) return false;

	AActor* Target = GetValidTakedownTarget();
	if (!Target)
	{
		return false;
	}

	// Check stealth resistance
	UCombatComponent* TargetCombat = Target->FindComponentByClass<UCombatComponent>();
	if (!TargetCombat) return false;

	// Begin takedown
	CurrentVictim = Target;
	CurrentTakedownType = Type;
	bIsTakingDown = true;
	TakedownTimer = GetTakedownDuration(Type);

	// Lock the victim in place (stagger them)
	TargetCombat->ReceiveAttack(0.f, EAttackDirection::Mid, EDamageType::StealthTakedown, GetOwner());

	OnTakedownStarted.Broadcast(Type, Target);
	return true;
}

AActor* UStealthTakedownComponent::GetValidTakedownTarget() const
{
	AActor* Owner = GetOwner();
	if (!Owner) return nullptr;

	// Find pawns in range
	TArray<AActor*> AllPawns;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), AllPawns);

	float ClosestDist = TakedownRange;
	AActor* BestTarget = nullptr;

	for (AActor* Pawn : AllPawns)
	{
		if (Pawn == Owner) continue;
		if (!IsTargetValidForTakedown(Pawn)) continue;

		float Dist = FVector::Dist(Owner->GetActorLocation(), Pawn->GetActorLocation());
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			BestTarget = Pawn;
		}
	}

	return BestTarget;
}

void UStealthTakedownComponent::CancelTakedown()
{
	if (!bIsTakingDown) return;

	bIsTakingDown = false;
	TakedownTimer = 0.f;
	CooldownTimer = TakedownCooldown;

	// Failed takedown makes noise — alerts nearby enemies
	if (GetOwner())
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), nullptr, GetOwner()->GetActorLocation());
	}

	if (CurrentVictim)
	{
		OnTakedownFailed.Broadcast(CurrentVictim);
	}

	CurrentVictim = nullptr;
}

void UStealthTakedownComponent::CompleteTakedown()
{
	if (!CurrentVictim) return;

	ApplyTakedownEffect();

	OnTakedownCompleted.Broadcast(CurrentTakedownType, CurrentVictim);

	bIsTakingDown = false;
	TakedownTimer = 0.f;
	CooldownTimer = TakedownCooldown;
	CurrentVictim = nullptr;
}

void UStealthTakedownComponent::ApplyTakedownEffect()
{
	if (!CurrentVictim) return;

	UCombatComponent* VictimCombat = CurrentVictim->FindComponentByClass<UCombatComponent>();
	if (!VictimCombat) return;

	switch (CurrentTakedownType)
	{
		case EStealthTakedownType::GrabFromBehind:
			// Grab and hold — stagger the enemy, no damage
			VictimCombat->ReceiveAttack(0.f, EAttackDirection::Mid, EDamageType::NonLethal, GetOwner());
			break;

		case EStealthTakedownType::Chokehold:
			// Non-lethal — downs the target (they can be revived by allies)
			VictimCombat->ReceiveAttack(VictimCombat->GetCurrentHealth() * 0.9f, EAttackDirection::Mid, EDamageType::NonLethal, GetOwner());
			break;

		case EStealthTakedownType::Knockout:
			// Quick knockout — deals enough damage to down but not kill
			VictimCombat->ReceiveAttack(VictimCombat->GetCurrentHealth() - 1.f, EAttackDirection::Mid, EDamageType::NonLethal, GetOwner());
			break;

		case EStealthTakedownType::SilentKill:
			// Lethal — instant kill
			VictimCombat->ReceiveAttack(9999.f, EAttackDirection::Mid, EDamageType::StealthTakedown, GetOwner());
			break;
	}
}

bool UStealthTakedownComponent::IsTargetValidForTakedown(AActor* Target) const
{
	if (!Target) return false;

	// Must be alive
	UCombatComponent* CombatComp = Target->FindComponentByClass<UCombatComponent>();
	if (!CombatComp || !CombatComp->IsAlive()) return false;

	// Must be staggered, unaware, or not in combat stance
	if (CombatComp->GetCurrentStance() == ECombatStance::Attacking ||
		CombatComp->GetCurrentStance() == ECombatStance::Blocking ||
		CombatComp->GetCurrentStance() == ECombatStance::KronoleMode)
	{
		return false;
	}

	// Must be behind the target
	if (!IsTargetBehind(Target)) return false;

	return true;
}

bool UStealthTakedownComponent::IsTargetBehind(AActor* Target) const
{
	AActor* Owner = GetOwner();
	if (!Owner || !Target) return false;

	FVector ToOwner = (Owner->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
	FVector TargetForward = Target->GetActorForwardVector();

	// Dot product: -1 = directly behind, +1 = directly in front
	float Dot = FVector::DotProduct(TargetForward, ToOwner);

	// Negative dot = we're behind the target
	float RequiredDot = -FMath::Cos(FMath::DegreesToRadians(RequiredBehindAngle));
	return Dot < RequiredDot;
}

bool UStealthTakedownComponent::IsTargetUnaware(AActor* Target) const
{
	if (!Target) return false;

	UCombatComponent* CombatComp = Target->FindComponentByClass<UCombatComponent>();
	if (!CombatComp) return false;

	// Target is unaware if in neutral stance (not in combat)
	return CombatComp->GetCurrentStance() == ECombatStance::Neutral;
}

float UStealthTakedownComponent::GetTakedownDuration(EStealthTakedownType Type) const
{
	switch (Type)
	{
		case EStealthTakedownType::GrabFromBehind:	return GrabDuration;
		case EStealthTakedownType::Chokehold:		return ChokeholdDuration;
		case EStealthTakedownType::Knockout:			return KnockoutDuration;
		case EStealthTakedownType::SilentKill:		return SilentKillDuration;
		default:									return 1.f;
	}
}
