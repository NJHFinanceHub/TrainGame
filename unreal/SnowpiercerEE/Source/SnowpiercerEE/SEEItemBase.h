#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SEEHealthComponent.h"
#include "SEEItemBase.generated.h"

UENUM(BlueprintType)
enum class ESEEItemCategory : uint8
{
	Weapon		UMETA(DisplayName = "Weapon"),
	Armor		UMETA(DisplayName = "Armor"),
	Consumable	UMETA(DisplayName = "Consumable"),
	Crafting	UMETA(DisplayName = "Crafting Material"),
	Quest		UMETA(DisplayName = "Quest Item"),
	Junk		UMETA(DisplayName = "Junk")
};

UENUM(BlueprintType)
enum class ESEEItemRarity : uint8
{
	Common		UMETA(DisplayName = "Common"),
	Uncommon	UMETA(DisplayName = "Uncommon"),
	Rare		UMETA(DisplayName = "Rare"),
	Legendary	UMETA(DisplayName = "Legendary")
};

USTRUCT(BlueprintType)
struct FSEEItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEItemCategory Category = ESEEItemCategory::Junk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEItemRarity Rarity = ESEEItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStackSize = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	// Consumable properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HungerRestore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StaminaRestore = 0.0f;

	// Weapon reference (if Category == Weapon)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> WeaponClass;

	// Armor properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BluntArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BladedArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PiercingArmor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ColdResistance = 0.0f;
};

USTRUCT(BlueprintType)
struct FSEEInventorySlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 0;

	bool IsEmpty() const { return ItemID.IsNone() || Quantity <= 0; }
};
