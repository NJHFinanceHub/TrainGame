// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "BossCharacterBase.h"
#include "BossFightComponent.h"
#include "CombatComponent.h"
#include "TrainGame/Weapons/WeaponComponent.h"

ABossCharacterBase::ABossCharacterBase()
{
	BossFightComp = CreateDefaultSubobject<UBossFightComponent>(TEXT("BossFightComponent"));

	// Bosses cannot be stealth-killed
	FEnemyConfig& Config = const_cast<FEnemyConfig&>(GetEnemyConfig());
	Config.bCanBeStealthKilled = false;
	Config.StealthResistance = 1.f;
}

void ABossCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (BossFightComp)
	{
		BossFightComp->OnBossPhaseChanged.AddDynamic(this, &ABossCharacterBase::OnPhaseChanged);
		BossFightComp->OnBossDefeated.AddDynamic(this, &ABossCharacterBase::OnDefeated);
	}
}

void ABossCharacterBase::SetupBoss(EBossIdentity Identity)
{
	BossIdentity = Identity;

	if (BossFightComp)
	{
		BossFightComp->ApplyBossPreset(Identity);
	}

	// Apply appropriate enemy type
	switch (Identity)
	{
		case EBossIdentity::ThePitChampion:
			ApplyTypePreset(EEnemyType::TailFighter);
			break;
		case EBossIdentity::ChefPierrot:
			ApplyTypePreset(EEnemyType::TailFighter);
			break;
		case EBossIdentity::JackbootCommander:
			ApplyTypePreset(EEnemyType::JackbootCaptain);
			break;
		case EBossIdentity::KronoleLord:
			ApplyTypePreset(EEnemyType::KronoleAddict);
			break;
		case EBossIdentity::FirstClassDuelist:
			ApplyTypePreset(EEnemyType::FirstClassGuard);
			break;
		case EBossIdentity::OrderHighPriest:
			ApplyTypePreset(EEnemyType::OrderZealot);
			break;
		case EBossIdentity::MrWilford:
			ApplyTypePreset(EEnemyType::FirstClassGuard);
			break;
	}
}

void ABossCharacterBase::BeginEncounter()
{
	if (BossFightComp)
	{
		BossFightComp->InitBossFight();
	}

	UE_LOG(LogTemp, Log, TEXT("Boss encounter begun: %s"), *UEnum::GetValueAsString(BossIdentity));
}

void ABossCharacterBase::OnPhaseChanged(EBossPhase NewPhase, EBossPhase OldPhase)
{
	UE_LOG(LogTemp, Log, TEXT("Boss %s phase: %s -> %s"),
		*UEnum::GetValueAsString(BossIdentity),
		*UEnum::GetValueAsString(OldPhase),
		*UEnum::GetValueAsString(NewPhase));

	// Boss-specific phase behaviors would be implemented here or in Blueprint subclasses
}

void ABossCharacterBase::OnDefeated()
{
	UE_LOG(LogTemp, Log, TEXT("Boss %s defeated! Zone cleared."), *UEnum::GetValueAsString(BossIdentity));
	// In full implementation: trigger zone completion, loot drops, cutscene
}
