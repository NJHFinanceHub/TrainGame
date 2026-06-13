#include "SEESaveGameSubsystem.h"
#include "Endings/SEELedgerSubsystem.h"
#include "SEEFactionManager.h"
#include "SEEHealthComponent.h"
#include "SEEStatsComponent.h"
#include "SEECharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

namespace
{
	// How often / how long the pending-restore timer polls for the player pawn.
	constexpr float GPendingPollInterval = 0.25f;
	constexpr int32 GPendingPollMaxAttempts = 80; // ~20s before giving up
}

void USEESaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadFromSlot();
}

void USEESaveGameSubsystem::Deinitialize()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			World->GetTimerManager().ClearTimer(PendingRestoreTimer);
		}
	}
	Super::Deinitialize();
}

void USEESaveGameSubsystem::SetCarState(int32 CarIndex, const FSEECarState& State)
{
	if (CarIndex < 0)
	{
		return;
	}

	RuntimeCarStates.Add(CarIndex, State);
}

bool USEESaveGameSubsystem::GetCarState(int32 CarIndex, FSEECarState& OutState) const
{
	if (const FSEECarState* Found = RuntimeCarStates.Find(CarIndex))
	{
		OutState = *Found;
		return true;
	}

	return false;
}

bool USEESaveGameSubsystem::DoesSaveGameExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex);
}

APawn* USEESaveGameSubsystem::GetPlayerPawn() const
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	return PC ? PC->GetPawn() : nullptr;
}

bool USEESaveGameSubsystem::HasSaveAuthority() const
{
	// CO-OP: the save is host-authoritative — only the host (listen/dedicated
	// server) or a standalone game writes the slot. A connected client writing its
	// local subsystem would persist a partial/bogus world snapshot over the shared
	// save, so client WriteToSlot is a no-op.
	const UGameInstance* GI = GetGameInstance();
	const UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return true; // no world yet — treat as authority (standalone boot / menu)
	}
	return World->GetNetMode() != NM_Client;
}

bool USEESaveGameSubsystem::WriteToSlot()
{
	// Host-authoritative: clients don't write the shared save. Standalone is
	// authority, so single-player save behaves exactly as before.
	if (!HasSaveAuthority())
	{
		return false;
	}

	USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
		UGameplayStatics::CreateSaveGameObject(USEESaveGameData::StaticClass()));

	if (!SaveObj)
	{
		return false;
	}

	SaveObj->CarStates = RuntimeCarStates;

	// --- Ledger (choices / flags) ---
	if (USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>())
	{
		SaveObj->LedgerScores = Ledger->GetScoresForSave();
		SaveObj->GlobalFlags = Ledger->GetGlobalFlagsForSave();
		SaveObj->GlobalIntFlags = Ledger->GetGlobalIntFlagsForSave();
		SaveObj->ChoiceHistory = Ledger->GetHistoryForSave();
	}

	// --- Quests ---
	if (USEEQuestManager* Quests = GetGameInstance()->GetSubsystem<USEEQuestManager>())
	{
		SaveObj->QuestStates = Quests->CaptureQuestSaveState();
	}

	// --- Factions ---
	if (USEEFactionManager* Factions = GetGameInstance()->GetSubsystem<USEEFactionManager>())
	{
		SaveObj->FactionState = Factions->GetSaveState();
	}

	// --- Host player pawn slice (transform / health / stamina / inventory / armor / stats) ---
	// CapturePlayerState uses GetPlayerPawn() = the local (player 0 / host) pawn, so
	// the host's slice still lands in the flat Player* fields — single-player format
	// is unchanged.
	SaveObj->bHasPlayerState = CapturePlayerState(SaveObj);

	// --- CO-OP: every OTHER connected player's slice (host-authoritative capture) ---
	// No-op in single-player (only one controller, which is the host).
	CaptureConnectedPlayers(SaveObj);

	return UGameplayStatics::SaveGameToSlot(SaveObj, SaveSlotName, UserIndex);
}

void USEESaveGameSubsystem::CapturePawnEntry(APawn* Pawn, FSEEPlayerSaveEntry& OutEntry)
{
	if (!Pawn)
	{
		return;
	}

	OutEntry.PlayerTransform = Pawn->GetActorTransform();

	// Health (component lookup is null-safe per slice).
	if (USEEHealthComponent* Health = Pawn->FindComponentByClass<USEEHealthComponent>())
	{
		OutEntry.PlayerHealth = Health->GetCurrentHealth();
		OutEntry.PlayerMaxHealth = Health->GetMaxHealth();
	}

	// Stamina lives on the character.
	if (ASEECharacter* Character = Cast<ASEECharacter>(Pawn))
	{
		OutEntry.PlayerStamina = Character->GetStamina();
		OutEntry.PlayerMaxStamina = Character->GetMaxStamina();
	}

	// Stats (XP / level).
	if (USEEStatsComponent* Stats = Pawn->FindComponentByClass<USEEStatsComponent>())
	{
		OutEntry.PlayerXP = Stats->GetCurrentXP();
		OutEntry.PlayerLevel = Stats->GetLevel();
	}

	// Inventory.
	if (USEEInventoryComponent* Inventory = Pawn->FindComponentByClass<USEEInventoryComponent>())
	{
		OutEntry.Inventory = Inventory->GetSaveState();
	}

	// Equipped armor.
	if (UArmorComponent* Armor = Pawn->FindComponentByClass<UArmorComponent>())
	{
		OutEntry.EquippedArmor = Armor->GetSaveState();
	}
}

bool USEESaveGameSubsystem::CapturePlayerState(USEESaveGameData* SaveObj)
{
	APawn* Pawn = GetPlayerPawn();
	if (!Pawn || !SaveObj)
	{
		return false;
	}

	// Capture into a shared entry, then copy into the flat host fields. Keeping the
	// flat fields preserves the existing single-player save format byte-for-byte.
	FSEEPlayerSaveEntry Entry;
	CapturePawnEntry(Pawn, Entry);

	SaveObj->PlayerTransform = Entry.PlayerTransform;
	SaveObj->PlayerHealth = Entry.PlayerHealth;
	SaveObj->PlayerMaxHealth = Entry.PlayerMaxHealth;
	SaveObj->PlayerStamina = Entry.PlayerStamina;
	SaveObj->PlayerMaxStamina = Entry.PlayerMaxStamina;
	SaveObj->PlayerXP = Entry.PlayerXP;
	SaveObj->PlayerLevel = Entry.PlayerLevel;
	SaveObj->Inventory = Entry.Inventory;
	SaveObj->EquippedArmor = Entry.EquippedArmor;

	return true;
}

void USEESaveGameSubsystem::CaptureConnectedPlayers(USEESaveGameData* SaveObj)
{
	if (!SaveObj)
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return;
	}

	// The host's pawn is already captured into the flat fields (it is player 0 =
	// World->GetFirstPlayerController()). Capture every OTHER connected player's
	// pawn here. Iterating controllers (not just player 0) is what makes the save
	// multi-player; in single-player there is only the host controller, so this
	// adds nothing and the save is unchanged.
	APlayerController* HostPC = World->GetFirstPlayerController();
	const APawn* HostPawn = HostPC ? HostPC->GetPawn() : nullptr;

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC)
		{
			continue;
		}
		APawn* Pawn = PC->GetPawn();
		if (!Pawn || Pawn == HostPawn)
		{
			continue; // host already captured into the flat fields
		}

		FSEEPlayerSaveEntry& Entry = SaveObj->ConnectedPlayers.AddDefaulted_GetRef();
		CapturePawnEntry(Pawn, Entry);
	}
}

bool USEESaveGameSubsystem::LoadFromSlot()
{
	// CO-OP: load is host-authoritative. The host loads the save and the restored
	// world (car states, quests, factions, pawns) replicates to clients; a client
	// loading its own local save would fight the replicated state. Standalone is
	// authority, so single-player load runs exactly as before. (Per-client restore
	// of each guest's own pawn slice from ConnectedPlayers is deferred — see report.)
	if (!HasSaveAuthority())
	{
		return false;
	}

	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
	{
		return false;
	}

	USEESaveGameData* SaveObj = Cast<USEESaveGameData>(
		UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

	if (!SaveObj)
	{
		return false;
	}

	RuntimeCarStates = SaveObj->CarStates;

	// --- Ledger ---
	if (USEELedgerSubsystem* Ledger = GetGameInstance()->GetSubsystem<USEELedgerSubsystem>())
	{
		Ledger->LoadScoresFromSave(SaveObj->LedgerScores);
		Ledger->LoadGlobalFlagsFromSave(SaveObj->GlobalFlags);
		Ledger->LoadGlobalIntFlagsFromSave(SaveObj->GlobalIntFlags);
		Ledger->LoadHistoryFromSave(SaveObj->ChoiceHistory);
	}

	// --- Quests ---
	if (USEEQuestManager* Quests = GetGameInstance()->GetSubsystem<USEEQuestManager>())
	{
		Quests->RestoreQuestSaveState(SaveObj->QuestStates);
	}

	// --- Factions ---
	if (USEEFactionManager* Factions = GetGameInstance()->GetSubsystem<USEEFactionManager>())
	{
		Factions->SetSaveState(SaveObj->FactionState);
	}

	// --- Player slice: stash as pending; the pawn may not exist yet (main-menu load). ---
	if (SaveObj->bHasPlayerState)
	{
		bHasPendingPlayerState = true;
		PendingTransform = SaveObj->PlayerTransform;
		PendingHealth = SaveObj->PlayerHealth;
		PendingMaxHealth = SaveObj->PlayerMaxHealth;
		PendingStamina = SaveObj->PlayerStamina;
		PendingXP = SaveObj->PlayerXP;
		PendingInventory = SaveObj->Inventory;
		PendingArmor = SaveObj->EquippedArmor;

		// Try immediately (in-level load), otherwise poll until the pawn spawns.
		if (APawn* Pawn = GetPlayerPawn())
		{
			ApplyPendingPlayerStateToPawn(Pawn);
		}
		else
		{
			StartPendingRestorePoll();
		}
	}

	return true;
}

void USEESaveGameSubsystem::StartPendingRestorePoll()
{
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	if (!World)
	{
		return; // no world yet; the in-level LoadFromSlot path / BeginPlay hook still covers it
	}

	PendingRestorePolls = 0;
	World->GetTimerManager().SetTimer(
		PendingRestoreTimer, this, &USEESaveGameSubsystem::PollForPlayerPawn,
		GPendingPollInterval, /*bLoop*/ true);
}

void USEESaveGameSubsystem::PollForPlayerPawn()
{
	++PendingRestorePolls;

	if (!bHasPendingPlayerState)
	{
		// Already applied (e.g. via an external BeginPlay hook) — stop polling.
		if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
		{
			World->GetTimerManager().ClearTimer(PendingRestoreTimer);
		}
		return;
	}

	if (APawn* Pawn = GetPlayerPawn())
	{
		ApplyPendingPlayerStateToPawn(Pawn); // clears the pending flag + timer
		return;
	}

	if (PendingRestorePolls >= GPendingPollMaxAttempts)
	{
		if (UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr)
		{
			World->GetTimerManager().ClearTimer(PendingRestoreTimer);
		}
	}
}

void USEESaveGameSubsystem::ApplyPendingPlayerStateToPawn(APawn* Pawn)
{
	if (!bHasPendingPlayerState || !Pawn)
	{
		return;
	}

	// Transform.
	Pawn->SetActorTransform(PendingTransform, /*bSweep*/ false, nullptr, ETeleportType::TeleportPhysics);

	// Inventory.
	if (USEEInventoryComponent* Inventory = Pawn->FindComponentByClass<USEEInventoryComponent>())
	{
		Inventory->SetSaveState(PendingInventory);
	}

	// Equipped armor.
	if (UArmorComponent* Armor = Pawn->FindComponentByClass<UArmorComponent>())
	{
		Armor->SetSaveState(PendingArmor);
	}

	// Stats: only XP/level are persisted. No public setter exists, so re-derive
	// XP by adding the saved total onto a fresh component (level 1 / 0 XP at
	// spawn). AddXP drives level-ups internally so level falls out correctly.
	if (USEEStatsComponent* Stats = Pawn->FindComponentByClass<USEEStatsComponent>())
	{
		const int32 XPDelta = PendingXP - Stats->GetCurrentXP();
		if (XPDelta > 0)
		{
			Stats->AddXP(XPDelta);
		}
	}

	// Health: the component has no direct setter. Heal up to the saved value;
	// if the saved value is below the spawn health, knock it down with a
	// non-typed (Environmental = no armor mitigation) damage tick. Guarded so a
	// restore never drops the player to a lethal/zero state on load.
	if (USEEHealthComponent* Health = Pawn->FindComponentByClass<USEEHealthComponent>())
	{
		if (PendingHealth > 0.0f)
		{
			const float Current = Health->GetCurrentHealth();
			if (PendingHealth > Current)
			{
				Health->Heal(PendingHealth - Current);
			}
			else if (PendingHealth < Current)
			{
				// Leave at least 1 HP to avoid triggering death on load.
				const float SafeTarget = FMath::Max(PendingHealth, 1.0f);
				const float Reduce = Current - SafeTarget;
				if (Reduce > 0.0f)
				{
					Health->TakeDamage(Reduce, ESEEDamageType::Environmental, nullptr);
				}
			}
		}
	}

	// Stamina: ASEECharacter exposes ConsumeStamina (drain) but no restore. The
	// pawn spawns at full stamina, so drain down to the saved value.
	if (ASEECharacter* Character = Cast<ASEECharacter>(Pawn))
	{
		const float CurrentStamina = Character->GetStamina();
		if (PendingStamina < CurrentStamina)
		{
			Character->ConsumeStamina(CurrentStamina - PendingStamina);
		}
	}

	// Consume the pending blob and stop polling.
	bHasPendingPlayerState = false;
	PendingInventory.Reset();
	PendingArmor.Reset();

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UWorld* World = GI->GetWorld())
		{
			World->GetTimerManager().ClearTimer(PendingRestoreTimer);
		}
	}
}
