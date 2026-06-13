#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEEColdComponent.generated.h"

UENUM(BlueprintType)
enum class ESEEFrostbiteStage : uint8
{
	None		UMETA(DisplayName = "None"),
	Shivers		UMETA(DisplayName = "Shivers"),
	Numbness	UMETA(DisplayName = "Numbness"),
	Blackout	UMETA(DisplayName = "Blackout")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFrostbiteStageChanged, ESEEFrostbiteStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTemperatureChanged, float, Temperature);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEColdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEEColdComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Cold")
	void EnterColdZone(float ZoneTemperature = -30.0f);

	UFUNCTION(BlueprintCallable, Category = "Cold")
	void ExitColdZone();

	UFUNCTION(BlueprintCallable, Category = "Cold")
	void SetNearFireSource(bool bNearFire);

	UFUNCTION(BlueprintCallable, Category = "Cold")
	void SetColdSuitBonus(float Bonus);

	UFUNCTION(BlueprintPure, Category = "Cold")
	float GetTemperature() const { return CurrentTemperature; }

	UFUNCTION(BlueprintPure, Category = "Cold")
	ESEEFrostbiteStage GetFrostbiteStage() const { return CurrentStage; }

	UFUNCTION(BlueprintPure, Category = "Cold")
	bool IsInColdZone() const { return bInColdZone; }

	UFUNCTION(BlueprintPure, Category = "Cold")
	float GetMoveSpeedModifier() const;

	UPROPERTY(BlueprintAssignable, Category = "Cold")
	FOnFrostbiteStageChanged OnFrostbiteStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Cold")
	FOnTemperatureChanged OnTemperatureChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float BodyTemperature = 37.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float ShiverThreshold = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float NumbnessThreshold = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float BlackoutThreshold = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float CoolingRate = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float WarmingRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cold")
	float BlackoutHealthDrain = 10.0f;

private:
	// CO-OP: cold/exposure is server-authoritative. The cooling/warming math and
	// the Blackout health-drain (TakeDamage, itself server-gated) run on the server
	// in TickComponent; CurrentTemperature + CurrentStage replicate to the OWNING
	// client only (COND_OwnerOnly), where the HUD polls GetTemperature() and the
	// OnReps fire the same OnTemperatureChanged / OnFrostbiteStageChanged delegates
	// the server fires, so each client's cold UI + damage cues are correct.
	// Standalone is authority: the server path runs, OnReps never fire — the
	// single-player feel is byte-for-byte unchanged.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTemperature)
	float CurrentTemperature = 37.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentStage)
	ESEEFrostbiteStage CurrentStage = ESEEFrostbiteStage::None;

	float ZoneTemperature = -30.0f;
	float ColdSuitBonus = 0.0f;
	bool bInColdZone = false;
	bool bNearFire = false;

	/** Client feedback: replicated temperature changed — fire OnTemperatureChanged. */
	UFUNCTION()
	void OnRep_CurrentTemperature();

	/** Client feedback: replicated frostbite stage changed — fire OnFrostbiteStageChanged. */
	UFUNCTION()
	void OnRep_CurrentStage();

	void UpdateFrostbiteStage();
};
