// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SEEBossController.generated.h"

class ASEEBossGrey;

/** AI states for Commander Grey's behavior */
UENUM(BlueprintType)
enum class EGreyAIState : uint8
{
	Commanding		UMETA(DisplayName = "Commanding"),     // Phase 1: at command post
	EngagingRanged	UMETA(DisplayName = "Engaging Ranged"), // Phase 2: cover-to-cover
	MeleeAssault	UMETA(DisplayName = "Melee Assault"),   // Phase 3: aggressive pursuit
	Desperate		UMETA(DisplayName = "Desperate"),       // Phase 3 <15% HP: all-in
	Defeated		UMETA(DisplayName = "Defeated")
};

UCLASS()
class SNOWPIERCEREE_API ASEEBossController : public AAIController
{
	GENERATED_BODY()

public:
	ASEEBossController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "AI|Grey")
	void SetAIState(EGreyAIState NewState);

	UFUNCTION(BlueprintPure, Category = "AI|Grey")
	EGreyAIState GetAIState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "AI|Grey")
	bool ShouldDescendFromPost() const;

	/** Command post position — Grey stays here during Phase 1 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Grey")
	FVector CommandPostLocation = FVector::ZeroVector;

	/** Cover points for Phase 2 ranged engagement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Grey")
	TArray<FVector> CoverPoints;

protected:
	void UpdateCommanding(float DeltaTime);
	void UpdateEngagingRanged(float DeltaTime);
	void UpdateMeleeAssault(float DeltaTime);
	void UpdateDesperate(float DeltaTime);

	void TrySpecialAttack();
	void MoveToTarget();
	AActor* GetTarget() const;

	UPROPERTY()
	ASEEBossGrey* GreyPawn = nullptr;

	EGreyAIState CurrentState = EGreyAIState::Commanding;

	float RallyCryTimer = 0.f;
	float SpotlightTimer = 0.f;
	float AttackTimer = 0.f;
	float CoverSwitchTimer = 0.f;
	int32 CurrentCoverIndex = 0;

	static constexpr float RallyCryInterval = 15.f;
	static constexpr float SpotlightInterval = 10.f;
	static constexpr float MeleeAttackInterval = 2.f;
	static constexpr float CoverSwitchInterval = 8.f;
	static constexpr float DesperateHealthThreshold = 0.15f;
};
