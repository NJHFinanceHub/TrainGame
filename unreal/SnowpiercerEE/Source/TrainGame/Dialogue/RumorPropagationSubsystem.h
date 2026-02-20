// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DialogueTypes.h"
#include "RumorPropagationSubsystem.generated.h"

class UNPCMemoryComponent;

/**
 * URumorPropagationSubsystem
 *
 * World subsystem that manages rumor spread across the train. Tracks active
 * rumors, advances their reach over game time, and delivers them to NPCs
 * in newly-reached cars.
 */
UCLASS()
class TRAINGAME_API URumorPropagationSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Create a new rumor at the specified car */
	UFUNCTION(BlueprintCallable, Category = "Rumor")
	void CreateRumor(FName RumorTag, int32 OriginCar);

	/** Create a rumor with custom propagation speed */
	UFUNCTION(BlueprintCallable, Category = "Rumor")
	void CreateRumorWithSpeed(FName RumorTag, int32 OriginCar, float PropagationRate);

	/** Get all active rumors */
	UFUNCTION(BlueprintCallable, Category = "Rumor")
	TArray<FRumorData> GetActiveRumors() const { return ActiveRumors; }

	/** Check if a rumor has reached a specific car */
	UFUNCTION(BlueprintPure, Category = "Rumor")
	bool HasRumorReachedCar(FName RumorTag, int32 CarIndex) const;

	/** Advance rumor propagation (called from game time tick) */
	UFUNCTION(BlueprintCallable, Category = "Rumor")
	void TickRumorPropagation(float GameMinutesElapsed);

protected:
	/** Deliver a rumor to all NPCs in a specific car */
	void DeliverRumorToCar(const FRumorData& Rumor, int32 CarIndex);

	/** Find all NPC memory components in a car */
	TArray<UNPCMemoryComponent*> GetNPCsInCar(int32 CarIndex) const;

private:
	/** All active rumors in the train */
	UPROPERTY()
	TArray<FRumorData> ActiveRumors;

	/** Track which cars have already received each rumor */
	TMap<FName, TSet<int32>> DeliveredCars;
};
