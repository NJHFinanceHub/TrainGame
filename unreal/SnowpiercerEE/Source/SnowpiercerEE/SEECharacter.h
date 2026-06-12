#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEECharacter.generated.h"

class USEEHealthComponent;
class USEEStatsComponent;
class USEECombatComponent;
class USEEInventoryComponent;
class UArmorComponent;
class USEEColdComponent;
class USEESkillTreeComponent;
class UClimbingComponent;
class USwimmingComponent;
class ASEEWeaponBase;

UCLASS()
class SNOWPIERCEREE_API ASEECharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEECharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveForward(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MoveRight(float Value);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopSprint();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopCrouch();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void Interact();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartRun();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StopRun();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleViewMode();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsSprinting() const { return bIsSprinting; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxStamina() const { return MaxStamina; }

	/** Spend stamina (clamped at zero) and reset the regen delay. */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void ConsumeStamina(float Amount);

	/** True if at least Amount stamina is available. */
	UFUNCTION(BlueprintPure, Category = "Stats")
	bool HasStamina(float Amount) const { return CurrentStamina >= Amount; }

	/** Invulnerability flag checked by the incoming-damage path (dodge i-frames). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvulnerable(bool bInvulnerable) { bIsInvulnerable = bInvulnerable; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsInvulnerable() const { return bIsInvulnerable; }

	/** Add a transient FOV offset (positive = punch out, negative = dip in) that decays back to zero. */
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void AddCameraFOVImpulse(float Offset);

	virtual void Landed(const FHitResult& Hit) override;

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultWalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 650.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 550.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchSpeed = 200.0f;

	// --- Landing recovery (hard landings briefly dampen move speed) ---

	/** Downward landing speed (cm/s) above which landing recovery kicks in */
	UPROPERTY(EditAnywhere, Category = "Movement|Landing")
	float HardLandingSpeed = 700.0f;

	/** How long the post-landing speed dampen lasts */
	UPROPERTY(EditAnywhere, Category = "Movement|Landing")
	float LandingRecoveryDuration = 0.35f;

	/** Move speed multiplier while recovering from a hard landing */
	UPROPERTY(EditAnywhere, Category = "Movement|Landing")
	float LandingRecoverySpeedScale = 0.4f;

	// --- Camera feel ---

	/** Base camera field of view */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float DefaultFOV = 90.0f;

	/** Field of view while sprinting at speed */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float SprintFOV = 100.0f;

	/** Interp speed for FOV transitions */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float FOVInterpSpeed = 8.0f;

	/** How quickly transient FOV impulses (dodge pulse, damage dip) decay */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float FOVImpulseRecoverySpeed = 6.0f;

	/** Clamp for accumulated transient FOV impulses */
	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxFOVImpulse = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaDrainRate = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaRegenRate = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaRegenDelay = 1.5f;

	/** Minimum stamina required to start sprinting (prevents stutter-sprint at zero) */
	UPROPERTY(EditAnywhere, Category = "Stats")
	float SprintMinStamina = 5.0f;

	// Combat input
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LightAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HeavyAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StopBlock();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void DodgeInput();

	// Quick slot input
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot1();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot2();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot3();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UseQuickSlot4();

	// --- Quickslot weapon equip ---

	/** Attach a spawned weapon actor to the right hand (socket if available, root offset fallback).
	    Overridable so the player character can route through its weapon socket helper. */
	virtual void AttachWeaponActorToHand(AActor* WeaponActor);

	/** Destroy and unequip the currently equipped quickslot weapon (no-op when none). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void UnequipQuickSlotWeapon();

	// Component accessors
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEEHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEEStatsComponent> StatsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEECombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEEInventoryComponent> InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArmorComponent> ArmorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEEColdComponent> ColdComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USEESkillTreeComponent> SkillTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UClimbingComponent> ClimbingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USwimmingComponent> SwimmingComponent;

private:
	UPROPERTY()
	TObjectPtr<class UCameraComponent> FirstPersonCamera;

	UPROPERTY()
	TObjectPtr<class USpringArmComponent> ThirdPersonArm;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> ThirdPersonCamera;

	float CurrentStamina = 100.0f;
	float StaminaRegenTimer = 0.0f;
	bool bIsRunning = false;
	bool bIsSprinting = false;
	bool bIsInvulnerable = false;
	bool bLandingRecoveryActive = false;
	bool bFirstPersonActive = true;
	bool bHeavyAttackCharging = false;
	float FOVImpulse = 0.0f;
	FTimerHandle LandingRecoveryTimer;

	/** Recompute MaxWalkSpeed from sprint/run flags and landing recovery */
	void RefreshMoveSpeed();
	void UpdateStamina(float DeltaTime);
	void UpdateCameraFOV(float DeltaTime);
	void EndLandingRecovery();

	/** Interact helper: sweep for a talkable NPC ahead and open dialogue. */
	bool TryStartNPCDialogue();

	// --- Quickslot weapon equip ---

	/** Shared quickslot handler: weapon slots toggle equip, everything else
	    falls through to the inventory's consumable quickslot path. */
	void HandleQuickSlot(int32 QuickSlotIndex);

	/** Find the Nth weapon-category item in the inventory and toggle-equip it.
	    Returns true if the slot resolved to a weapon (handled), false to fall through. */
	bool TryToggleWeaponQuickSlot(int32 WeaponOrdinal);

	/** Spawn + equip a weapon for the given inventory ItemID, replacing any previous one. */
	void EquipWeaponByItemID(FName ItemID);

	/** Weapon actor spawned by the quickslot equip flow (owned + destroyed by this character). */
	UPROPERTY()
	TObjectPtr<ASEEWeaponBase> QuickSlotWeapon;
};
