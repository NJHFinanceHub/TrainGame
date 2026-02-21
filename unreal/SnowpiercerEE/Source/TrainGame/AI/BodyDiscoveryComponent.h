// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "BodyDiscoveryComponent.generated.h"

class AJackbootAIController;

// ============================================================================
// UBodyDiscoveryComponent
//
// Gives NPCs the ability to discover unconscious/dead bodies and react.
// Scans periodically for bodies within line of sight. When a body is found:
// - Jackboots: raise alert, call backup, investigate
// - Civilians: flee or raise alarm
// - Crowd NPCs: flee in terror
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBodyFound,
	AActor*, DiscoveredBody,
	EBodyState, BodyState,
	AActor*, Discoverer);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBodyDiscoveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBodyDiscoveryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Body Registration ---

	/** Register a body that can be discovered (called when NPC is incapacitated) */
	UFUNCTION(BlueprintCallable, Category = "Body Discovery")
	static void RegisterBody(AActor* Body, EBodyState State);

	/** Unregister a body (when hidden or removed) */
	UFUNCTION(BlueprintCallable, Category = "Body Discovery")
	static void UnregisterBody(AActor* Body);

	/** Update a body's state (e.g., moved from Unconscious to Hidden) */
	UFUNCTION(BlueprintCallable, Category = "Body Discovery")
	static void UpdateBodyState(AActor* Body, EBodyState NewState);

	/** Get all registered bodies */
	UFUNCTION(BlueprintPure, Category = "Body Discovery")
	static TArray<AActor*> GetAllRegisteredBodies();

	// --- Discovery ---

	/** Check if this NPC has already discovered a specific body */
	UFUNCTION(BlueprintPure, Category = "Body Discovery")
	bool HasDiscoveredBody(AActor* Body) const;

	/** Fired when this NPC discovers a body */
	UPROPERTY(BlueprintAssignable, Category = "Body Discovery")
	FOnBodyFound OnBodyFound;

protected:
	/** How far this NPC can spot a body */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Discovery")
	float DiscoveryRange = 1000.f;

	/** Vision cone half-angle for body detection (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Discovery")
	float DiscoveryVisionAngle = 60.f;

	/** How often to scan for bodies (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Discovery")
	float ScanInterval = 1.f;

	/** Whether hidden bodies can be discovered by this NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body Discovery")
	bool bCanFindHiddenBodies = false;

private:
	void ScanForBodies();
	bool CanSeeBody(AActor* Body) const;
	void ReactToBody(AActor* Body, EBodyState State);

	/** Bodies this NPC has already discovered (don't re-trigger) */
	UPROPERTY()
	TArray<AActor*> DiscoveredBodies;

	float ScanTimer = 0.f;

	/** Global registry of incapacitated bodies */
	struct FBodyEntry
	{
		TWeakObjectPtr<AActor> Body;
		EBodyState State;
	};
	static TArray<FBodyEntry> BodyRegistry;
};
