#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SwimmingComponent.generated.h"

class ACharacter;

UENUM(BlueprintType)
enum class ESwimState : uint8
{
	NotSwimming		UMETA(DisplayName = "Not Swimming"),
	SurfaceSwim		UMETA(DisplayName = "Surface Swimming"),
	Underwater		UMETA(DisplayName = "Underwater"),
	Emerging		UMETA(DisplayName = "Emerging")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSwimStateChanged, ESwimState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBreathChanged, float, BreathPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDrowning);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USwimmingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USwimmingComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Enter water volume
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void EnterWater(float WaterSurfaceZ);

	// Leave water volume
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void ExitWater();

	// Dive underwater
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void StartDive();

	// Surface from underwater
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void Surface();

	// Swimming movement input
	UFUNCTION(BlueprintCallable, Category = "Swimming")
	void SwimInput(FVector Direction);

	UFUNCTION(BlueprintPure, Category = "Swimming")
	bool IsInWater() const { return SwimState != ESwimState::NotSwimming; }

	UFUNCTION(BlueprintPure, Category = "Swimming")
	bool IsUnderwater() const { return SwimState == ESwimState::Underwater; }

	UFUNCTION(BlueprintPure, Category = "Swimming")
	ESwimState GetSwimState() const { return SwimState; }

	UFUNCTION(BlueprintPure, Category = "Swimming")
	float GetBreathPercent() const { return CurrentBreath / MaxBreath; }

	UFUNCTION(BlueprintPure, Category = "Swimming")
	float GetCurrentBreath() const { return CurrentBreath; }

	// Swim speed multiplier (modified by skill tree)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	float SwimSpeedMultiplier = 1.0f;

	// Whether extended swim is unlocked (doubles breath)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Swimming")
	bool bExtendedSwimUnlocked = false;

	UPROPERTY(BlueprintAssignable, Category = "Swimming")
	FOnSwimStateChanged OnSwimStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Swimming")
	FOnBreathChanged OnBreathChanged;

	UPROPERTY(BlueprintAssignable, Category = "Swimming")
	FOnDrowning OnDrowning;

protected:
	virtual void BeginPlay() override;

	// Movement speeds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float SurfaceSwimSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float UnderwaterSwimSpeed = 250.0f;

	// Breath system
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float MaxBreath = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float BreathDrainRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float BreathRecoveryRate = 5.0f;

	// Drowning damage per second when breath is empty
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float DrowningDamageRate = 10.0f;

	// Stamina drain while swimming
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float SwimStaminaDrain = 5.0f;

	// Buoyancy force keeping the player at water surface
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swimming|Config")
	float BuoyancyForce = 200.0f;

private:
	UPROPERTY()
	TWeakObjectPtr<ACharacter> OwnerCharacter;

	ESwimState SwimState = ESwimState::NotSwimming;
	float CurrentBreath;
	float WaterSurfaceHeight = 0.0f;

	void SetSwimState(ESwimState NewState);
	void UpdateSurfaceSwim(float DeltaTime);
	void UpdateUnderwater(float DeltaTime);
	float GetEffectiveMaxBreath() const;
};
