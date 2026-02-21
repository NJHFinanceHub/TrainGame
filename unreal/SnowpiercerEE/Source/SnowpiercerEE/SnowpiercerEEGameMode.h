// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SEETypes.h"
#include "SnowpiercerEEGameMode.generated.h"

struct FDifficultyModifiers;
enum class EDifficultyTier : uint8;

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	Exploration		UMETA(DisplayName = "Exploration"),
	Combat			UMETA(DisplayName = "Combat"),
	Dialogue		UMETA(DisplayName = "Dialogue"),
	Cutscene		UMETA(DisplayName = "Cutscene"),
	BossFight		UMETA(DisplayName = "Boss Fight"),
	Transition		UMETA(DisplayName = "Zone Transition"),
	Paused			UMETA(DisplayName = "Paused")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGamePhaseChanged, EGamePhase, OldPhase, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnZoneTransition, ESEETrainZone, OldZone, ESEETrainZone, NewZone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCarEntered, int32, CarIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifficultyChangedGM, EDifficultyTier, OldTier, EDifficultyTier, NewTier);

UCLASS()
class SNOWPIERCEREE_API ASnowpiercerEEGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASnowpiercerEEGameMode();

	// --- Game Phase ---

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetGamePhase(EGamePhase NewPhase);

	UFUNCTION(BlueprintPure, Category = "GameMode")
	EGamePhase GetGamePhase() const { return CurrentPhase; }

	// --- Difficulty ---

	UFUNCTION(BlueprintCallable, Category = "GameMode|Difficulty")
	void SetDifficulty(EDifficultyTier NewTier);

	UFUNCTION(BlueprintPure, Category = "GameMode|Difficulty")
	EDifficultyTier GetDifficulty() const { return CurrentDifficulty; }

	UFUNCTION(BlueprintPure, Category = "GameMode|Difficulty")
	const FDifficultyModifiers& GetDifficultyModifiers() const;

	// --- Zone / Car ---

	UFUNCTION(BlueprintPure, Category = "GameMode|Zone")
	ESEETrainZone GetCurrentZone() const { return CurrentZone; }

	UFUNCTION(BlueprintPure, Category = "GameMode|Zone")
	int32 GetCurrentCarIndex() const { return CurrentCarIndex; }

	UFUNCTION(BlueprintCallable, Category = "GameMode|Zone")
	void OnPlayerEnteredCar(int32 CarIndex);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Zone")
	void BeginZoneTransition(ESEETrainZone NewZone);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Zone")
	void CompleteZoneTransition();

	UFUNCTION(BlueprintPure, Category = "GameMode|Zone")
	bool HasVisitedCar(int32 CarIndex) const { return VisitedCars.Contains(CarIndex); }

	// --- Checkpoint / Respawn ---

	UFUNCTION(BlueprintCallable, Category = "GameMode|Checkpoint")
	void SetCheckpoint(int32 CarIndex, FTransform Transform);

	UFUNCTION(BlueprintCallable, Category = "GameMode|Checkpoint")
	void RespawnPlayer();

	UFUNCTION(BlueprintPure, Category = "GameMode|Checkpoint")
	int32 GetDeathCount() const { return DeathCount; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnGamePhaseChanged OnGamePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnZoneTransition OnZoneTransition;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnCarEntered OnCarEntered;

	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnDifficultyChangedGM OnDifficultyChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State")
	EGamePhase CurrentPhase = EGamePhase::Exploration;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State")
	ESEETrainZone CurrentZone = ESEETrainZone::Tail;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State")
	int32 CurrentCarIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|State")
	TSet<int32> VisitedCars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameMode|Difficulty")
	EDifficultyTier CurrentDifficulty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|Checkpoint")
	int32 LastCheckpointCarIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|Checkpoint")
	FTransform LastCheckpointTransform;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameMode|Checkpoint")
	int32 DeathCount = 0;

private:
	void InitDifficultyModifiers();

	TMap<EDifficultyTier, FDifficultyModifiers> DifficultyMap;
	ESEETrainZone PendingZone = ESEETrainZone::Tail;
};
