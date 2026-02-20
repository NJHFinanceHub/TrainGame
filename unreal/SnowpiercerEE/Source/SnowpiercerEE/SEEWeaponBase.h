#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEHealthComponent.h"
#include "SEEWeaponBase.generated.h"

UENUM(BlueprintType)
enum class ESEEWeaponTier : uint8
{
	Improvised		UMETA(DisplayName = "Improvised"),
	Functional		UMETA(DisplayName = "Functional"),
	Military		UMETA(DisplayName = "Military"),
	Specialized		UMETA(DisplayName = "Specialized"),
	Legendary		UMETA(DisplayName = "Legendary")
};

UCLASS()
class SNOWPIERCEREE_API ASEEWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	ASEEWeaponBase();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetBaseDamage() const { return BaseDamage; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	ESEEDamageType GetDamageType() const { return DamageType; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAttackSpeed() const { return AttackSpeed; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetWeaponRange() const { return Range; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetDurabilityPercent() const { return MaxDurability > 0.0f ? CurrentDurability / MaxDurability : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsBroken() const { return CurrentDurability <= 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DegradeDurability(float Amount = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void Repair(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ESEEWeaponTier Tier = ESEEWeaponTier::Improvised;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	ESEEDamageType DamageType = ESEEDamageType::Blunt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Range = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float Weight = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float MaxDurability = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	float CurrentDurability = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float StaminaCostLight = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float StaminaCostHeavy = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	float HeavyDamageMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

protected:
	virtual void BeginPlay() override;
};
