// ArmorComponent.h - Equipped armor management: slots, damage reduction, cold resistance, noise
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowyEngine/Inventory/InventoryTypes.h"
#include "ArmorComponent.generated.h"

/**
 * Armor equipment slots. Head, Torso, and Shield.
 * Shields provide active block bonuses rather than passive DR.
 */
UENUM(BlueprintType)
enum class EArmorSlot : uint8
{
	Head	UMETA(DisplayName = "Head"),
	Torso	UMETA(DisplayName = "Torso"),
	Shield	UMETA(DisplayName = "Shield")
};

/** Noise level categories for stealth interaction */
UENUM(BlueprintType)
enum class EArmorNoiseLevel : uint8
{
	Silent	UMETA(DisplayName = "Silent"),
	Quiet	UMETA(DisplayName = "Quiet"),
	Normal	UMETA(DisplayName = "Normal"),
	Loud	UMETA(DisplayName = "Loud")
};

/** Runtime state of an equipped armor piece */
USTRUCT(BlueprintType)
struct FEquippedArmor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ArmorItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EArmorSlot Slot = EArmorSlot::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageReduction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColdResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EArmorNoiseLevel NoiseLevel = EArmorNoiseLevel::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDurability = 100.0f;

	/** Shield block bonus (only for Shield slot) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BlockBonus = 0.0f;

	bool IsValid() const { return !ArmorItemID.IsNone(); }
	bool IsBroken() const { return CurrentDurability <= 0.0f; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArmorEquipped, EArmorSlot, Slot, const FEquippedArmor&, Armor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmorUnequipped, EArmorSlot, Slot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnArmorDamaged, EArmorSlot, Slot, float, RemainingDurability);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnArmorBroken, EArmorSlot, Slot);

/**
 * UArmorComponent
 *
 * Manages the 3 armor equipment slots (Head, Torso, Shield).
 * Calculates total damage reduction, cold resistance, and noise multiplier
 * from equipped pieces. Armor degrades on hits received.
 *
 * Shields provide active block bonuses rather than passive DR.
 * Heavy armor (>7kg torso) reduces dodge speed.
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UArmorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UArmorComponent();

	// --- Equip / Unequip ---

	/** Equip an armor piece into the appropriate slot. Returns the previously equipped piece (if any). */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	bool EquipArmor(const FEquippedArmor& Armor);

	/** Unequip armor from a specific slot. */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	FEquippedArmor UnequipArmor(EArmorSlot Slot);

	/** Get the armor piece in a specific slot. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	bool GetArmorInSlot(EArmorSlot Slot, FEquippedArmor& OutArmor) const;

	/** Check if a slot has armor equipped. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	bool HasArmorInSlot(EArmorSlot Slot) const;

	// --- Damage & Durability ---

	/** Apply durability damage to armor when hit. Returns DR% applied. */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	float ApplyHitToArmor(EArmorSlot Slot, float DurabilityLoss);

	/** Repair armor in a slot. */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void RepairArmor(EArmorSlot Slot, float RepairAmount);

	// --- Aggregate Stats ---

	/** Get total passive damage reduction from Head + Torso armor. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetTotalDamageReduction() const;

	/** Get shield block bonus (active blocking only). */
	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetShieldBlockBonus() const;

	/** Get total cold resistance from all equipped armor. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetTotalColdResistance() const;

	/** Get worst noise level across all equipped armor. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	EArmorNoiseLevel GetWorstNoiseLevel() const;

	/** Get noise multiplier for stealth detection (Silent=0.5, Quiet=0.75, Normal=1.0, Loud=1.5). */
	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetNoiseMultiplier() const;

	/** Get total armor weight across all slots. */
	UFUNCTION(BlueprintPure, Category = "Armor")
	float GetTotalArmorWeight() const;

	/** Check if torso armor is heavy enough to penalize dodge speed (>7kg). */
	UFUNCTION(BlueprintPure, Category = "Armor")
	bool HasHeavyArmorPenalty() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Armor")
	FOnArmorEquipped OnArmorEquipped;

	UPROPERTY(BlueprintAssignable, Category = "Armor")
	FOnArmorUnequipped OnArmorUnequipped;

	UPROPERTY(BlueprintAssignable, Category = "Armor")
	FOnArmorDamaged OnArmorDamaged;

	UPROPERTY(BlueprintAssignable, Category = "Armor")
	FOnArmorBroken OnArmorBroken;

protected:
	/** Heavy armor weight threshold for dodge penalty (kg) */
	UPROPERTY(EditAnywhere, Category = "Armor|Config")
	float HeavyArmorThreshold = 7.0f;

	/** Dodge speed penalty multiplier when wearing heavy armor */
	UPROPERTY(EditAnywhere, Category = "Armor|Config")
	float HeavyArmorDodgePenalty = 0.85f;

private:
	/** Armor slots: Head, Torso, Shield */
	TMap<EArmorSlot, FEquippedArmor> EquippedArmor;
};
