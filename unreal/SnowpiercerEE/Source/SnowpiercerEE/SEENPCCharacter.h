#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEETypes.h"
#include "SEENPCCharacter.generated.h"

UENUM(BlueprintType)
enum class ESEENPCState : uint8
{
	Idle,
	Patrolling,
	Conversing,
	Working,
	Suspicious,
	Alerted,
	Fleeing,
	Combat,
	Dead
};

UENUM(BlueprintType)
enum class ESEENPCClass : uint8
{
	Tailie,
	ThirdClass,
	SecondClass,
	FirstClass,
	Jackboot,
	OrderMember,
	Breachman,
	Conductor,
	Engineer
};

USTRUCT(BlueprintType)
struct SNOWPIERCEREE_API FSEENPCDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 3.0f;
};

UCLASS()
class SNOWPIERCEREE_API ASEENPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEENPCCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "NPC")
	ESEENPCState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	void SetState(ESEENPCState NewState);

	UFUNCTION(BlueprintCallable, Category = "NPC")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintCallable, Category = "NPC")
	ESEENPCClass GetNPCClass() const { return NPCClass; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	bool HasDialogue() const { return AmbientLines.Num() > 0; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Dialogue")
	FSEENPCDialogueLine GetRandomAmbientLine() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
	void AddPatrolPoint(FVector Point);

	UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
	void StartPatrol();

	UFUNCTION(BlueprintCallable, Category = "NPC|Patrol")
	void StopPatrol();

	UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
	bool CanSeeActor(AActor* Target) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Detection")
	float GetDetectionLevel() const { return DetectionLevel; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ESEENPCClass NPCClass = ESEENPCClass::Tailie;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ESEENPCState CurrentState = ESEENPCState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float WalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float RunSpeed = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
	float SightRange = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
	float SightAngle = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
	float HearingRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Detection")
	float DetectionRate = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	TArray<FSEENPCDialogueLine> AmbientLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
	TArray<FVector> PatrolPoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Patrol")
	float PatrolWaitTime = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadWrite, Category = "NPC|Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float MeleeDamage = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float AttackCooldown = 1.2f;

	void UpdatePatrol(float DeltaTime);
	void UpdateDetection(float DeltaTime);
	void UpdateCombat(float DeltaTime);

private:
	int32 CurrentPatrolIndex = 0;
	float PatrolWaitTimer = 0.0f;
	float DetectionLevel = 0.0f;
	float AttackTimer = 0.0f;
	bool bPatrolForward = true;
};
