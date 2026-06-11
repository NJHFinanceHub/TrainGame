// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCBrainSubsystem.h"

#include "SEENPCAIController.h"
#include "SnowpiercerEE/SEECharacter.h"
#include "SnowpiercerEE/SEECompanionCharacter.h"
#include "SnowpiercerEE/SEEHealthComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "TrainGame/AI/SEENPCCharacter.h"

namespace
{
	constexpr float GScanInterval = 2.0f;
	constexpr float GFirstScanDelay = 0.5f;
}

void USEENPCBrainSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (!InWorld.IsGameWorld()) return;

	// Repeating scan: catches both the initially placed NPCs and any pawns
	// that stream in later with their train car.
	InWorld.GetTimerManager().SetTimer(ScanTimerHandle, this,
		&USEENPCBrainSubsystem::ScanForUnpossessedNPCs, GScanInterval, true, GFirstScanDelay);
}

void USEENPCBrainSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ScanTimerHandle);
	}
	Super::Deinitialize();
}

void USEENPCBrainSubsystem::ScanForUnpossessedNPCs()
{
	UWorld* World = GetWorld();
	if (!World) return;

	for (TActorIterator<ACharacter> It(World); It; ++It)
	{
		ACharacter* PawnChar = *It;
		if (!IsValid(PawnChar)) continue;
		if (PawnChar->GetController() != nullptr) continue; // player, brain, or other AI already owns it
		if (!ShouldAdoptPawn(PawnChar)) continue;

		// Never adopt a corpse (the brain unpossesses on death; downed == dead for NPCs).
		if (const USEEHealthComponent* Health = PawnChar->FindComponentByClass<USEEHealthComponent>())
		{
			if (Health->IsDead() || Health->IsDowned()) continue;
		}

		AdoptPawn(PawnChar);
	}
}

bool USEENPCBrainSubsystem::ShouldAdoptPawn(const ACharacter* PawnChar)
{
	// Player and companions have their own control schemes.
	if (PawnChar->IsA<ASEECharacter>()) return false;
	if (PawnChar->IsA<ASEECompanionCharacter>()) return false;

	// C++ NPC base (covers future reparented blueprints).
	if (PawnChar->IsA<ASEENPCCharacter>()) return true;

	// The placed plain-ACharacter blueprints: BP_NPC_Civilian_C, BP_NPC_Jackboot_C, ...
	if (PawnChar->GetClass()->GetName().StartsWith(TEXT("BP_NPC_"))) return true;

	// Population-script labels (NPC_Gilliam, Boss_CmdrGrey, ...).
	const FString Label = PawnChar->GetActorNameOrLabel();
	return Label.StartsWith(TEXT("NPC_")) || Label.StartsWith(TEXT("Boss_"));
}

void USEENPCBrainSubsystem::AdoptPawn(ACharacter* PawnChar)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient; // runtime-only, never saved into the level

	ASEENPCAIController* Controller = World->SpawnActor<ASEENPCAIController>(
		ASEENPCAIController::StaticClass(),
		PawnChar->GetActorLocation(), PawnChar->GetActorRotation(), SpawnParams);
	if (!Controller) return;

	// Configure before possession so OnPossess applies the right stats/health.
	ConfigureController(Controller, PawnChar);
	Controller->Possess(PawnChar);
}

void USEENPCBrainSubsystem::ConfigureController(ASEENPCAIController* Controller, const ACharacter* PawnChar)
{
	const FString ClassName = PawnChar->GetClass()->GetName();
	const FString Label = PawnChar->GetActorNameOrLabel();
	const FString Identity = Label + TEXT("|") + ClassName;

	const bool bJackboot = ClassName.StartsWith(TEXT("BP_NPC_Jackboot")) || Identity.Contains(TEXT("Jackboot"));
	const bool bMerchant = ClassName.StartsWith(TEXT("BP_NPC_Merchant")) || Identity.Contains(TEXT("Merchant"));
	const bool bBreachman = ClassName.StartsWith(TEXT("BP_NPC_Breachman")) || Identity.Contains(TEXT("Breachman"));
	const bool bBoss = Label.Contains(TEXT("Boss"));

	if (bBoss)
	{
		// Commander Grey: same brain, heavier numbers, slightly faster swing.
		Controller->bHostile = true;
		Controller->MaxHealth = 400.0f;
		Controller->AttackDamage = 30.0f;
		Controller->AttackRange = 220.0f;
		Controller->AttackIntervalMin = 0.9f;
		Controller->AttackIntervalMax = 1.3f;
		Controller->ChaseSpeed = 550.0f;
		Controller->SightRange = 3000.0f;
		Controller->DialogueEntryNode = NAME_None; // bosses do their talking with a baton
	}
	else if (bJackboot)
	{
		Controller->bHostile = true;
		Controller->MaxHealth = 150.0f;
		Controller->AttackDamage = 20.0f;
		Controller->AttackRange = 200.0f;
		Controller->SightRange = 2500.0f;
		Controller->ChaseSpeed = 500.0f;
		Controller->WalkSpeed = 220.0f;
		// Talkable while un-aggroed: the gate-guard tree.
		Controller->DialogueEntryNode = TEXT("Guard_01");
	}
	else if (bMerchant)
	{
		Controller->bHostile = false;
		Controller->MaxHealth = 100.0f;
		Controller->WanderRadius = 0.0f; // stays behind the stall
		Controller->DialogueEntryNode = PickDialogueEntryNode(Identity, true, false);
	}
	else
	{
		// Civilians, Breachmen, FirstClass and anything unrecognized: friendly wanderer.
		Controller->bHostile = false;
		Controller->MaxHealth = bBreachman ? 120.0f : 60.0f;
		Controller->WanderRadius = 600.0f;
		Controller->WalkSpeed = 180.0f;
		Controller->DialogueEntryNode = PickDialogueEntryNode(Identity, false, bBreachman);
	}

	Controller->MarkConfigured();
}

FName USEENPCBrainSubsystem::PickDialogueEntryNode(const FString& Identity, bool bMerchant, bool bBreachman)
{
	// Named characters first (labels set by populate_zone1.py), then class fallbacks.
	// Entry rows live in /Game/DataTables/DT_Dialogue_Zone1 (see create_datatables.py):
	//   Pike_01 (Old Man Pike), Dealer_01 (Kronole Dealer), Guard_01 (Jackboot Guard),
	//   Mechanic_01 (Workshop Mechanic), Injured_01 (Injured Tailie).
	if (Identity.Contains(TEXT("Gilliam")) || Identity.Contains(TEXT("Elder")) || Identity.Contains(TEXT("Whisper")))
	{
		return TEXT("Pike_01");
	}
	if (Identity.Contains(TEXT("Kronole")) || Identity.Contains(TEXT("Smuggler")) || Identity.Contains(TEXT("Dealer")))
	{
		return TEXT("Dealer_01");
	}
	if (Identity.Contains(TEXT("Workshop")) || Identity.Contains(TEXT("Mechanic")) || Identity.Contains(TEXT("Car07")))
	{
		return TEXT("Mechanic_01");
	}
	if (bMerchant) return TEXT("Dealer_01");
	if (bBreachman) return TEXT("Mechanic_01");

	// DrAsha, Mourner and generic civilians: the injured-Tailie quest tree.
	return TEXT("Injured_01");
}
