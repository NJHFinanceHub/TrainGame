// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "WeaponComponent.generated.h"

// ============================================================================
// UWeaponComponent
//
// Manages equipped weapon state, degradation, ammo, and swapping.
// Weapons break with use, lose effectiveness as durability drops,
// and must be repaired or replaced. Ranged weapons consume ammo.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEquipped, const FWeaponStats&, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponBroken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDurabilityChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChanged, int32, Current, int32, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAmmoEmpty);

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

	/** Consume one round of ammo (for ranged weapons) */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool ConsumeAmmo();

	/** Add ammo to current weapon */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AddAmmo(int32 Amount);

	/** Get current weapon stats */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FWeaponStats& GetCurrentWeapon() const { return CurrentWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasWeaponEquipped() const { return bHasWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsWeaponBroken() const { return bHasWeapon && CurrentWeapon.IsBroken(); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsRangedWeapon() const { return bHasWeapon && CurrentWeapon.IsRanged(); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool HasAmmo() const { return bHasWeapon && CurrentWeapon.HasAmmo(); }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetDurabilityPercent() const;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponBroken OnWeaponBroken;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnDurabilityChanged OnDurabilityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnAmmoEmpty OnAmmoEmpty;

	// --- Preset Weapon Factory Methods ---

	// Tail (Improvised)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakePipeClub();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeShiv();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeNailBat();

	// Third Class (Functional)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeReinforcedAxe();

	// Military (Jackboot)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeJackbootBaton();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeOfficerSword();

	// Ranged
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeCrossbow();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeImprovisedFirearm();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeFirstClassPistol();

	// Thrown
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeThrowingKnife();

	// Specialized (Enemy-unique)
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeZealotBlade();
	UFUNCTION(BlueprintCallable, Category = "Weapon|Presets")
	static FWeaponStats MakeElectricProd();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	FWeaponStats CurrentWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	bool bHasWeapon = false;

private:
	void HandleWeaponBreak();
};
