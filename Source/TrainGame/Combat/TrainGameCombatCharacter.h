// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TrainGameCombatCharacter.generated.h"

class UCombatComponent;
class UWeaponComponent;

// ============================================================================
// ATrainGameCombatCharacter
//
// Base character for the combat prototype. Both the player and enemies
// derive from this. Combines CombatComponent + WeaponComponent for a
// complete combatant in tight train corridors.
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

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UWeaponComponent* WeaponComp;

private:
	// Input handlers for combat
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
};
