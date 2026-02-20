#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEStatsComponent.generated.h"

UENUM(BlueprintType)
enum class ESEEStat : uint8
{
	Strength		UMETA(DisplayName = "Strength"),
	Agility			UMETA(DisplayName = "Agility"),
	Endurance		UMETA(DisplayName = "Endurance"),
	Cunning			UMETA(DisplayName = "Cunning"),
	Perception		UMETA(DisplayName = "Perception"),
	Charisma		UMETA(DisplayName = "Charisma")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatChanged, ESEEStat, Stat, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelUp, int32, NewLevel, int32, SkillPointsAvailable);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEStatsComponent();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetStat(ESEEStat Stat) const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStat(ESEEStat Stat, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ModifyStat(ESEEStat Stat, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetStatModifier(ESEEStat Stat) const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void AddXP(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetLevel() const { return CurrentLevel; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCurrentXP() const { return CurrentXP; }

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetXPToNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetSkillPoints() const { return AvailableSkillPoints; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool SpendSkillPoint(ESEEStat Stat);

	UFUNCTION(BlueprintPure, Category = "Stats")
	bool PassesSkillCheck(ESEEStat Stat, int32 Difficulty) const;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnStatChanged OnStatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stats")
	FOnLevelUp OnLevelUp;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<ESEEStat, int32> BaseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	TMap<ESEEStat, int32> StatModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 CurrentXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 AvailableSkillPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 MaxStatValue = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	int32 XPPerLevel = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float XPScaleFactor = 1.5f;
};
