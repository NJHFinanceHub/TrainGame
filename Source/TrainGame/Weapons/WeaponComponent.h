// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "WeaponComponent.generated.h"

// ============================================================================
// UWeaponComponent
//
// Manages equipped weapon state, degradation, and swapping.
// Weapons break with use, lose effectiveness as durability drops,
// and must be repaired or replaced. Improvised weapons from the Tail
// break fast; military-grade from the Armory last much longer.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, const FWeaponStats&, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDurabilityChanged, float, Current, float, Max);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	/** Equip a weapon by stats */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EquipWeapon(const FWeaponStats& NewWeapon);

	/** Unequip current weapon (go unarmed) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void UnequipWeapon();

	/** Apply durability damage from landing a hit */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyHitDurabilityLoss();

	/** Apply durability damage from having attack blocked */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ApplyBlockDurabilityLoss();

	/** Repair weapon by a fixed amount (uses repair materials) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void RepairWeapon(float RepairAmount);

	/** Get current weapon stats */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FWeaponStats& GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeaponEquipped() const { return bHasWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsWeaponBroken() const { return bHasWeapon && CurrentWeapon.IsBroken(); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetDurabilityPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponBroken OnWeaponBroken;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnDurabilityChanged OnDurabilityChanged;

	// --- Preset Weapon Factory Methods ---

	/** Create a pipe club (Tier 1 - Tail) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakePipeClub();

	/** Create a shiv (Tier 1 - Tail) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeShiv();

	/** Create a reinforced axe (Tier 2 - Third Class) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeReinforcedAxe();

	/** Create a nail bat (Tier 1 - Tail) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeNailBat();

	/** Create a Jackboot baton (Tier 3 - Military) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeJackbootBaton();

	/** Create a crossbow (Tier 2 - Ranged) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeCrossbow();

	/** Create an improvised firearm (Tier 2 - Ranged) */
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeImprovisedFirearm();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bHasWeapon = false;

private:
	void HandleWeaponBreak();
};
