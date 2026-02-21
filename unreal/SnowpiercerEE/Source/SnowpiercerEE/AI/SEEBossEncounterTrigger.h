// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEBossEncounterTrigger.generated.h"

class ABossCharacter;
class AEnemyCharacter;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncounterStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEncounterCompleted);

UCLASS(Blueprintable)
class SNOWPIERCEREE_API ASEEBossEncounterTrigger : public AActor
{
	GENERATED_BODY()

public:
	ASEEBossEncounterTrigger();

	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void StartEncounter();

	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void EndEncounter();

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEncounterActive() const { return bEncounterActive; }

	UFUNCTION(BlueprintPure, Category = "Encounter")
	bool IsEncounterComplete() const { return bEncounterComplete; }

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterStarted OnEncounterStarted;

	UPROPERTY(BlueprintAssignable, Category = "Encounter")
	FOnEncounterCompleted OnEncounterCompleted;

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TSubclassOf<ABossCharacter> BossClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	FTransform BossSpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	TArray<FTransform> GuardSpawnTransforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	bool bAutoTriggerOnOverlap = true;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnTriggerOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnBossDefeated();

	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter")
	void LockEncounterDoors();

	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter")
	void UnlockEncounterDoors();

	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter")
	void StartEncounterMusic();

	UFUNCTION(BlueprintImplementableEvent, Category = "Encounter")
	void StopEncounterMusic();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerVolume;

	UPROPERTY()
	ABossCharacter* SpawnedBoss = nullptr;

	UPROPERTY()
	TArray<AEnemyCharacter*> SpawnedGuards;

	bool bEncounterActive = false;
	bool bEncounterComplete = false;
};
