// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EnemyCharacterBase.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyCharacterBase::AEnemyCharacterBase()
{
}

void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	// Apply type preset if configured
	ApplyTypePreset(EnemyConfig.EnemyType);

	// Equip starting weapon
	if (WeaponComp && StartingWeapon.WeaponName != NAME_None)
	{
		WeaponComp->EquipWeapon(StartingWeapon);
	}
}

void AEnemyCharacterBase::ApplyTypePreset(EEnemyType Type)
{
	EnemyConfig.EnemyType = Type;

	switch (Type)
	{
		case EEnemyType::JackbootGrunt:		SetupJackbootGrunt(); break;
		case EEnemyType::JackbootCaptain:	SetupJackbootCaptain(); break;
		case EEnemyType::OrderZealot:		SetupOrderZealot(); break;
		case EEnemyType::FirstClassGuard:	SetupFirstClassGuard(); break;
		case EEnemyType::TailFighter:		SetupTailFighter(); break;
		case EEnemyType::KronoleAddict:		SetupKronoleAddict(); break;
		case EEnemyType::Breachman:			SetupBreachman(); break;
	}
}

void AEnemyCharacterBase::AlertToThreat(AActor* Threat)
{
	if (bIsAlerted) return;

	bIsAlerted = true;
	OnEnemyAlerted.Broadcast(this, Threat);

	if (bCallsBackup)
	{
		CallForBackup();
	}
}

void AEnemyCharacterBase::CallForBackup()
{
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacterBase::StaticClass(), AllEnemies);

	for (AActor* Actor : AllEnemies)
	{
		if (Actor == this) continue;

		float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
		if (Dist <= BackupCallRadius)
		{
			AEnemyCharacterBase* OtherEnemy = Cast<AEnemyCharacterBase>(Actor);
			if (OtherEnemy && !OtherEnemy->IsAlerted())
			{
				OtherEnemy->AlertToThreat(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
			}
		}
	}
}

// ============================================================================
// Enemy Type Presets
// ============================================================================

void AEnemyCharacterBase::SetupJackbootGrunt()
{
	// Standard soldier: disciplined, decent stats, shock baton
	EnemyConfig.MaxHealth = 80.f;
	EnemyConfig.MoveSpeed = 450.f;
	EnemyConfig.BaseDamage = 12.f;
	EnemyConfig.DetectionRange = 1500.f;
	EnemyConfig.AttackRange = 180.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.1f;

	StartingWeapon = UWeaponComponent::MakeJackbootBaton();

	if (CombatComp)
	{
		// Jackboots are trained — better blocking
	}
}

void AEnemyCharacterBase::SetupJackbootCaptain()
{
	// Commander: rallies troops, tougher, calls reinforcements
	EnemyConfig.MaxHealth = 150.f;
	EnemyConfig.MoveSpeed = 400.f;
	EnemyConfig.BaseDamage = 18.f;
	EnemyConfig.DetectionRange = 2000.f;
	EnemyConfig.AttackRange = 200.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.4f;

	bCallsBackup = true;
	BackupCallRadius = 3000.f;

	// Captains carry reinforced axes
	StartingWeapon = UWeaponComponent::MakeReinforcedAxe();
}

void AEnemyCharacterBase::SetupOrderZealot()
{
	// Fanatical: high aggression, hard to stagger, fights to the death
	EnemyConfig.MaxHealth = 120.f;
	EnemyConfig.MoveSpeed = 500.f;
	EnemyConfig.BaseDamage = 20.f;
	EnemyConfig.DetectionRange = 1800.f;
	EnemyConfig.AttackRange = 160.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.3f;

	// Zealots carry bladed weapons
	FWeaponStats ZealotBlade;
	ZealotBlade.WeaponName = FName("Ceremonial Blade");
	ZealotBlade.Category = EWeaponCategory::Bladed;
	ZealotBlade.Tier = EWeaponTier::Specialized;
	ZealotBlade.BaseDamage = 22.f;
	ZealotBlade.AttackSpeed = 1.2f;
	ZealotBlade.Range = 140.f;
	ZealotBlade.StaminaCostPerSwing = 10.f;
	ZealotBlade.Durability = 100.f;
	ZealotBlade.MaxDurability = 100.f;
	ZealotBlade.DurabilityLossPerHit = 1.f;
	ZealotBlade.DurabilityLossOnBlock = 2.f;
	StartingWeapon = ZealotBlade;
}

void AEnemyCharacterBase::SetupFirstClassGuard()
{
	// Elite: fast, precise, high dodge chance, elegant fighting style
	EnemyConfig.MaxHealth = 100.f;
	EnemyConfig.MoveSpeed = 550.f;
	EnemyConfig.BaseDamage = 16.f;
	EnemyConfig.DetectionRange = 2000.f;
	EnemyConfig.AttackRange = 170.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.5f;

	// First Class guards carry fencing sabres
	FWeaponStats Sabre;
	Sabre.WeaponName = FName("Fencing Sabre");
	Sabre.Category = EWeaponCategory::Bladed;
	Sabre.Tier = EWeaponTier::Specialized;
	Sabre.BaseDamage = 18.f;
	Sabre.AttackSpeed = 1.5f;
	Sabre.Range = 190.f;
	Sabre.StaminaCostPerSwing = 8.f;
	Sabre.Durability = 150.f;
	Sabre.MaxDurability = 150.f;
	Sabre.DurabilityLossPerHit = 0.5f;
	Sabre.DurabilityLossOnBlock = 1.f;
	StartingWeapon = Sabre;
}

void AEnemyCharacterBase::SetupTailFighter()
{
	// Desperate: scrappy, low health, improvised weapons
	EnemyConfig.MaxHealth = 60.f;
	EnemyConfig.MoveSpeed = 420.f;
	EnemyConfig.BaseDamage = 10.f;
	EnemyConfig.DetectionRange = 1000.f;
	EnemyConfig.AttackRange = 150.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.f;

	// Random improvised weapon
	float Roll = FMath::FRand();
	if (Roll < 0.33f)
		StartingWeapon = UWeaponComponent::MakePipeClub();
	else if (Roll < 0.66f)
		StartingWeapon = UWeaponComponent::MakeShiv();
	else
		StartingWeapon = UWeaponComponent::MakeNailBat();
}

void AEnemyCharacterBase::SetupKronoleAddict()
{
	// Unpredictable: can enter Kronole mode, erratic behavior
	EnemyConfig.MaxHealth = 70.f;
	EnemyConfig.MoveSpeed = 480.f;
	EnemyConfig.BaseDamage = 14.f;
	EnemyConfig.DetectionRange = 1200.f;
	EnemyConfig.AttackRange = 160.f;
	EnemyConfig.bCanBeStealthKilled = true;
	EnemyConfig.StealthResistance = 0.2f;

	bCanUseKronole = true;

	StartingWeapon = UWeaponComponent::MakeShiv();
}

void AEnemyCharacterBase::SetupBreachman()
{
	// Tank: heavy armor, cold resistant, very slow but hits hard
	EnemyConfig.MaxHealth = 200.f;
	EnemyConfig.MoveSpeed = 300.f;
	EnemyConfig.BaseDamage = 25.f;
	EnemyConfig.DetectionRange = 1200.f;
	EnemyConfig.AttackRange = 200.f;
	EnemyConfig.bCanBeStealthKilled = false; // Too bulky for stealth kills
	EnemyConfig.StealthResistance = 1.f;

	// Armor: strong vs cold and physical, weak vs electrical
	EnemyConfig.ArmorValues.Add(EDamageType::Physical, 0.4f);
	EnemyConfig.ArmorValues.Add(EDamageType::Cold, 0.8f);
	EnemyConfig.ArmorValues.Add(EDamageType::Thermal, 0.2f);
	EnemyConfig.ArmorValues.Add(EDamageType::Electrical, 0.f);

	// Breachmen carry heavy tools
	FWeaponStats IcePick;
	IcePick.WeaponName = FName("Breachman's Ice Pick");
	IcePick.Category = EWeaponCategory::Piercing;
	IcePick.Tier = EWeaponTier::Military;
	IcePick.BaseDamage = 30.f;
	IcePick.AttackSpeed = 0.5f;
	IcePick.Range = 200.f;
	IcePick.StaminaCostPerSwing = 25.f;
	IcePick.Durability = 200.f;
	IcePick.MaxDurability = 200.f;
	IcePick.DurabilityLossPerHit = 0.5f;
	IcePick.DurabilityLossOnBlock = 1.f;
	StartingWeapon = IcePick;
}
