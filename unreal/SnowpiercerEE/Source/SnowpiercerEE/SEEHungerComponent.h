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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	// CO-OP: hunger is server-authoritative. The drain in TickComponent and the
	// starving health-drain (TakeDamage, server-gated) run on the server; Eat is
	// authority-gated and already reached server-side via USEEInventoryComponent's
	// ServerUseItem path. CurrentHunger replicates to the OWNING client only
	// (COND_OwnerOnly), where the HUD polls GetHungerPercent() and OnRep fires the
	// same OnHungerChanged delegate the server fires. Standalone is authority:
	// the server path runs, OnRep never fires — single-player is unchanged.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CurrentHunger, Category = "Hunger")
	float CurrentHunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunger")
	float DrainRate = 0.83f; // ~1 point per 2 minutes

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunger")
	float StarvingHealthDrain = 1.0f;

	/** Client feedback: replicated hunger changed — fire OnHungerChanged. */
	UFUNCTION()
	void OnRep_CurrentHunger();
};
