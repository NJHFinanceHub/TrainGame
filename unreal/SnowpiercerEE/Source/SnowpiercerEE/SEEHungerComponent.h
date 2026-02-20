#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEHungerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHungerChanged, float, HungerPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStarving);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEHungerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEHungerComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Hunger")
	void Eat(float Amount);

	UFUNCTION(BlueprintPure, Category = "Hunger")
	float GetHungerPercent() const { return MaxHunger > 0.0f ? CurrentHunger / MaxHunger : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Hunger")
	float GetCurrentHunger() const { return CurrentHunger; }

	UFUNCTION(BlueprintPure, Category = "Hunger")
	float GetStaminaRegenModifier() const;

	UFUNCTION(BlueprintPure, Category = "Hunger")
	float GetMaxStaminaModifier() const;

	UFUNCTION(BlueprintPure, Category = "Hunger")
	bool IsStarving() const { return CurrentHunger <= 0.0f; }

	UPROPERTY(BlueprintAssignable, Category = "Hunger")
	FOnHungerChanged OnHungerChanged;

	UPROPERTY(BlueprintAssignable, Category = "Hunger")
	FOnStarving OnStarving;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunger")
	float MaxHunger = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hunger")
	float CurrentHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunger")
	float DrainRate = 0.83f; // ~1 point per 2 minutes

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunger")
	float StarvingHealthDrain = 1.0f;
};
