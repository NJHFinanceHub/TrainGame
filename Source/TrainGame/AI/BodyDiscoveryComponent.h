// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "BodyDiscoveryComponent.generated.h"

// ============================================================================
// UBodyDiscoveryComponent
//
// Enables NPCs to discover unconscious/dead bodies and raise alerts.
// Periodically scans for downed NPCs within detection range, then triggers
// the alert chain: notify AI controller → propagate to nearby NPCs →
// escalate Jackboot alert level if applicable.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBodyFound, const FBodyDiscoveryReport&, Report);

UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UBodyDiscoveryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBodyDiscoveryComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Discovery ---

	/** Manually check for bodies now (called by behavior tree tasks) */
	UFUNCTION(BlueprintCallable, Category = "AI|BodyDiscovery")
	bool ScanForBodies();

	/** Report a body that was found */
	UFUNCTION(BlueprintCallable, Category = "AI|BodyDiscovery")
	void ReportBody(AActor* Body, EBodyState BodyState);

	/** Check if this NPC has discovered any bodies recently */
	UFUNCTION(BlueprintPure, Category = "AI|BodyDiscovery")
	bool HasDiscoveredBody() const { return DiscoveredBodies.Num() > 0; }

	/** Get all discovered body reports */
	UFUNCTION(BlueprintPure, Category = "AI|BodyDiscovery")
	const TArray<FBodyDiscoveryReport>& GetDiscoveredBodies() const { return DiscoveredBodies; }

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "AI|BodyDiscovery")
	FOnBodyFound OnBodyFound;

protected:
	virtual void BeginPlay() override;

	/** Range to scan for bodies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BodyDiscovery")
	float ScanRange = 800.f;

	/** How often to scan for bodies (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BodyDiscovery")
	float ScanInterval = 2.f;

	/** Whether this NPC requires line of sight to discover bodies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BodyDiscovery")
	bool bRequiresLineOfSight = true;

	/** Radius to alert nearby NPCs when a body is found */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|BodyDiscovery")
	float AlertPropagationRadius = 1500.f;

private:
	void PropagateBodyAlert(const FBodyDiscoveryReport& Report);
	bool IsBodyAlreadyKnown(AActor* Body) const;
	bool HasLineOfSightToBody(AActor* Body) const;

	TArray<FBodyDiscoveryReport> DiscoveredBodies;
	float ScanTimer = 0.f;

	/** Set of body actors already known to avoid re-reporting */
	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> KnownBodies;
};
