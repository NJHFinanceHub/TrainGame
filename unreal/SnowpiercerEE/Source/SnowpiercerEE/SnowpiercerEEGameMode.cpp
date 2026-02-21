// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SnowpiercerEEGameMode.h"
#include "SEEPlayerController.h"
#include "SEECharacter.h"
#include "SEEHUD.h"
#include "SEECarStreamingSubsystem.h"
#include "TrainGame/Settings/DifficultyTypes.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ASnowpiercerEEGameMode::ASnowpiercerEEGameMode()
{
	PlayerControllerClass = ASEEPlayerController::StaticClass();
	DefaultPawnClass = ASEECharacter::StaticClass();
	HUDClass = ASEEHUD::StaticClass();

	CurrentDifficulty = EDifficultyTier::Survivor;
	InitDifficultyModifiers();
}

void ASnowpiercerEEGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Register Zone 1 car sublevels on startup
	if (UWorld* World = GetWorld())
	{
		if (USEECarStreamingSubsystem* Streaming = World->GetSubsystem<USEECarStreamingSubsystem>())
		{
			Streaming->RegisterZone1Cars();
		}
	}
}

// --- Game Phase ---

void ASnowpiercerEEGameMode::SetGamePhase(EGamePhase NewPhase)
{
	if (CurrentPhase == NewPhase) return;

	EGamePhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;
	OnGamePhaseChanged.Broadcast(OldPhase, NewPhase);
}

// --- Difficulty ---

void ASnowpiercerEEGameMode::SetDifficulty(EDifficultyTier NewTier)
{
	if (CurrentDifficulty == NewTier) return;

	EDifficultyTier OldTier = CurrentDifficulty;
	CurrentDifficulty = NewTier;
	OnDifficultyChanged.Broadcast(OldTier, NewTier);
}

const FDifficultyModifiers& ASnowpiercerEEGameMode::GetDifficultyModifiers() const
{
	const FDifficultyModifiers* Found = DifficultyMap.Find(CurrentDifficulty);
	check(Found);
	return *Found;
}

void ASnowpiercerEEGameMode::InitDifficultyModifiers()
{
	// Passenger — story-focused, forgiving
	{
		FDifficultyModifiers M;
		M.EnemyHealthMultiplier = 0.7f;
		M.EnemyDamageMultiplier = 0.6f;
		M.EnemyDetectionRangeMultiplier = 0.8f;
		M.EnemyDetectionSpeedMultiplier = 0.7f;
		M.ResourceDropRateMultiplier = 1.5f;
		M.ResourceDegradationMultiplier = 0.5f;
		M.ColdExposureTimeMultiplier = 1.5f;
		M.StaminaDrainMultiplier = 0.7f;
		M.XPGainMultiplier = 1.2f;
		M.CompanionDownTimerSeconds = 60.f;
		M.FallDamageMultiplier = 0.5f;
		M.InjurySeverityShift = -1;
		M.StatCheckModifier = 2;
		M.DeathResourceLossFraction = 0.0f;
		M.bCompanionPermadeath = false;
		M.bPermadeath = false;
		M.bFriendlyFire = false;
		M.bShowQuestMarkers = true;
		M.bRestrictedSaving = false;
		DifficultyMap.Add(EDifficultyTier::Passenger, M);
	}

	// Survivor — intended experience
	{
		FDifficultyModifiers M;
		// All defaults (1.0 multipliers)
		DifficultyMap.Add(EDifficultyTier::Survivor, M);
	}

	// Eternal Engine — hardcore
	{
		FDifficultyModifiers M;
		M.EnemyHealthMultiplier = 1.4f;
		M.EnemyDamageMultiplier = 1.5f;
		M.EnemyDetectionRangeMultiplier = 1.3f;
		M.EnemyDetectionSpeedMultiplier = 1.3f;
		M.ResourceDropRateMultiplier = 0.6f;
		M.ResourceDegradationMultiplier = 1.5f;
		M.ColdExposureTimeMultiplier = 0.7f;
		M.StaminaDrainMultiplier = 1.3f;
		M.XPGainMultiplier = 0.8f;
		M.CompanionDownTimerSeconds = 15.f;
		M.FallDamageMultiplier = 1.5f;
		M.InjurySeverityShift = 1;
		M.StatCheckModifier = -1;
		M.DeathResourceLossFraction = 0.25f;
		M.bCompanionPermadeath = true;
		M.bFriendlyFire = true;
		M.FriendlyFireDamageMultiplier = 0.5f;
		M.bShowQuestMarkers = false;
		M.bRestrictedSaving = true;
		DifficultyMap.Add(EDifficultyTier::EternalEngine, M);
	}

	// Mr. Wilford — permadeath
	{
		FDifficultyModifiers M;
		M.EnemyHealthMultiplier = 1.6f;
		M.EnemyDamageMultiplier = 2.0f;
		M.EnemyDetectionRangeMultiplier = 1.5f;
		M.EnemyDetectionSpeedMultiplier = 1.5f;
		M.ResourceDropRateMultiplier = 0.4f;
		M.ResourceDegradationMultiplier = 2.0f;
		M.ColdExposureTimeMultiplier = 0.5f;
		M.StaminaDrainMultiplier = 1.5f;
		M.XPGainMultiplier = 0.6f;
		M.CompanionDownTimerSeconds = 10.f;
		M.FallDamageMultiplier = 2.0f;
		M.InjurySeverityShift = 2;
		M.StatCheckModifier = -2;
		M.DeathResourceLossFraction = 1.0f;
		M.bCompanionPermadeath = true;
		M.bPermadeath = true;
		M.bFriendlyFire = true;
		M.FriendlyFireDamageMultiplier = 1.0f;
		M.bShowQuestMarkers = false;
		M.bRestrictedSaving = true;
		DifficultyMap.Add(EDifficultyTier::MrWilford, M);
	}
}

// --- Zone / Car ---

void ASnowpiercerEEGameMode::OnPlayerEnteredCar(int32 CarIndex)
{
	CurrentCarIndex = CarIndex;
	VisitedCars.Add(CarIndex);

	if (UWorld* World = GetWorld())
	{
		if (USEECarStreamingSubsystem* Streaming = World->GetSubsystem<USEECarStreamingSubsystem>())
		{
			Streaming->EnterCar(CarIndex);
		}
	}

	OnCarEntered.Broadcast(CarIndex);
}

void ASnowpiercerEEGameMode::BeginZoneTransition(ESEETrainZone NewZone)
{
	PendingZone = NewZone;
	SetGamePhase(EGamePhase::Transition);
}

void ASnowpiercerEEGameMode::CompleteZoneTransition()
{
	ESEETrainZone OldZone = CurrentZone;
	CurrentZone = PendingZone;
	SetGamePhase(EGamePhase::Exploration);
	OnZoneTransition.Broadcast(OldZone, CurrentZone);
}

// --- Checkpoint / Respawn ---

void ASnowpiercerEEGameMode::SetCheckpoint(int32 CarIndex, FTransform Transform)
{
	LastCheckpointCarIndex = CarIndex;
	LastCheckpointTransform = Transform;
}

void ASnowpiercerEEGameMode::RespawnPlayer()
{
	DeathCount++;

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	APawn* Pawn = PC->GetPawn();
	if (Pawn)
	{
		Pawn->SetActorTransform(LastCheckpointTransform);
	}

	OnPlayerEnteredCar(LastCheckpointCarIndex);
	SetGamePhase(EGamePhase::Exploration);
}
