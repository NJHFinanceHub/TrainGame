// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AITypes.h"
#include "SEENPCCharacter.generated.h"

class UDetectionComponent;
class UNPCMemoryComponent;
class UDialogueComponent;
class UCombatComponent;
class UWeaponComponent;
class UNPCScheduleComponent;
class UBodyDiscoveryComponent;
class UBehaviorTree;
class UBlackboardData;

// ============================================================================
// ASEENPCCharacter
//
// Base NPC character for Snowpiercer: Eternal Engine. Wires together all
// AI subsystems: BehaviorTree-driven AI controller, detection, combat,
// dialogue/memory, scheduling, and body discovery. Place in level or spawn
// via NPCSpawnerComponent. Configure via NPC role and attached components.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCStateChanged, ENPCAIState, OldState, ENPCAIState, NewState);

UCLASS()
class TRAINGAME_API ASEENPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEENPCCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Identity ---

	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	ENPCRole GetNPCRole() const { return NPCRole; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	ETrainZone GetHomeZone() const { return HomeZone; }

	// --- State ---

	UFUNCTION(BlueprintPure, Category = "NPC")
	ENPCAIState GetCurrentAIState() const;

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsAlive() const { return CurrentHealth > 0.f; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsCombatCapable() const { return bCanFight; }

	// --- Health ---

	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	void ApplyDamage(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "NPC|Combat")
	void Incapacitate(EBodyState NewBodyState);

	UFUNCTION(BlueprintPure, Category = "NPC|Combat")
	float GetHealthPercent() const { return MaxHealth > 0.f ? CurrentHealth / MaxHealth : 0.f; }

	// --- Ambient Dialogue ---

	UFUNCTION(BlueprintPure, Category = "NPC|Dialogue")
	bool HasAmbientDialogue() const { return AmbientLines.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "NPC|Dialogue")
	FText GetRandomAmbientLine() const;

	// --- Component Access ---

	UFUNCTION(BlueprintPure, Category = "NPC|Components")
	UDetectionComponent* GetDetectionComponent() const { return DetectionComp; }

	UFUNCTION(BlueprintPure, Category = "NPC|Components")
	UNPCScheduleComponent* GetScheduleComponent() const { return ScheduleComp; }

	UFUNCTION(BlueprintPure, Category = "NPC|Components")
	UBodyDiscoveryComponent* GetBodyDiscoveryComponent() const { return BodyDiscoveryComp; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCStateChanged OnNPCStateChanged;

protected:
	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ENPCRole NPCRole = ENPCRole::ThirdClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	ETrainZone HomeZone = ETrainZone::ThirdClass;

	// --- AI ---

	/** Behavior tree to assign to the AI controller */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
	UBehaviorTree* BehaviorTreeAsset = nullptr;

	/** Blackboard data asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
	UBlackboardData* BlackboardAsset = nullptr;

	/** AI controller class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
	TSubclassOf<AAIController> AIControllerClass_Override;

	// --- Combat ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	bool bCanFight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float MaxHealth = 100.f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Combat")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float MeleeDamage = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Combat")
	float AttackRange = 200.f;

	// --- Movement ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float WalkSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float RunSpeed = 450.f;

	// --- Ambient Dialogue ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Dialogue")
	TArray<FText> AmbientLines;

	// --- Body State ---

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	EBodyState BodyState = EBodyState::Stunned;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	bool bIsIncapacitated = false;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UDetectionComponent* DetectionComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UNPCScheduleComponent* ScheduleComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UBodyDiscoveryComponent* BodyDiscoveryComp = nullptr;

private:
	void Die(AActor* Killer);
	void MarkAsBody(EBodyState State);

	ENPCAIState LastKnownAIState = ENPCAIState::Idle;
};
