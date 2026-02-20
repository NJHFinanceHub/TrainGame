// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Core/CombatTypes.h"
#include "StealthTakedownComponent.generated.h"

class UCombatComponent;

// ============================================================================
// UStealthTakedownComponent
//
// Enables stealth takedowns: grab from behind, chokehold, knockout.
// In Snowpiercer's corridors, approaching from behind requires timing
// and environmental awareness. Guards patrol; windows of opportunity
// are brief. Failed takedowns alert the target and nearby enemies.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownStarted, EStealthTakedownType, Type, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakedownCompleted, EStealthTakedownType, Type, AActor*, Victim);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakedownFailed, AActor*, Target);

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UStealthTakedownComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthTakedownComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Attempt a stealth takedown on the nearest valid target */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stealth")
	bool AttemptTakedown(EStealthTakedownType Type);

	/** Check if any valid target is in takedown range and position */
	UFUNCTION(BlueprintPure, Category = "Combat|Stealth")
	AActor* GetValidTakedownTarget() const;

	/** Check if we're currently performing a takedown */
	UFUNCTION(BlueprintPure, Category = "Combat|Stealth")
	bool IsPerformingTakedown() const { return bIsTakingDown; }

	/** Cancel an in-progress takedown (if interrupted by damage) */
	UFUNCTION(BlueprintCallable, Category = "Combat|Stealth")
	void CancelTakedown();

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnTakedownStarted OnTakedownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnTakedownCompleted OnTakedownCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Stealth")
	FOnTakedownFailed OnTakedownFailed;

protected:
	virtual void BeginPlay() override;

	/** Max distance to attempt takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float TakedownRange = 150.f;

	/** Required angle behind the target (degrees from their back) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float RequiredBehindAngle = 60.f;

	/** Duration of grab-from-behind takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float GrabDuration = 0.5f;

	/** Duration of chokehold takedown (non-lethal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float ChokeholdDuration = 3.f;

	/** Duration of knockout takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float KnockoutDuration = 0.8f;

	/** Duration of silent kill takedown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SilentKillDuration = 1.5f;

	/** Cooldown between takedown attempts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float TakedownCooldown = 2.f;

	/** Noise radius when a takedown fails (alerts nearby enemies) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float FailedTakedownNoiseRadius = 800.f;

	/** Noise radius when a takedown succeeds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth")
	float SuccessfulTakedownNoiseRadius = 200.f;

private:
	bool IsTargetValidForTakedown(AActor* Target) const;
	bool IsTargetBehind(AActor* Target) const;
	bool IsTargetUnaware(AActor* Target) const;
	float GetTakedownDuration(EStealthTakedownType Type) const;
	void CompleteTakedown();
	void ApplyTakedownEffect();

	UPROPERTY()
	AActor* CurrentVictim = nullptr;

	EStealthTakedownType CurrentTakedownType = EStealthTakedownType::Knockout;
	bool bIsTakingDown = false;
	float TakedownTimer = 0.f;
	float CooldownTimer = 0.f;
};
