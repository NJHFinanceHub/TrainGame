#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SEECharacter.generated.h"

UCLASS()
class SNOWPIERCEREE_API ASEECharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASEECharacter();

	virtual void BeginPlay() override;
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

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetStamina() const { return CurrentStamina; }

	UFUNCTION(BlueprintCallable, Category = "Stats")
	float GetMaxStamina() const { return MaxStamina; }

	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultWalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float RunSpeed = 550.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaDrainRate = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaRegenRate = 8.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Stats")
	float StaminaRegenDelay = 1.5f;

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
	bool bFirstPersonActive = true;
};
