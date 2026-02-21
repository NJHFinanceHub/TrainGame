// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EnvironmentalHazardComponent.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"

UEnvironmentalHazardComponent::UEnvironmentalHazardComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UEnvironmentalHazardComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UEnvironmentalHazardComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Cooldown
	if (CooldownTimer > 0.f)
	{
		CooldownTimer -= DeltaTime;
	}

	// Active hazard applies damage over time
	if (bIsHazardActive)
	{
		ActiveTimer -= DeltaTime;

		if (bDamageOverTime)
		{
			TArray<AActor*> VictimsInZone = GetActorsInHazardZone();
			for (AActor* Victim : VictimsInZone)
			{
				UCombatComponent* CombatComp = Victim->FindComponentByClass<UCombatComponent>();
				if (CombatComp && CombatComp->IsAlive())
				{
					EDamageType DmgType = GetDamageTypeForHazard();
					CombatComp->ReceiveAttack(DamagePerSecond * DeltaTime, EAttackDirection::Mid, DmgType, GetOwner());
				}
			}
		}

		if (ActiveTimer <= 0.f)
		{
			bIsHazardActive = false;
			if (!bOneShot)
			{
				CooldownTimer = Cooldown;
			}
		}
	}
}

void UEnvironmentalHazardComponent::TriggerHazard(AActor* Instigator)
{
	if (!CanTrigger()) return;

	bIsHazardActive = true;
	bHasBeenTriggered = true;
	ActiveTimer = ActiveDuration;

	// Apply immediate effects to everyone in zone
	TArray<AActor*> Victims = GetActorsInHazardZone();
	for (AActor* Victim : Victims)
	{
		// Don't damage the instigator (player triggered it intentionally)
		if (Victim == Instigator) continue;

		ApplyHazardEffect(Victim, Instigator);
		OnHazardTriggered.Broadcast(HazardType, Victim);
	}
}

bool UEnvironmentalHazardComponent::IsActorInHazardZone(AActor* Actor) const
{
	if (!Actor || !GetOwner()) return false;

	float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
	return Distance <= HazardRadius;
}

bool UEnvironmentalHazardComponent::CanTrigger() const
{
	if (bOneShot && bHasBeenTriggered) return false;
	if (bIsHazardActive) return false;
	if (CooldownTimer > 0.f) return false;
	return true;
}

void UEnvironmentalHazardComponent::ApplyHazardEffect(AActor* Victim, AActor* Instigator)
{
	if (!Victim) return;

	UCombatComponent* CombatComp = Victim->FindComponentByClass<UCombatComponent>();
	if (!CombatComp || !CombatComp->IsAlive()) return;

	EDamageType DmgType = GetDamageTypeForHazard();

	if (bInstantKill)
	{
		// Window breach: instant death (sucked into frozen void)
		CombatComp->ReceiveAttack(9999.f, EAttackDirection::Mid, DmgType, Instigator);
	}
	else
	{
		// Apply base damage
		CombatComp->ReceiveAttack(BaseDamage, EAttackDirection::Mid, DmgType, Instigator);
	}

	// Apply knockback
	if (bAppliesKnockback)
	{
		ApplyKnockback(Victim);
	}
}

void UEnvironmentalHazardComponent::ApplyKnockback(AActor* Victim) const
{
	ACharacter* VictimChar = Cast<ACharacter>(Victim);
	if (!VictimChar) return;

	// Transform knockback direction from hazard local space to world space
	FVector WorldKnockback = GetOwner()->GetActorTransform().TransformVector(KnockbackDirection);
	WorldKnockback.Normalize();
	VictimChar->LaunchCharacter(WorldKnockback * KnockbackForce, true, true);
}

EDamageType UEnvironmentalHazardComponent::GetDamageTypeForHazard() const
{
	switch (HazardType)
	{
		case EEnvironmentalHazard::SteamVent:
			return EDamageType::Thermal;
		case EEnvironmentalHazard::ElectricalPanel:
			return EDamageType::Electrical;
		case EEnvironmentalHazard::WindowBreach:
			return EDamageType::Cold;
		case EEnvironmentalHazard::FlammableLeak:
			return EDamageType::Thermal;
		case EEnvironmentalHazard::PressureValve:
			return EDamageType::Physical;
		default:
			return EDamageType::Physical;
	}
}

TArray<AActor*> UEnvironmentalHazardComponent::GetActorsInHazardZone() const
{
	TArray<AActor*> Results;
	if (!GetOwner()) return Results;

	// Overlap sphere to find actors in zone
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetOwner()->GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(HazardRadius),
		Params
	);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* Actor = Overlap.GetActor())
		{
			Results.AddUnique(Actor);
		}
	}

	return Results;
}
