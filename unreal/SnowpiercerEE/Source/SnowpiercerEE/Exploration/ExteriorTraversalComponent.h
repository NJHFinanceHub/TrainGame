// ExteriorTraversalComponent.h
// Snowpiercer: Eternal Engine - Rooftop & exterior traversal mechanics

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExteriorTraversalComponent.generated.h"

/** Current cold exposure stage */
UENUM(BlueprintType)
enum class EColdExposureStage : uint8
{
	None        UMETA(DisplayName = "None (Interior)"),
	Chill       UMETA(DisplayName = "Chill"),
	BitingCold  UMETA(DisplayName = "Biting Cold"),
	Hypothermia UMETA(DisplayName = "Hypothermia"),
	Frostbite   UMETA(DisplayName = "Frostbite"),
	Lethal      UMETA(DisplayName = "Lethal")
};

/** Wind intensity level affecting movement */
UENUM(BlueprintType)
enum class EWindIntensity : uint8
{
	None     UMETA(DisplayName = "None (Interior)"),
	Moderate UMETA(DisplayName = "Moderate (Uphill)"),
	Strong   UMETA(DisplayName = "Strong (Flat)"),
	Severe   UMETA(DisplayName = "Severe (Downhill)"),
	Extreme  UMETA(DisplayName = "Extreme (Tunnel Entry)")
};

/** Player posture on the rooftop */
UENUM(BlueprintType)
enum class ERooftopPosture : uint8
{
	Standing,
	Crouching,
	Prone
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColdStageChanged, EColdExposureStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTunnelWarning);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTunnelImpact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWindIntensityChanged, EWindIntensity, NewIntensity);

/**
 * Manages exterior traversal state: cold exposure, wind physics,
 * tunnel clearance, and rooftop movement modifications.
 * Attach to the player character.
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UExteriorTraversalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UExteriorTraversalComponent();

	// --- State ---

	/** Whether the player is currently on the train exterior */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior")
	bool bIsExterior = false;

	/** Whether the player is on the rooftop (vs under-car) */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior")
	bool bIsRooftop = false;

	/** Current cold exposure timer (seconds) */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior|Cold")
	float ColdExposureTime = 0.0f;

	/** Current cold stage */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior|Cold")
	EColdExposureStage ColdStage = EColdExposureStage::None;

	/** Current wind intensity (rooftop only) */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior|Wind")
	EWindIntensity WindIntensity = EWindIntensity::None;

	/** Current rooftop posture */
	UPROPERTY(BlueprintReadOnly, Category = "Exterior|Rooftop")
	ERooftopPosture Posture = ERooftopPosture::Standing;

	// --- Configuration ---

	/** Cold suit tier (0 = none, 1-3 = suit tiers) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exterior|Equipment", meta = (ClampMin = "0", ClampMax = "3"))
	int32 ColdSuitTier = 0;

	/** Base time (seconds) before advancing each cold stage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior|Cold")
	float BaseColdStageInterval = 60.0f;

	/** Cold timer reset rate when returning to interior (seconds recovered per second) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior|Cold")
	float InteriorRecoveryRate = 2.0f;

	/** Health drain per second at Hypothermia stage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior|Cold")
	float HypothermiaHealthDrain = 2.0f;

	/** Health drain per second at Frostbite stage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior|Cold")
	float FrostbiteHealthDrain = 5.0f;

	/** Health drain per second at Lethal stage */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Exterior|Cold")
	float LethalHealthDrain = 15.0f;

	// --- Interface ---

	/** Enter exterior mode (call when player exits the train) */
	UFUNCTION(BlueprintCallable, Category = "Exterior")
	void EnterExterior(bool bRooftop);

	/** Return to interior (call when player re-enters the train) */
	UFUNCTION(BlueprintCallable, Category = "Exterior")
	void ReturnToInterior();

	/** Set posture (standing/crouching/prone) */
	UFUNCTION(BlueprintCallable, Category = "Exterior|Rooftop")
	void SetPosture(ERooftopPosture NewPosture);

	/** Get current movement speed multiplier from wind + posture */
	UFUNCTION(BlueprintPure, Category = "Exterior|Movement")
	float GetMovementSpeedMultiplier() const;

	/** Get current stamina penalty from cold exposure */
	UFUNCTION(BlueprintPure, Category = "Exterior|Cold")
	float GetStaminaPenalty() const;

	/** Check if player is in a heated zone (near exhaust vents) */
	UFUNCTION(BlueprintCallable, Category = "Exterior|Cold")
	void SetInHeatedZone(bool bHeated);

	/** Get time bonus from cold suit tier */
	UFUNCTION(BlueprintPure, Category = "Exterior|Equipment")
	float GetColdSuitTimeBonus() const;

	/** Check if wind resistance is granted by current suit */
	UFUNCTION(BlueprintPure, Category = "Exterior|Equipment")
	bool HasWindResistance() const;

	/** Process stagger check from wind (returns true if staggered) */
	UFUNCTION(BlueprintCallable, Category = "Exterior|Wind")
	bool ProcessWindStaggerCheck(int32 AgilityScore);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Exterior|Events")
	FOnColdStageChanged OnColdStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Exterior|Events")
	FOnTunnelWarning OnTunnelWarning;

	UPROPERTY(BlueprintAssignable, Category = "Exterior|Events")
	FOnTunnelImpact OnTunnelImpact;

	UPROPERTY(BlueprintAssignable, Category = "Exterior|Events")
	FOnWindIntensityChanged OnWindIntensityChanged;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bInHeatedZone = false;
	float StaggerTimer = 0.0f;

	void UpdateColdExposure(float DeltaTime);
	void UpdateWindEffects(float DeltaTime);
	void ApplyColdEffects();
	EColdExposureStage CalculateColdStage() const;
	float GetStaggerInterval() const;
};
