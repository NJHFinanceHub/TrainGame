// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TrainGameAITypes.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "SEENPCCharacter.generated.h"

class UCombatComponent;
class UDetectionComponent;
class UStealthComponent;
class UNPCMemoryComponent;
class UNPCScheduleComponent;
class UBodyDiscoveryComponent;
class UCompanionComponent;

// ============================================================================
// ASEENPCCharacter
//
// Base NPC character for Snowpiercer: Eternal Engine. Wires together all
// AI-relevant components and provides the interface between gameplay systems
// and the AI controllers.
//
// Component architecture:
//   Required: DetectionComponent, NPCScheduleComponent, BodyDiscoveryComponent
//   Optional: CombatComponent (non-crowd NPCs), CompanionComponent (companions)
//             NPCMemoryComponent (dialogue-capable NPCs)
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCStateChanged,
	ENPCAIState, OldState,
	ENPCAIState, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPCIncapacitated,
	EBodyState, BodyState);

UCLASS()
class TRAINGAME_API ASEENPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEENPCCharacter();

	virtual void BeginPlay() override;

	// --- Identity ---

	/** Get this NPC's class/caste */
	UFUNCTION(BlueprintPure, Category = "NPC")
	ENPCClass GetNPCClass() const { return NPCRole; }

	/** Get this NPC's display name */
	UFUNCTION(BlueprintPure, Category = "NPC")
	FText GetNPCName() const { return NPCName; }

	/** Get the zone this NPC belongs to */
	UFUNCTION(BlueprintPure, Category = "NPC")
	EStealthZone GetHomeZone() const { return HomeZone; }

	// --- Health/State ---

	/** Check if this NPC is alive and active */
	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsAlive() const { return CurrentBodyState == EBodyState::Restrained || !bIsIncapacitated; }

	/** Check if this NPC is incapacitated */
	UFUNCTION(BlueprintPure, Category = "NPC")
	bool IsIncapacitated() const { return bIsIncapacitated; }

	/** Get the body state if incapacitated */
	UFUNCTION(BlueprintPure, Category = "NPC")
	EBodyState GetBodyState() const { return CurrentBodyState; }

	/** Incapacitate this NPC (registers body for discovery) */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void Incapacitate(EBodyState State);

	/** Wake up / revive this NPC */
	UFUNCTION(BlueprintCallable, Category = "NPC")
	void Revive();

	// --- Component Access ---

	UFUNCTION(BlueprintPure, Category = "NPC")
	UDetectionComponent* GetDetectionComponent() const { return DetectionComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UCombatComponent* GetCombatComponent() const { return CombatComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UNPCScheduleComponent* GetScheduleComponent() const { return ScheduleComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UBodyDiscoveryComponent* GetBodyDiscoveryComponent() const { return BodyDiscoveryComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UNPCMemoryComponent* GetMemoryComponent() const { return MemoryComp; }

	UFUNCTION(BlueprintPure, Category = "NPC")
	UCompanionComponent* GetCompanionComponent() const { return CompanionComp; }

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCStateChanged OnNPCStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC")
	FOnNPCIncapacitated OnNPCIncapacitated;

protected:
	// --- Identity ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	ENPCClass NPCRole = ENPCClass::ThirdClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	FText NPCName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	EStealthZone HomeZone = EStealthZone::ThirdClass;

	/** Whether this NPC is a unique named character (vs generic) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Identity")
	bool bIsUnique = false;

	// --- Components ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UDetectionComponent* DetectionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UNPCScheduleComponent* ScheduleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UBodyDiscoveryComponent* BodyDiscoveryComp;

	/** Optional — added for combat-capable NPCs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UCombatComponent* CombatComp;

	/** Optional — added for dialogue-capable NPCs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UNPCMemoryComponent* MemoryComp;

	/** Optional — added for companion NPCs */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Components")
	UCompanionComponent* CompanionComp;

private:
	bool bIsIncapacitated = false;
	EBodyState CurrentBodyState = EBodyState::Stunned;
};
