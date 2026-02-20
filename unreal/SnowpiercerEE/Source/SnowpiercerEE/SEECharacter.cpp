#include "SEECharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Exploration/CollectibleComponent.h"

ASEECharacter::ASEECharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(35.0f, 88.0f);

	// First person camera (attached directly to capsule)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Third person spring arm + camera
	ThirdPersonArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ThirdPersonArm"));
	ThirdPersonArm->SetupAttachment(GetCapsuleComponent());
	ThirdPersonArm->TargetArmLength = 300.0f;
	ThirdPersonArm->bUsePawnControlRotation = true;
	ThirdPersonArm->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f));

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonArm, USpringArmComponent::SocketName);

	// Default to first person
	FirstPersonCamera->SetActive(true);
	ThirdPersonCamera->SetActive(false);
	ThirdPersonArm->SetActive(false);
	bFirstPersonActive = true;

	// Movement defaults
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(44.0f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->bCanWalkOffLedges = true;
	GetCharacterMovement()->JumpZVelocity = 420.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	bUseControllerRotationYaw = true;

	CurrentStamina = MaxStamina;
}

void ASEECharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	CurrentStamina = MaxStamina;
}

void ASEECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Stamina system
	if (bIsRunning && GetVelocity().SizeSquared() > 100.0f)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * DeltaTime);
		StaminaRegenTimer = StaminaRegenDelay;

		if (CurrentStamina <= 0.0f)
		{
			StopRun();
		}
	}
	else
	{
		StaminaRegenTimer = FMath::Max(0.0f, StaminaRegenTimer - DeltaTime);
		if (StaminaRegenTimer <= 0.0f)
		{
			CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
		}
	}

	// Sprint also drains stamina
	if (GetCharacterMovement()->MaxWalkSpeed >= SprintSpeed && GetVelocity().SizeSquared() > 100.0f)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - StaminaDrainRate * 1.5f * DeltaTime);
		StaminaRegenTimer = StaminaRegenDelay;

		if (CurrentStamina <= 0.0f)
		{
			StopSprint();
		}
	}
}

void ASEECharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASEECharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASEECharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASEECharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASEECharacter::StopSprint);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASEECharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASEECharacter::StopCrouch);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASEECharacter::Interact);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("ToggleView", IE_Pressed, this, &ASEECharacter::ToggleViewMode);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &ASEECharacter::StartRun);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &ASEECharacter::StopRun);
}

void ASEECharacter::MoveForward(float Value)
{
	if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASEECharacter::MoveRight(float Value)
{
	if (FMath::Abs(Value) > KINDA_SMALL_NUMBER)
	{
		const FRotator Rotation = GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ASEECharacter::StartSprint()
{
	if (CurrentStamina > 0.0f)
	{
		bIsRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void ASEECharacter::StopSprint()
{
	if (bIsRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	}
}

void ASEECharacter::StartRun()
{
	if (CurrentStamina > 0.0f)
	{
		bIsRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
}

void ASEECharacter::StopRun()
{
	bIsRunning = false;
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
}

void ASEECharacter::StartCrouch()
{
	bIsRunning = false;
	Crouch();
}

void ASEECharacter::StopCrouch()
{
	UnCrouch();
}

void ASEECharacter::ToggleViewMode()
{
	bFirstPersonActive = !bFirstPersonActive;

	FirstPersonCamera->SetActive(bFirstPersonActive);
	ThirdPersonCamera->SetActive(!bFirstPersonActive);
	ThirdPersonArm->SetActive(!bFirstPersonActive);

	bUseControllerRotationYaw = bFirstPersonActive;
	if (!bFirstPersonActive)
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
}

void ASEECharacter::Interact()
{
	UCameraComponent* ActiveCamera = bFirstPersonActive ? FirstPersonCamera : ThirdPersonCamera;
	FVector Start = ActiveCamera->GetComponentLocation();
	FVector End = Start + ActiveCamera->GetForwardVector() * 400.0f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			// Check for collectible component
			if (UCollectibleComponent* Collectible = HitActor->FindComponentByClass<UCollectibleComponent>())
			{
				Collectible->ActivateCollectible();
				return;
			}

			// Generic interaction — call interface or delegate on hit actor
			// Future: IInteractable interface check
		}
	}
}
