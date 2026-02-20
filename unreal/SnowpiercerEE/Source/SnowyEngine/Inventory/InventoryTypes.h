// InventoryTypes.h - Item and inventory type definitions
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InventoryTypes.generated.h"

// Category of item for filtering and slot assignment
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	Armor		UMETA(DisplayName = "Armor"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Crafting	UMETA(DisplayName = "Crafting Material"),
	Quest		UMETA(DisplayName = "Quest Item"),
	Tool		UMETA(DisplayName = "Tool"),
	Disguise	UMETA(DisplayName = "Disguise"),
	Ammo		UMETA(DisplayName = "Ammunition"),
	Lore		UMETA(DisplayName = "Lore/Collectible"),
	Misc		UMETA(DisplayName = "Miscellaneous")
};

// Rarity tier — affects value, crafting requirements, and visual indicators
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	VeryRare	UMETA(DisplayName = "Very Rare"),
	Legendary	UMETA(DisplayName = "Legendary")
};

// Weapon damage types for combat calculations
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Blunt		UMETA(DisplayName = "Blunt"),
	Bladed		UMETA(DisplayName = "Bladed"),
	Piercing	UMETA(DisplayName = "Piercing"),
	Explosive	UMETA(DisplayName = "Explosive"),
	Electric	UMETA(DisplayName = "Electric"),
	Cold		UMETA(DisplayName = "Cold"),
	Poison		UMETA(DisplayName = "Poison")
};

// Static item definition loaded from data tables
USTRUCT(BlueprintType)
struct FItemDefinition : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemCategory Category = EItemCategory::Misc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EItemRarity Rarity = EItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsQuestItem = false;

	// Weapon-specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Weapon"))
	float BaseDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Weapon"))
	EDamageType PrimaryDamageType = EDamageType::Blunt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Weapon"))
	float MaxDurability = 100.0f;

	// Armor-specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Armor"))
	float DamageReduction = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Armor"))
	float ColdResistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Armor"))
	float NoiseMultiplier = 1.0f;

	// Consumable-specific
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Consumable"))
	float HungerRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="Category==EItemCategory::Consumable"))
	float HealthRestore = 0.0f;

	// Icon/mesh references (soft pointers to avoid hard dependencies)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> WorldMesh;
};

// Runtime instance of an item in inventory (tracks durability, stack count, etc.)
USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	// Unique instance ID for this item stack
	UPROPERTY(BlueprintReadOnly)
	FGuid InstanceID;

	// Reference to the static definition
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	// Current stack count
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 1;

	// Current durability (for weapons/armor)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentDurability = -1.0f; // -1 = not applicable

	FInventoryItem()
	{
		InstanceID = FGuid::NewGuid();
	}

	bool IsValid() const { return !ItemID.IsNone() && StackCount > 0; }
};

// Delegate for inventory changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAdded, const FInventoryItem&, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, const FInventoryItem&, Item);
