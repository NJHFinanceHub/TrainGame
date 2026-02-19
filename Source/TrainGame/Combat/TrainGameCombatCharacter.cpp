// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "TrainGameCombatCharacter.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "Components/InputComponent.h"

ATrainGameCombatCharacter::ATrainGameCombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
}

void ATrainGameCombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Weapon degradation is handled in the input handlers (Input_AttackHigh/Mid/Low)
	// where we check the hit result and apply durability loss accordingly.
	// A full implementation would use a delegate-based approach.
}

void ATrainGameCombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind combat actions
	// These use the legacy input system for the prototype.
	// Full game will use Enhanced Input (UE5).
	PlayerInputComponent->BindAction("AttackHigh", IE_Pressed, this, &ATrainGameCombatCharacter::Input_AttackHigh);
	PlayerInputComponent->BindAction("AttackMid", IE_Pressed, this, &ATrainGameCombatCharacter::Input_AttackMid);
	PlayerInputComponent->BindAction("AttackLow", IE_Pressed, this, &ATrainGameCombatCharacter::Input_AttackLow);

	PlayerInputComponent->BindAction("BlockHigh", IE_Pressed, this, &ATrainGameCombatCharacter::Input_BlockHigh);
	PlayerInputComponent->BindAction("BlockMid", IE_Pressed, this, &ATrainGameCombatCharacter::Input_BlockMid);
	PlayerInputComponent->BindAction("BlockLow", IE_Pressed, this, &ATrainGameCombatCharacter::Input_BlockLow);
	PlayerInputComponent->BindAction("BlockHigh", IE_Released, this, &ATrainGameCombatCharacter::Input_StopBlock);
	PlayerInputComponent->BindAction("BlockMid", IE_Released, this, &ATrainGameCombatCharacter::Input_StopBlock);
	PlayerInputComponent->BindAction("BlockLow", IE_Released, this, &ATrainGameCombatCharacter::Input_StopBlock);

	PlayerInputComponent->BindAction("DodgeLeft", IE_Pressed, this, &ATrainGameCombatCharacter::Input_DodgeLeft);
	PlayerInputComponent->BindAction("DodgeRight", IE_Pressed, this, &ATrainGameCombatCharacter::Input_DodgeRight);
	PlayerInputComponent->BindAction("DodgeBack", IE_Pressed, this, &ATrainGameCombatCharacter::Input_DodgeBack);

	PlayerInputComponent->BindAction("KronoleMode", IE_Pressed, this, &ATrainGameCombatCharacter::Input_ToggleKronoleMode);
}

void ATrainGameCombatCharacter::Input_AttackHigh()
{
	if (CombatComp)
	{
		FHitResult_Combat Result = CombatComp->PerformAttack(EAttackDirection::High);
		if (Result.bHit && WeaponComp && WeaponComp->HasWeaponEquipped())
		{
			if (Result.bBlocked)
				WeaponComp->ApplyBlockDurabilityLoss();
			else
				WeaponComp->ApplyHitDurabilityLoss();
		}
	}
}

void ATrainGameCombatCharacter::Input_AttackMid()
{
	if (CombatComp)
	{
		FHitResult_Combat Result = CombatComp->PerformAttack(EAttackDirection::Mid);
		if (Result.bHit && WeaponComp && WeaponComp->HasWeaponEquipped())
		{
			if (Result.bBlocked)
				WeaponComp->ApplyBlockDurabilityLoss();
			else
				WeaponComp->ApplyHitDurabilityLoss();
		}
	}
}

void ATrainGameCombatCharacter::Input_AttackLow()
{
	if (CombatComp)
	{
		FHitResult_Combat Result = CombatComp->PerformAttack(EAttackDirection::Low);
		if (Result.bHit && WeaponComp && WeaponComp->HasWeaponEquipped())
		{
			if (Result.bBlocked)
				WeaponComp->ApplyBlockDurabilityLoss();
			else
				WeaponComp->ApplyHitDurabilityLoss();
		}
	}
}

void ATrainGameCombatCharacter::Input_BlockHigh()
{
	if (CombatComp) CombatComp->StartBlock(EBlockDirection::High);
}

void ATrainGameCombatCharacter::Input_BlockMid()
{
	if (CombatComp) CombatComp->StartBlock(EBlockDirection::Mid);
}

void ATrainGameCombatCharacter::Input_BlockLow()
{
	if (CombatComp) CombatComp->StartBlock(EBlockDirection::Low);
}

void ATrainGameCombatCharacter::Input_StopBlock()
{
	if (CombatComp) CombatComp->StopBlock();
}

void ATrainGameCombatCharacter::Input_DodgeLeft()
{
	if (CombatComp) CombatComp->PerformDodge(-GetActorRightVector());
}

void ATrainGameCombatCharacter::Input_DodgeRight()
{
	if (CombatComp) CombatComp->PerformDodge(GetActorRightVector());
}

void ATrainGameCombatCharacter::Input_DodgeBack()
{
	if (CombatComp) CombatComp->PerformDodge(-GetActorForwardVector());
}

void ATrainGameCombatCharacter::Input_ToggleKronoleMode()
{
	if (!CombatComp) return;

	if (CombatComp->IsInKronoleMode())
		CombatComp->DeactivateKronoleMode();
	else
		CombatComp->ActivateKronoleMode();
}
