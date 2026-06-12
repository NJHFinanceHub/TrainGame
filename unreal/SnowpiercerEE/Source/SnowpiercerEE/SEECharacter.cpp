#include "SEECharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Exploration/CollectibleComponent.h"
#include "SEEHealthComponent.h"
#include "SEEStatsComponent.h"
#include "SEECombatComponent.h"
#include "SEEInventoryComponent.h"
#include "SEEWeaponBase.h"
#include "TrainGame/Economy/ArmorComponent.h"
#include "SEEColdComponent.h"
#include "Progression/SkillTreeComponent.h"
#include "Exploration/ClimbingComponent.h"
#include "Exploration/SwimmingComponent.h"
#include "AI/SEENPCAIController.h"
#include "UI/SEEUISubsystem.h"
#include "Engine/GameInstance.h"

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

	// Movement defaults — tuned for snappy starts and hard stops in tight corridors
	GetCharacterMovement()->MaxWalkSpeed = DefaultWalkSpeed;
	GetCharacterMovement()->MaxAcceleration = 2048.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
	GetCharacterMovement()->GroundFriction = 8.0f;
	GetCharacterMovement()->bUseSeparateBrakingFriction = true;
	GetCharacterMovement()->BrakingFriction = 6.0f;
	GetCharacterMovement()->BrakingFrictionFactor = 1.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(44.0f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	GetCharacterMovement()->bCanWalkOffLedges = true;
	GetCharacterMovement()->JumpZVelocity = 420.0f;
	GetCharacterMovement()->AirControl = 0.25f;

	// Camera FOV baseline
	FirstPersonCamera->SetFieldOfView(DefaultFOV);
	ThirdPersonCamera->SetFieldOfView(DefaultFOV);

	bUseControllerRotationYaw = true;

	CurrentStamina = MaxStamina;

	// Core gameplay components
	HealthComponent = CreateDefaultSubobject<USEEHealthComponent>(TEXT("HealthComponent"));
	StatsComponent = CreateDefaultSubobject<USEEStatsComponent>(TEXT("StatsComponent"));
	CombatComponent = CreateDefaultSubobject<USEECombatComponent>(TEXT("CombatComponent"));
	InventoryComponent = CreateDefaultSubobject<USEEInventoryComponent>(TEXT("InventoryComponent"));
	ArmorComponent = CreateDefaultSubobject<UArmorComponent>(TEXT("ArmorComponent"));
	ColdComponent = CreateDefaultSubobject<USEEColdComponent>(TEXT("ColdComponent"));
	SkillTreeComponent = CreateDefaultSubobject<USEESkillTreeComponent>(TEXT("SkillTreeComponent"));
	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
	SwimmingComponent = CreateDefaultSubobject<USwimmingComponent>(TEXT("SwimmingComponent"));
}

void ASEECharacter::BeginPlay()
{
	Super::BeginPlay();
	RefreshMoveSpeed();
	CurrentStamina = MaxStamina;
}

void ASEECharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateStamina(DeltaTime);
	UpdateCameraFOV(DeltaTime);
}

void ASEECharacter::UpdateStamina(float DeltaTime)
{
	const bool bMoving = GetVelocity().SizeSquared() > 100.0f;

	// Drain while actually moving under sprint/run; regenerate after a short delay otherwise
	float DrainRate = 0.0f;
	if (bMoving)
	{
		if (bIsSprinting)
		{
			DrainRate = StaminaDrainRate * 1.5f;
		}
		else if (bIsRunning)
		{
			DrainRate = StaminaDrainRate;
		}
	}

	if (DrainRate > 0.0f)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - DrainRate * DeltaTime);
		StaminaRegenTimer = StaminaRegenDelay;

		if (CurrentStamina <= 0.0f)
		{
			if (bIsSprinting) StopSprint();
			if (bIsRunning) StopRun();
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
}

void ASEECharacter::UpdateCameraFOV(float DeltaTime)
{
	// Transient impulses (dodge pulse, damage dip) decay back to zero
	FOVImpulse = FMath::FInterpTo(FOVImpulse, 0.0f, DeltaTime, FOVImpulseRecoverySpeed);

	// Sprint FOV kick only while actually moving at sprint pace
	const bool bSprintingFast = bIsSprinting && GetVelocity().SizeSquared() > FMath::Square(DefaultWalkSpeed * 0.5f);
	const float TargetFOV = (bSprintingFast ? SprintFOV : DefaultFOV) + FOVImpulse;

	UCameraComponent* ActiveCamera = bFirstPersonActive ? FirstPersonCamera : ThirdPersonCamera;
	if (ActiveCamera)
	{
		ActiveCamera->SetFieldOfView(FMath::FInterpTo(ActiveCamera->FieldOfView, TargetFOV, DeltaTime, FOVInterpSpeed));
	}
}

void ASEECharacter::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f) return;

	CurrentStamina = FMath::Max(0.0f, CurrentStamina - Amount);
	StaminaRegenTimer = StaminaRegenDelay;

	if (CurrentStamina <= 0.0f)
	{
		if (bIsSprinting) StopSprint();
		if (bIsRunning) StopRun();
	}
}

void ASEECharacter::AddCameraFOVImpulse(float Offset)
{
	FOVImpulse = FMath::Clamp(FOVImpulse + Offset, -MaxFOVImpulse, MaxFOVImpulse);
}

void ASEECharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// Hard landings briefly dampen move speed for a weighty recovery
	const float FallSpeed = -GetCharacterMovement()->Velocity.Z;
	if (FallSpeed >= HardLandingSpeed)
	{
		bLandingRecoveryActive = true;
		RefreshMoveSpeed();
		GetWorldTimerManager().SetTimer(LandingRecoveryTimer, this, &ASEECharacter::EndLandingRecovery, LandingRecoveryDuration, false);
	}
}

void ASEECharacter::EndLandingRecovery()
{
	bLandingRecoveryActive = false;
	RefreshMoveSpeed();
}

void ASEECharacter::RefreshMoveSpeed()
{
	float Speed = DefaultWalkSpeed;
	if (bIsSprinting)
	{
		Speed = SprintSpeed;
	}
	else if (bIsRunning)
	{
		Speed = RunSpeed;
	}

	if (bLandingRecoveryActive)
	{
		Speed *= LandingRecoverySpeedScale;
	}

	GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void ASEECharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Spawned quickslot weapons are owned by this character — don't leak them
	if (QuickSlotWeapon)
	{
		QuickSlotWeapon->Destroy();
		QuickSlotWeapon = nullptr;
	}

	Super::EndPlay(EndPlayReason);
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
	if (CurrentStamina >= SprintMinStamina)
	{
		bIsSprinting = true;
		bIsRunning = false;
		RefreshMoveSpeed();
	}
}

void ASEECharacter::StopSprint()
{
	bIsSprinting = false;
	RefreshMoveSpeed();
}

void ASEECharacter::StartRun()
{
	if (CurrentStamina > 0.0f)
	{
		bIsRunning = true;
		bIsSprinting = false;
		RefreshMoveSpeed();
	}
}

void ASEECharacter::StopRun()
{
	bIsRunning = false;
	RefreshMoveSpeed();
}

void ASEECharacter::StartCrouch()
{
	bIsRunning = false;
	bIsSprinting = false;
	RefreshMoveSpeed();
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
	// A full-screen/dialogue panel already owns input — the panel handles E.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
		{
			if (UISub->IsScreenOpen()) return;
		}
	}

	// Talkable NPC in front of us takes priority over world interactables.
	if (TryStartNPCDialogue()) return;

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

bool ASEECharacter::TryStartNPCDialogue()
{
	// Pawn capsules ignore the visibility channel, so NPCs need their own
	// sweep on the pawn channel: short reach, fat radius for easy targeting.
	UCameraComponent* ActiveCamera = bFirstPersonActive ? FirstPersonCamera : ThirdPersonCamera;
	if (!ActiveCamera) return false;

	const FVector Start = ActiveCamera->GetComponentLocation();
	const FVector End = Start + ActiveCamera->GetForwardVector() * 250.0f;

	FCollisionQueryParams Params(FName(TEXT("SEEInteractNPC")), false, this);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(60.0f), Params);

	for (const FHitResult& Hit : Hits)
	{
		APawn* HitPawn = Cast<APawn>(Hit.GetActor());
		if (!HitPawn || HitPawn == this) continue;

		ASEENPCAIController* Brain = Cast<ASEENPCAIController>(HitPawn->GetController());
		if (!Brain || !Brain->CanStartDialogue()) continue;

		if (UGameInstance* GI = GetGameInstance())
		{
			if (USEEUISubsystem* UISub = GI->GetSubsystem<USEEUISubsystem>())
			{
				UISub->OpenDialogue(HitPawn);
				return true;
			}
		}
	}

	return false;
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

void ASEECharacter::UseQuickSlot1() { HandleQuickSlot(0); }
void ASEECharacter::UseQuickSlot2() { HandleQuickSlot(1); }
void ASEECharacter::UseQuickSlot3() { HandleQuickSlot(2); }
void ASEECharacter::UseQuickSlot4() { HandleQuickSlot(3); }

void ASEECharacter::HandleQuickSlot(int32 QuickSlotIndex)
{
	// Weapon-category items take priority: slot N toggles the Nth weapon in the bag.
	if (TryToggleWeaponQuickSlot(QuickSlotIndex))
	{
		return;
	}

	// Fall through to the inventory's assigned quickslot path (consumables etc.)
	if (InventoryComponent)
	{
		InventoryComponent->UseQuickSlot(QuickSlotIndex);
	}
}

bool ASEECharacter::TryToggleWeaponQuickSlot(int32 WeaponOrdinal)
{
	if (!InventoryComponent || !CombatComponent)
	{
		return false;
	}

	// Scan inventory slots in order for the Nth weapon-category item
	FName TargetItemID = NAME_None;
	int32 WeaponsSeen = 0;
	for (const FSEEInventorySlot& Slot : InventoryComponent->GetAllSlots())
	{
		if (Slot.IsEmpty())
		{
			continue;
		}

		bool bIsWeapon = false;
		if (const FSEEItemData* Data = InventoryComponent->GetItemDataPtr(Slot.ItemID))
		{
			bIsWeapon = (Data->Category == ESEEItemCategory::Weapon);
		}
		else
		{
			// Item data table missing/unassigned — fall back to the known weapon ItemID set
			bIsWeapon = ASEEWeaponBase::IsWeaponItemID(Slot.ItemID);
		}

		if (!bIsWeapon)
		{
			continue;
		}

		if (WeaponsSeen == WeaponOrdinal)
		{
			TargetItemID = Slot.ItemID;
			break;
		}
		++WeaponsSeen;
	}

	if (TargetItemID.IsNone())
	{
		return false;
	}

	// Same weapon already equipped -> toggle off
	if (QuickSlotWeapon && QuickSlotWeapon->GetSourceItemID() == TargetItemID)
	{
		UnequipQuickSlotWeapon();
		return true;
	}

	EquipWeaponByItemID(TargetItemID);
	return true; // handled even if the spawn failed (don't consume the item as a consumable)
}

void ASEECharacter::EquipWeaponByItemID(FName ItemID)
{
	// Replace any previously spawned quickslot weapon
	UnequipQuickSlotWeapon();

	ASEEWeaponBase* NewWeapon = ASEEWeaponBase::SpawnWeaponForItem(GetWorld(), ItemID, this);
	if (!NewWeapon)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: failed to spawn weapon for item '%s'"), *GetName(), *ItemID.ToString());
		return;
	}

	// Combat component drives damage/speed/range from the equipped weapon
	// (and performs its own attach to the mesh's weapon_r socket)
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(NewWeapon);
	}

	// Refine the attachment for meshes without a weapon_r socket
	AttachWeaponActorToHand(NewWeapon);

	QuickSlotWeapon = NewWeapon;
	UE_LOG(LogTemp, Log, TEXT("%s equipped '%s' (dmg %.0f, speed %.1f)"),
		*GetName(), *ItemID.ToString(), NewWeapon->GetBaseDamage(), NewWeapon->GetAttackSpeed());
}

void ASEECharacter::UnequipQuickSlotWeapon()
{
	if (!QuickSlotWeapon)
	{
		return;
	}

	if (CombatComponent && CombatComponent->GetEquippedWeapon() == QuickSlotWeapon)
	{
		CombatComponent->UnequipWeapon();
	}

	UE_LOG(LogTemp, Log, TEXT("%s unequipped '%s'"), *GetName(), *QuickSlotWeapon->GetSourceItemID().ToString());
	QuickSlotWeapon->Destroy();
	QuickSlotWeapon = nullptr;
}

void ASEECharacter::AttachWeaponActorToHand(AActor* WeaponActor)
{
	if (!WeaponActor)
	{
		return;
	}

	const FAttachmentTransformRules Rules(EAttachmentRule::SnapToTarget, true);
	USkeletalMeshComponent* MeshComp = GetMesh();

	if (MeshComp && MeshComp->DoesSocketExist(TEXT("weapon_r")))
	{
		WeaponActor->AttachToComponent(MeshComp, Rules, TEXT("weapon_r"));
	}
	else if (MeshComp && MeshComp->DoesSocketExist(TEXT("hand_r")))
	{
		WeaponActor->AttachToComponent(MeshComp, Rules, TEXT("hand_r"));
	}
	else
	{
		// No usable skeleton — hang it off the capsule with a forward offset so it's visible
		WeaponActor->AttachToComponent(GetRootComponent(), Rules);
		WeaponActor->SetActorRelativeLocation(FVector(45.0f, 20.0f, 10.0f));
		WeaponActor->SetActorRelativeRotation(FRotator::ZeroRotator);
	}
}
