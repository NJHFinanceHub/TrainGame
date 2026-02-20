// SEECombatComponent.h
// Snowpiercer: Eternal Engine - Player combat: melee, ranged, blocking, combos, stamina

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEDamageTypes.h"
#include "SEECombatComponent.generated.h"

class ASEEWeaponBase;

/** Combat stance */
UENUM(BlueprintType)
enum class ESEECombatStance : uint8
{
	None       UMETA(DisplayName = "None"),
	Melee      UMETA(DisplayName = "Melee"),
	Ranged     UMETA(DisplayName = "Ranged"),
	Blocking   UMETA(DisplayName = "Blocking"),
	Dodging    UMETA(DisplayName = "Dodging")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, const FSEEDamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComboAdvanced, int32, ComboCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnComboReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEquipped, ASEEWeaponBase*, Weapon, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStanceChanged, ESEECombatStance, NewStance);

/**
 * Combat component for player and NPC characters.
 * Handles melee attacks, blocking, dodging, damage processing,
 * weapon slots, combo tracking, and stamina management.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEECombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEECombatComponent();

	// --- Health ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Health")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Health")
	bool bIsDead = false;

	// --- Stamina ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float MaxStamina = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Stamina")
	float CurrentStamina = 100.0f;

	/** Stamina regenerated per second when not attacking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float StaminaRegenRate = 15.0f;

	/** Delay in seconds before stamina starts regenerating after use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stamina")
	float StaminaRegenDelay = 1.0f;

	// --- Blocking ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	float BlockDamageReduction = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Block")
	float BlockStaminaCostPerHit = 15.0f;

	// --- Dodge ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeStaminaCost = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Dodge")
	float DodgeCooldown = 0.5f;

	// --- Combo ---

	UPROPERTY(BlueprintReadOnly, Category = "Combat|Combo")
	int32 CurrentComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	int32 MaxComboCount = 5;

	/** Time window to continue combo after a hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	float ComboWindowSeconds = 1.5f;

	/** Bonus damage per combo step (additive) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	float ComboDamageBonus = 0.1f;

	// --- Armor ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Armor")
	FSEEArmorData Armor;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	ESEECombatStance CurrentStance = ESEECombatStance::None;

	// --- Weapon Slots ---

	/** Primary weapon slot */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapons")
	ASEEWeaponBase* PrimaryWeapon = nullptr;

	/** Secondary weapon slot */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapons")
	ASEEWeaponBase* SecondaryWeapon = nullptr;

	/** Currently active weapon (primary or secondary) */
	UPROPERTY(BlueprintReadOnly, Category = "Combat|Weapons")
	ASEEWeaponBase* ActiveWeapon = nullptr;

	// --- Interface ---

	/** Perform a melee attack in the given direction */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void MeleeAttack(ESEEAttackDirection Direction);

	/** Begin blocking */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock();

	/** Stop blocking */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	/** Perform a dodge */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Dodge();

	/** Process incoming damage (applies armor, blocking, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TakeDamage(FSEEDamageInfo DamageInfo);

	/** Equip weapon to primary slot (0) or secondary slot (1) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapons")
	void EquipWeapon(ASEEWeaponBase* Weapon, int32 SlotIndex);

	/** Switch active weapon between primary and secondary */
	UFUNCTION(BlueprintCallable, Category = "Combat|Weapons")
	void SwitchWeapon();

	/** Check if there is enough stamina for an action */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	bool HasStamina(float Cost) const;

	/** Get health as 0-1 ratio */
	UFUNCTION(BlueprintPure, Category = "Combat|Health")
	float GetHealthPercent() const;

	/** Get stamina as 0-1 ratio */
	UFUNCTION(BlueprintPure, Category = "Combat|Stamina")
	float GetStaminaPercent() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnComboAdvanced OnComboAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnComboReset OnComboReset;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnWeaponEquipped OnWeaponEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnStanceChanged OnStanceChanged;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void ConsumeStamina(float Amount);
	void ResetCombo();
	void AdvanceCombo();
	float ApplyArmor(float RawDamage, ESEEDamageType Type) const;
	void SetStance(ESEECombatStance NewStance);

	float TimeSinceLastStaminaUse = 0.0f;
	float TimeSinceLastComboHit = 0.0f;
	float DodgeCooldownRemaining = 0.0f;
};
