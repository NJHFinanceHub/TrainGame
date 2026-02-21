// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEEBossEncounterTrigger.h"
#include "TrainGame/Combat/BossCharacter.h"
#include "TrainGame/Combat/BossFightComponent.h"
#include "TrainGame/Combat/EnemyCharacter.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

ASEEBossEncounterTrigger::ASEEBossEncounterTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;
	TriggerVolume->SetBoxExtent(FVector(200.f, 300.f, 200.f));
	TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void ASEEBossEncounterTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoTriggerOnOverlap && TriggerVolume)
	{
		TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ASEEBossEncounterTrigger::OnTriggerOverlap);
	}
}

void ASEEBossEncounterTrigger::OnTriggerOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bEncounterActive || bEncounterComplete) return;

	// Only trigger for player characters
	ACharacter* PlayerChar = Cast<ACharacter>(OtherActor);
	if (!PlayerChar || !PlayerChar->IsPlayerControlled()) return;

	StartEncounter();
}

void ASEEBossEncounterTrigger::StartEncounter()
{
	if (bEncounterActive || bEncounterComplete) return;

	UWorld* World = GetWorld();
	if (!World || !BossClass) return;

	bEncounterActive = true;

	// Spawn boss
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnedBoss = World->SpawnActor<ABossCharacter>(BossClass, BossSpawnTransform, Params);

	if (SpawnedBoss)
	{
		UBossFightComponent* BossComp = SpawnedBoss->GetBossFightComponent();
		if (BossComp)
		{
			BossComp->OnBossDefeated.AddDynamic(this, &ASEEBossEncounterTrigger::OnBossDefeated);
		}
	}

	LockEncounterDoors();
	StartEncounterMusic();

	// Disable trigger to prevent re-entry
	if (TriggerVolume)
	{
		TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OnEncounterStarted.Broadcast();
}

void ASEEBossEncounterTrigger::EndEncounter()
{
	bEncounterActive = false;
	bEncounterComplete = true;

	UnlockEncounterDoors();
	StopEncounterMusic();

	for (AEnemyCharacter* Guard : SpawnedGuards)
	{
		if (IsValid(Guard))
		{
			Guard->Destroy();
		}
	}
	SpawnedGuards.Reset();

	OnEncounterCompleted.Broadcast();
}

void ASEEBossEncounterTrigger::OnBossDefeated()
{
	EndEncounter();
}
