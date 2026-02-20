// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AITypes.h"
#include "SEENPCCharacter.generated.h"

class UCombatComponent;
class UWeaponComponent;
class UDetectionComponent;
class UNPCScheduleComponent;
class UBodyDiscoveryComponent;

// ============================================================================
// ASEENPCCharacter
//
// Base character for all NPCs in the train. Combines combat, detection,
// scheduling, and body-state tracking. AI controllers attach to this to
// drive behavior via behavior trees.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCStateChanged, ENPCAIState, OldState, ENPCAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBodyStateChanged, EBodyState, OldState, EBodyState, NewState);

UCLASS()
class TRAINGAME_API ASEENPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEENPCCharacter();

	// --- Component Accessors ---

	UFUNCTION(BlueprintPure, Category = "NPC")
	UCombatComponent* GetCombatComponent() const { return CombatComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UWeaponComponent* GetWeaponComponent() const { return WeaponComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UDetectionComponent* GetDetectionComponent() const { return DetectionComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UNPCScheduleComponent* GetScheduleComponent() const { return ScheduleComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UBodyDiscoveryComponent* GetBodyDiscoveryComponent() const { return BodyDiscoveryComp; }

	// --- NPC Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	ENPCArchetype Archetype = ENPCArchetype::Civilian;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	ETrainZone HomeZone = ETrainZone::ThirdClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FName NPCID;

	// --- Body State (for stealth/body discovery) ---

	UFUNCTION(BlueprintPure, Category = "NPC|Body")
	EBodyState GetBodyState() const { return BodyState; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Body")
	void SetBodyState(EBodyState NewState);

	UFUNCTION(BlueprintPure, Category = "NPC|Body")
	bool IsIncapacitated() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Body")
	bool IsAlive() const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCStateChanged OnNPCStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnBodyStateChanged OnBodyStateChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Combat")
	UCombatComponent* CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Combat")
	UWeaponComponent* WeaponComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Detection")
	UDetectionComponent* DetectionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Schedule")
	UNPCScheduleComponent* ScheduleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|BodyDiscovery")
	UBodyDiscoveryComponent* BodyDiscoveryComp;

private:
	EBodyState BodyState = EBodyState::Stunned; // Stunned = "alive and normal" for NPCs
};
