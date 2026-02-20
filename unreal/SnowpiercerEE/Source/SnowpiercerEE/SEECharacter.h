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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DefaultWalkSpeed = 400.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SprintSpeed = 700.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float CrouchSpeed = 200.0f;

private:
	UPROPERTY()
	TObjectPtr<class UCameraComponent> FirstPersonCamera;

	UPROPERTY()
	TObjectPtr<class USpringArmComponent> ThirdPersonArm;

	UPROPERTY()
	TObjectPtr<class UCameraComponent> ThirdPersonCamera;
};
