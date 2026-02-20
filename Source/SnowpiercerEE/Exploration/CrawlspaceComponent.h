// CrawlspaceComponent.h
// Snowpiercer: Eternal Engine - Under-car crawlspace traversal mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrawlspaceComponent.generated.h"

/** Crawlspace movement mode */
UENUM(BlueprintType)
enum class ECrawlSpeed : uint8
{
	Slow    UMETA(DisplayName = "Slow Crawl (0.5 m/s)"),
	Normal  UMETA(DisplayName = "Normal Crawl (1.0 m/s)"),
	Fast    UMETA(DisplayName = "Fast Crawl (1.8 m/s)"),
	Squeeze UMETA(DisplayName = "Squeeze (0.3 m/s)")
};

/** Hazard types in the crawlspace */
UENUM(BlueprintType)
enum class ECrawlspaceHazard : uint8
{
	None,
	DriveShaft,     // Moving parts, timing-based, instant death
	SteamVent,      // Periodic burst, 30% health damage
	Electrical,     // Static placement, 20% health + stun
	LooseGrating,   // Fall risk, AGI check
	IceFormation    // Slippery, slide toward edge
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrawlspaceEntered, int32, CarNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCrawlspaceExited);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEavesdrop, int32, CarNumber, FName, ConversationID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHazardTriggered, ECrawlspaceHazard, HazardType);

/**
 * Manages under-car crawlspace traversal: movement, noise generation,
 * hazard detection, eavesdropping, and junction box hacking.
 * Attach to the player character alongside ExteriorTraversalComponent.
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class UCrawlspaceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCrawlspaceComponent();

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Crawlspace")
	bool bInCrawlspace = false;

	UPROPERTY(BlueprintReadOnly, Category = "Crawlspace")
	ECrawlSpeed CurrentSpeed = ECrawlSpeed::Normal;

	/** Car number currently beneath */
	UPROPERTY(BlueprintReadOnly, Category = "Crawlspace")
	int32 CurrentCarNumber = 0;

	// --- Configuration ---

	/** Base noise radius for normal crawl (meters, upward through floor) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crawlspace|Noise")
	float BaseNoiseRadius = 4.0f;

	/** Train motion noise masking (subtracted from noise radius) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crawlspace|Noise")
	float TrainMotionMasking = 3.0f;

	/** Stamina cost per second for normal crawl */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crawlspace|Movement")
	float BaseStaminaCost = 5.0f;

	// --- Interface ---

	/** Enter crawlspace from a floor hatch or between-car drop */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	void EnterCrawlspace(int32 CarNumber);

	/** Exit crawlspace to interior */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	void ExitCrawlspace();

	/** Set crawl speed mode */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	void SetCrawlSpeed(ECrawlSpeed Speed);

	/** Get noise radius for current speed (accounting for masking) */
	UFUNCTION(BlueprintPure, Category = "Crawlspace|Noise")
	float GetCurrentNoiseRadius() const;

	/** Get stamina cost per second for current speed */
	UFUNCTION(BlueprintPure, Category = "Crawlspace|Movement")
	float GetCurrentStaminaCost() const;

	/** Get movement speed in m/s for current mode */
	UFUNCTION(BlueprintPure, Category = "Crawlspace|Movement")
	float GetCurrentMoveSpeed() const;

	/** Check if player is near an observation grate (can look up into car) */
	UFUNCTION(BlueprintPure, Category = "Crawlspace")
	bool IsNearObservationGrate() const;

	/** Begin eavesdropping through a grate */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	void BeginEavesdrop();

	/** Attempt to hack a junction box (returns true if CUN check passed) */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	bool TryHackJunctionBox(int32 CunningScore);

	/** Check for hazard at current position */
	UFUNCTION(BlueprintPure, Category = "Crawlspace")
	ECrawlspaceHazard GetCurrentHazard() const;

	/** Process hazard interaction (returns damage dealt, 0 if avoided) */
	UFUNCTION(BlueprintCallable, Category = "Crawlspace")
	float ProcessHazard(ECrawlspaceHazard Hazard, int32 AgilityScore, int32 PerceptionScore);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Crawlspace|Events")
	FOnCrawlspaceEntered OnCrawlspaceEntered;

	UPROPERTY(BlueprintAssignable, Category = "Crawlspace|Events")
	FOnCrawlspaceExited OnCrawlspaceExited;

	UPROPERTY(BlueprintAssignable, Category = "Crawlspace|Events")
	FOnEavesdrop OnEavesdrop;

	UPROPERTY(BlueprintAssignable, Category = "Crawlspace|Events")
	FOnHazardTriggered OnHazardTriggered;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void GenerateNoise(float Radius);
	void UpdateCarTracking();
};
