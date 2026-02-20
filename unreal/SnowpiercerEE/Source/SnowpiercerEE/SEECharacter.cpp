#include "SEECharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Exploration/CollectibleComponent.h"
#include "SEEHealthComponent.h"
#include "SEEStatsComponent.h"
#include "SEECombatComponent.h"
#include "SEEInventoryComponent.h"
#include "SEEHungerComponent.h"
#include "SEEColdComponent.h"
#include "Progression/SkillTreeComponent.h"
#include "Exploration/ClimbingComponent.h"
#include "Exploration/SwimmingComponent.h"

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

	// Core gameplay components
	HealthComponent = CreateDefaultSubobject<USEEHealthComponent>(TEXT("HealthComponent"));
	StatsComponent = CreateDefaultSubobject<USEEStatsComponent>(TEXT("StatsComponent"));
	CombatComponent = CreateDefaultSubobject<USEECombatComponent>(TEXT("CombatComponent"));
	InventoryComponent = CreateDefaultSubobject<USEEInventoryComponent>(TEXT("InventoryComponent"));
	HungerComponent = CreateDefaultSubobject<USEEHungerComponent>(TEXT("HungerComponent"));
	ColdComponent = CreateDefaultSubobject<USEEColdComponent>(TEXT("ColdComponent"));
	SkillTreeComponent = CreateDefaultSubobject<USEESkillTreeComponent>(TEXT("SkillTreeComponent"));
	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
	SwimmingComponent = CreateDefaultSubobject<USwimmingComponent>(TEXT("SwimmingComponent"));
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

	// Combat
	PlayerInputComponent->BindAction("LightAttack", IE_Pressed, this, &ASEECharacter::LightAttack);
	PlayerInputComponent->BindAction("HeavyAttack", IE_Pressed, this, &ASEECharacter::HeavyAttack);
	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &ASEECharacter::StartBlock);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &ASEECharacter::StopBlock);
	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &ASEECharacter::DodgeInput);

	// Quick slots
	PlayerInputComponent->BindAction("QuickSlot1", IE_Pressed, this, &ASEECharacter::UseQuickSlot1);
	PlayerInputComponent->BindAction("QuickSlot2", IE_Pressed, this, &ASEECharacter::UseQuickSlot2);
	PlayerInputComponent->BindAction("QuickSlot3", IE_Pressed, this, &ASEECharacter::UseQuickSlot3);
	PlayerInputComponent->BindAction("QuickSlot4", IE_Pressed, this, &ASEECharacter::UseQuickSlot4);
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

void ASEECharacter::LightAttack()
{
	if (CombatComponent) CombatComponent->LightAttack();
}

void ASEECharacter::HeavyAttack()
{
	if (CombatComponent) CombatComponent->HeavyAttack();
}

void ASEECharacter::StartBlock()
{
	if (CombatComponent) CombatComponent->StartBlock();
}

void ASEECharacter::StopBlock()
{
	if (CombatComponent) CombatComponent->StopBlock();
}

void ASEECharacter::DodgeInput()
{
	if (CombatComponent)
	{
		FVector DodgeDir = GetLastMovementInputVector();
		CombatComponent->Dodge(DodgeDir);
	}
}

void ASEECharacter::UseQuickSlot1() { if (InventoryComponent) InventoryComponent->UseQuickSlot(0); }
void ASEECharacter::UseQuickSlot2() { if (InventoryComponent) InventoryComponent->UseQuickSlot(1); }
void ASEECharacter::UseQuickSlot3() { if (InventoryComponent) InventoryComponent->UseQuickSlot(2); }
void ASEECharacter::UseQuickSlot4() { if (InventoryComponent) InventoryComponent->UseQuickSlot(3); }
