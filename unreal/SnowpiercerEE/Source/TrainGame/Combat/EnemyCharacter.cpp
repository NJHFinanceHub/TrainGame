// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "EnemyCharacter.h"
#include "CombatComponent.h"
#include "CombatAIController.h"
#include "TrainGame/Weapons/WeaponComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	// Default to Jackboot grunt
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ACombatAIController::StaticClass();
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	ApplyArchetypeStats();
}

void AEnemyCharacter::InitializeArchetype(EEnemyType Type)
{
	EnemyStats.EnemyType = Type;
	ApplyArchetypeStats();
}

void AEnemyCharacter::ApplyArchetypeStats()
{
	switch (EnemyStats.EnemyType)
	{
	case EEnemyType::JackbootGrunt:		SetupJackbootGrunt(); break;
	case EEnemyType::JackbootCaptain:	SetupJackbootCaptain(); break;
	case EEnemyType::OrderZealot:		SetupOrderZealot(); break;
	case EEnemyType::FirstClassGuard:	SetupFirstClassGuard(); break;
	case EEnemyType::TailScrapper:		SetupTailScrapper(); break;
	}

	// Apply weapon
	if (WeaponComp)
	{
		WeaponComp->EquipWeapon(DefaultWeapon);
	}

	// Apply damage resistances to combat component
	if (CombatComp)
	{
		for (auto& Pair : EnemyStats.DamageResistances)
		{
			CombatComp->SetDamageResistance(Pair.Key, Pair.Value);
		}
	}

	// Set AI profile on controller
	ACombatAIController* AIController = Cast<ACombatAIController>(GetController());
	if (AIController)
	{
		AIController->SetProfile(EnemyStats);
	}
}

void AEnemyCharacter::SetupJackbootGrunt()
{
	EnemyStats.Health = 80.f;
	EnemyStats.DamageMultiplier = 1.0f;
	EnemyStats.MovementSpeed = 600.f;
	EnemyStats.bCanBeTakenDown = true;
	EnemyStats.TakedownStrengthRequired = 0;
	EnemyStats.bCallsReinforcements = true;
	EnemyStats.ReinforcementDelay = 15.f;

	// Standard baton
	DefaultWeapon = UWeaponComponent::MakeJackbootBaton();
}

void AEnemyCharacter::SetupJackbootCaptain()
{
	EnemyStats.Health = 150.f;
	EnemyStats.DamageMultiplier = 1.3f;
	EnemyStats.MovementSpeed = 550.f;
	EnemyStats.bCanBeTakenDown = true;
	EnemyStats.TakedownStrengthRequired = 12;
	EnemyStats.bCallsReinforcements = true;
	EnemyStats.ReinforcementDelay = 8.f;

	// Captains have better equipment — reinforced axe
	DefaultWeapon = UWeaponComponent::MakeReinforcedAxe();

	// Partial blunt resistance (armor)
	EnemyStats.DamageResistances.Add(EDamageType::Blunt, 0.7f);
	EnemyStats.DamageResistances.Add(EDamageType::Physical, 0.8f);
}

void AEnemyCharacter::SetupOrderZealot()
{
	// Fanatical followers of Wilford — high damage, unpredictable, immune to fear
	EnemyStats.Health = 100.f;
	EnemyStats.DamageMultiplier = 1.5f;
	EnemyStats.MovementSpeed = 650.f;
	EnemyStats.bCanBeTakenDown = true;
	EnemyStats.TakedownStrengthRequired = 10;
	EnemyStats.bCallsReinforcements = false; // Fight alone, don't retreat

	// Ceremonial bladed weapon
	DefaultWeapon.WeaponName = FName("CeremonialBlade");
	DefaultWeapon.Category = EWeaponCategory::Bladed;
	DefaultWeapon.Tier = EWeaponTier::Specialized;
	DefaultWeapon.BaseDamage = 25.f;
	DefaultWeapon.AttackSpeed = 1.3f;
	DefaultWeapon.Range = 160.f;
	DefaultWeapon.StaminaCostPerSwing = 12.f;
	DefaultWeapon.Durability = 200.f;
	DefaultWeapon.MaxDurability = 200.f;
	DefaultWeapon.DurabilityLossPerHit = 1.f;
	DefaultWeapon.DurabilityLossOnBlock = 2.f;

	// Resistance to cold (zealous devotion keeps them warm)
	EnemyStats.DamageResistances.Add(EDamageType::Cold, 0.5f);
	// Weakness to fire
	EnemyStats.DamageResistances.Add(EDamageType::Fire, 1.5f);
}

void AEnemyCharacter::SetupFirstClassGuard()
{
	// Elite guards protecting First Class — best equipment, high defense
	EnemyStats.Health = 120.f;
	EnemyStats.DamageMultiplier = 1.2f;
	EnemyStats.MovementSpeed = 500.f;
	EnemyStats.bCanBeTakenDown = true;
	EnemyStats.TakedownStrengthRequired = 14;
	EnemyStats.bCallsReinforcements = true;
	EnemyStats.ReinforcementDelay = 5.f; // Fast response in First Class

	// Military-grade electrified baton
	DefaultWeapon.WeaponName = FName("ElectrifiedBaton");
	DefaultWeapon.Category = EWeaponCategory::Blunt;
	DefaultWeapon.Tier = EWeaponTier::Military;
	DefaultWeapon.BaseDamage = 20.f;
	DefaultWeapon.AttackSpeed = 1.0f;
	DefaultWeapon.Range = 170.f;
	DefaultWeapon.StaminaCostPerSwing = 10.f;
	DefaultWeapon.Durability = 300.f;
	DefaultWeapon.MaxDurability = 300.f;
	DefaultWeapon.DurabilityLossPerHit = 0.5f;
	DefaultWeapon.DurabilityLossOnBlock = 1.f;

	// Heavy armor — resistant to most physical damage
	EnemyStats.DamageResistances.Add(EDamageType::Blunt, 0.6f);
	EnemyStats.DamageResistances.Add(EDamageType::Bladed, 0.7f);
	EnemyStats.DamageResistances.Add(EDamageType::Piercing, 0.8f);
	EnemyStats.DamageResistances.Add(EDamageType::Physical, 0.6f);
	// Vulnerable to electrical (conductive armor)
	EnemyStats.DamageResistances.Add(EDamageType::Electrical, 1.5f);
}

void AEnemyCharacter::SetupTailScrapper()
{
	// Desperate fighters from the Tail — low health, fast, dirty fighting
	EnemyStats.Health = 60.f;
	EnemyStats.DamageMultiplier = 0.8f;
	EnemyStats.MovementSpeed = 700.f;
	EnemyStats.bCanBeTakenDown = true;
	EnemyStats.TakedownStrengthRequired = 0;
	EnemyStats.bCallsReinforcements = false;

	// Improvised shiv
	DefaultWeapon = UWeaponComponent::MakeShiv();
}

void AEnemyCharacter::AlertEnemy(AActor* AlertSource)
{
	if (bIsAlerted || bIsUnconscious) return;

	bIsAlerted = true;
	OnEnemyAlerted.Broadcast(this);

	// Call for help if this type does that
	if (EnemyStats.bCallsReinforcements && !bHasCalledReinforcements)
	{
		CallReinforcements();
	}
}

void AEnemyCharacter::CallReinforcements()
{
	if (bHasCalledReinforcements) return;

	bHasCalledReinforcements = true;
	OnReinforcementsCalled.Broadcast(this, EnemyStats.ReinforcementDelay);
}

bool AEnemyCharacter::CanBeTakenDown(int32 AttackerStrength) const
{
	if (!EnemyStats.bCanBeTakenDown) return false;
	if (bIsUnconscious) return false;
	if (bIsAlerted) return false; // Can't takedown alerted enemies
	return AttackerStrength >= EnemyStats.TakedownStrengthRequired;
}

void AEnemyCharacter::ReceiveTakedown(bool bLethal, AActor* Attacker)
{
	if (bLethal)
	{
		// Instant kill via stealth
		if (CombatComp)
		{
			CombatComp->ReceiveAttack(9999.f, EAttackDirection::Mid, EDamageType::StealthTakedown, Attacker);
		}
	}
	else
	{
		// Knockout — unconscious state
		bIsUnconscious = true;
		if (CombatComp)
		{
			CombatComp->ReceiveAttack(0.f, EAttackDirection::Mid, EDamageType::NonLethal, Attacker);
		}
	}

	OnEnemyDowned.Broadcast(this);
}
