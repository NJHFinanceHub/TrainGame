// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueTypes.h"
#include "NPCMemoryComponent.generated.h"

/**
 * UNPCMemoryComponent
 *
 * Manages NPC memory: personal events, faction reputation, witnessed actions,
 * rumor propagation, and temporary mood. Drives dialogue option availability
 * and NPC disposition.
 */
UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UNPCMemoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNPCMemoryComponent();

	// --- Memory Management ---

	/** Add a memory to this NPC */
	UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
	void AddMemory(const FNPCMemory& Memory);

	/** Check if this NPC has a specific memory tag */
	UFUNCTION(BlueprintPure, Category = "NPC|Memory")
	bool HasMemory(FName MemoryTag) const;

	/** Get all memories of a specific category */
	UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
	TArray<FNPCMemory> GetMemoriesByCategory(EMemoryCategory Category) const;

	/** Remove a specific memory by tag */
	UFUNCTION(BlueprintCallable, Category = "NPC|Memory")
	void RemoveMemory(FName MemoryTag);

	// --- Disposition ---

	/** Get current disposition toward the player */
	UFUNCTION(BlueprintPure, Category = "NPC|Disposition")
	int32 GetDisposition() const { return Disposition; }

	/** Get the disposition bracket */
	UFUNCTION(BlueprintPure, Category = "NPC|Disposition")
	ENPCDisposition GetDispositionBracket() const;

	/** Modify disposition by delta (clamped to -100..+100) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Disposition")
	void ModifyDisposition(int32 Delta);

	/** Set disposition to an absolute value */
	UFUNCTION(BlueprintCallable, Category = "NPC|Disposition")
	void SetDisposition(int32 NewValue);

	// --- Rumor System ---

	/** Receive a rumor from the propagation system */
	UFUNCTION(BlueprintCallable, Category = "NPC|Rumor")
	void ReceiveRumor(const FRumorData& Rumor);

	/** Get rumor fidelity based on distance traveled */
	UFUNCTION(BlueprintPure, Category = "NPC|Rumor")
	float GetRumorFidelity(int32 DistanceFromOrigin) const;

	// --- Lie Tracking ---

	/** Record a lie told to this NPC */
	UFUNCTION(BlueprintCallable, Category = "NPC|Lies")
	void RecordLie(FName LieTag, FName ContradictoryTruth);

	/** Check if a specific lie has been exposed */
	UFUNCTION(BlueprintPure, Category = "NPC|Lies")
	bool IsLieExposed(FName LieTag) const;

	/** Expose a lie (called when contradictory info reaches this NPC) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Lies")
	void ExposeLie(FName LieTag);

	// --- Delegates ---

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDispositionChanged, int32, OldValue, int32, NewValue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryAdded, FName, MemoryTag);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLieExposed, FName, LieTag);

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnDispositionChanged OnDispositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnMemoryAdded OnMemoryAdded;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnLieExposed OnLieExposed;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Decay mood memories over time */
	void TickMoodDecay(float DeltaTime);

private:
	/** All memories stored by this NPC */
	UPROPERTY()
	TArray<FNPCMemory> Memories;

	/** Current disposition toward the player (-100 to +100) */
	UPROPERTY(EditAnywhere, Category = "NPC")
	int32 Disposition = 0;

	/** Faction ID for factional memory sharing */
	UPROPERTY(EditAnywhere, Category = "NPC")
	FName FactionID = NAME_None;

	/** Car index this NPC is in (for rumor distance calculation) */
	UPROPERTY(EditAnywhere, Category = "NPC")
	int32 CarIndex = 0;

	/** Active lies: maps LieTag to the truth that would expose it */
	UPROPERTY()
	TMap<FName, FName> ActiveLies;

	/** Exposed lies */
	UPROPERTY()
	TSet<FName> ExposedLies;
};
