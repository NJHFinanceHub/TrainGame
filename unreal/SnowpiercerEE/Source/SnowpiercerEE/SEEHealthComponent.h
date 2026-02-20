#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEHealthComponent.generated.h"

UENUM(BlueprintType)
enum class ESEEDamageType : uint8
{
	Blunt		UMETA(DisplayName = "Blunt"),
	Bladed		UMETA(DisplayName = "Bladed"),
	Piercing	UMETA(DisplayName = "Piercing"),
	Fire		UMETA(DisplayName = "Fire"),
	Cold		UMETA(DisplayName = "Cold"),
	Electric	UMETA(DisplayName = "Electric"),
	Environmental UMETA(DisplayName = "Environmental")
};

UENUM(BlueprintType)
enum class ESEEInjuryType : uint8
{
	None			UMETA(DisplayName = "None"),
	BrokenArm		UMETA(DisplayName = "Broken Arm"),
	LegWound		UMETA(DisplayName = "Leg Wound"),
	Concussion		UMETA(DisplayName = "Concussion"),
	DeepCut			UMETA(DisplayName = "Deep Cut"),
	InternalBleeding UMETA(DisplayName = "Internal Bleeding")
};

USTRUCT(BlueprintType)
struct FSEEInjury
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEEInjuryType Type = ESEEInjuryType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Severity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresMedStation = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, float, Damage, ESEEDamageType, DamageType, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDowned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRevived);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInjuryApplied, ESEEInjuryType, InjuryType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealthPercent);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEHealthComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float BaseDamage, ESEEDamageType DamageType, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Revive(float HealthPercent = 0.3f);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyInjury(ESEEInjuryType InjuryType, float Severity = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void HealInjury(ESEEInjuryType InjuryType);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void HealAllInjuries();

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDowned() const { return bIsDowned; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool HasInjury(ESEEInjuryType InjuryType) const;

	UFUNCTION(BlueprintPure, Category = "Health")
	TArray<FSEEInjury> GetActiveInjuries() const { return ActiveInjuries; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMeleeDamageModifier() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMoveSpeedModifier() const;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetArmorReduction(ESEEDamageType DamageType) const;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetArmor(float BluntArmor, float BladedArmor, float PiercingArmor);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDamageTaken OnDamageTaken;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDowned OnDowned;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnRevived OnRevived;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnInjuryApplied OnInjuryApplied;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float DownedDuration = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	float BleedingDrainRate = 2.0f;

private:
	TArray<FSEEInjury> ActiveInjuries;
	TMap<ESEEDamageType, float> ArmorValues;
	bool bIsDead = false;
	bool bIsDowned = false;
	float DownedTimer = 0.0f;
};
