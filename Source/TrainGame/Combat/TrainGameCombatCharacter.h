// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TrainGameCombatCharacter.generated.h"

class UCombatComponent;
class UWeaponComponent;
class URangedCombatComponent;
class UStealthTakedownComponent;

// ============================================================================
// ATrainGameCombatCharacter
//
// Base character for the combat prototype. Both the player and enemies
// derive from this. Combines CombatComponent + WeaponComponent +
// RangedCombatComponent + StealthTakedownComponent for a complete
// combatant in tight train corridors.
// ============================================================================

UCLASS()
class TRAINGAME_API ATrainGameCombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATrainGameCombatCharacter();

	UFUNCTION(BlueprintPure, Category = "Combat")
	UCombatComponent* GetCombatComponent() const { return CombatComp; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UWeaponComponent* GetWeaponComponent() const { return WeaponComp; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	URangedCombatComponent* GetRangedCombatComponent() const { return RangedCombatComp; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	UStealthTakedownComponent* GetStealthComponent() const { return StealthComp; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Ranged")
	URangedCombatComponent* RangedCombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Stealth")
	UStealthTakedownComponent* StealthComp;

private:
	// Input handlers for melee combat
	void Input_AttackHigh();
	void Input_AttackMid();
	void Input_AttackLow();
	void Input_BlockHigh();
	void Input_BlockMid();
	void Input_BlockLow();
	void Input_StopBlock();
	void Input_DodgeLeft();
	void Input_DodgeRight();
	void Input_DodgeBack();
	void Input_ToggleKronoleMode();

	// Input handlers for ranged combat
	void Input_FireRanged();
	void Input_ThrowObject();
	void Input_Reload();

	// Input handlers for stealth
	void Input_StealthTakedown();
	void Input_StealthChokehold();
};
