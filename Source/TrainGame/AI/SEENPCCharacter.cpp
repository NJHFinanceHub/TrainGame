// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCCharacter.h"
#include "TrainGame/Combat/CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "TrainGame/Stealth/DetectionComponent.h"
#include "NPCScheduleComponent.h"
#include "BodyDiscoveryComponent.h"

ASEENPCCharacter::ASEENPCCharacter()
{
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComp"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComp"));
	DetectionComp = CreateDefaultSubobject<UDetectionComponent>(TEXT("DetectionComp"));
	ScheduleComp = CreateDefaultSubobject<UNPCScheduleComponent>(TEXT("ScheduleComp"));
	BodyDiscoveryComp = CreateDefaultSubobject<UBodyDiscoveryComponent>(TEXT("BodyDiscoveryComp"));
}

void ASEENPCCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASEENPCCharacter::SetBodyState(EBodyState NewState)
{
	if (BodyState == NewState)
	{
		return;
	}

	EBodyState OldState = BodyState;
	BodyState = NewState;
	OnBodyStateChanged.Broadcast(OldState, NewState);
}

bool ASEENPCCharacter::IsIncapacitated() const
{
	return BodyState == EBodyState::Unconscious
		|| BodyState == EBodyState::Dead
		|| BodyState == EBodyState::Restrained;
}

bool ASEENPCCharacter::IsAlive() const
{
	return BodyState != EBodyState::Dead;
}
