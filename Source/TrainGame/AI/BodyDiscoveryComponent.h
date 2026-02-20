// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "BodyDiscoveryComponent.generated.h"

class UDetectionComponent;

// ============================================================================
// UBodyDiscoveryComponent
//
// Attached to NPCs that can discover incapacitated bodies. Periodically
// scans for nearby unconscious/dead NPCs and raises alerts when found.
// Triggers the DetectionComponent's body discovery delegate and escalates
// the NPC's alert state.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyFound, const FBodyDiscoveryEvent&, Event);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBodyDiscoveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBodyDiscoveryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Force a scan for nearby bodies right now */
	UFUNCTION(BlueprintCallable, Category = "NPC|BodyDiscovery")
	void ScanForBodies();

	/** Check if a specific actor is a body that should be reported */
	UFUNCTION(BlueprintPure, Category = "NPC|BodyDiscovery")
	bool IsDiscoverableBody(AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "NPC|BodyDiscovery")
	bool HasDiscoveredBody() const { return bHasDiscoveredBody; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "NPC|BodyDiscovery")
	FOnBodyFound OnBodyFound;

protected:
	virtual void BeginPlay() override;

	/** Radius to scan for bodies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|BodyDiscovery")
	float ScanRadius = 800.f;

	/** How often to scan (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|BodyDiscovery")
	float ScanInterval = 2.0f;

	/** Whether this NPC requires line of sight to discover bodies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|BodyDiscovery")
	bool bRequiresLineOfSight = true;

private:
	void HandleBodyFound(AActor* Body, EBodyState State);

	UPROPERTY()
	UDetectionComponent* DetectionComp = nullptr;

	float ScanTimer = 0.f;
	bool bHasDiscoveredBody = false;

	/** Track bodies already discovered to avoid re-alerting */
	UPROPERTY()
	TSet<AActor*> DiscoveredBodies;
};
