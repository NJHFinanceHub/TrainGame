// WindowViewComponent.h
// Snowpiercer: Eternal Engine - Window/exterior viewing and thaw progression

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WindowViewComponent.generated.h"

/** Window types with different view properties */
UENUM(BlueprintType)
enum class EWindowType : uint8
{
	Passenger       UMETA(DisplayName = "Passenger (frost-covered)"),
	Porthole        UMETA(DisplayName = "Porthole (small, steamed)"),
	Observation     UMETA(DisplayName = "Observation (large, clear)"),
	Sealed          UMETA(DisplayName = "Sealed (shuttered)"),
	Broken          UMETA(DisplayName = "Broken (cold exposure)"),
	FirstClassPano  UMETA(DisplayName = "First Class Panoramic")
};

/** Thaw progression stage visible through windows */
UENUM(BlueprintType)
enum class EThawStage : uint8
{
	DeepWinter  UMETA(DisplayName = "Deep Winter (-67C)"),
	Winter      UMETA(DisplayName = "Winter (-58C)"),
	LateWinter  UMETA(DisplayName = "Late Winter (-45C)"),
	EarlyThaw   UMETA(DisplayName = "Early Thaw (-31C)"),
	Thaw        UMETA(DisplayName = "Thaw (-18C)"),
	Spring      UMETA(DisplayName = "Spring (-7C)")
};

/** Route segment affecting what's visible outside */
UENUM(BlueprintType)
enum class ERouteSegment : uint8
{
	Northern,    // Most frozen, least recovery
	Coastal,     // Moderate recovery
	Equatorial,  // Most recovery visible
	Mountain,    // Variable, tunnels frequent
	Geothermal   // Anomalous warmth
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThawStageChanged, EThawStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandmarkVisible, FName, LandmarkID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExteriorDiscovery, FName, DiscoveryID);

/**
 * Placed on window actors to manage exterior viewing, thaw progression
 * visibility, and landmark detection. Also handles frost-wiping interaction
 * and telescope mechanics.
 */
UCLASS(ClassGroup=(Exploration), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API UWindowViewComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWindowViewComponent();

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window")
	EWindowType WindowType = EWindowType::Passenger;

	/** Car number this window is in */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window")
	int32 CarNumber = 0;

	/** Whether this window has a telescope */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window|Telescope")
	bool bHasTelescope = false;

	/** Telescope zoom range (min-max multiplier) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window|Telescope", meta = (EditCondition = "bHasTelescope"))
	FVector2D TelescopeZoomRange = FVector2D(2.0f, 8.0f);

	/** Whether frost can be wiped from this window */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window")
	bool bCanWipeFrost = true;

	/** Duration of clear view after wiping frost (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window")
	float FrostClearDuration = 30.0f;

	/** Whether this window creates cold exposure when broken */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window")
	bool bCreatesColdZone = false;

	/** Landmark IDs visible from this window (checked against route) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Window|Landmarks")
	TArray<FName> VisibleLandmarks;

	// --- State ---

	UPROPERTY(BlueprintReadOnly, Category = "Window")
	bool bFrostWiped = false;

	UPROPERTY(BlueprintReadOnly, Category = "Window")
	float FrostTimer = 0.0f;

	// --- Interface ---

	/** Wipe frost from the window (player interaction) */
	UFUNCTION(BlueprintCallable, Category = "Window")
	void WipeFrost();

	/** Get the current thaw stage based on game progression */
	UFUNCTION(BlueprintPure, Category = "Window")
	EThawStage GetCurrentThawStage() const;

	/** Get the current route segment */
	UFUNCTION(BlueprintPure, Category = "Window")
	ERouteSegment GetCurrentRouteSegment() const;

	/** Check if a specific landmark is currently visible */
	UFUNCTION(BlueprintPure, Category = "Window|Landmarks")
	bool IsLandmarkVisible(FName LandmarkID) const;

	/** Begin telescope interaction */
	UFUNCTION(BlueprintCallable, Category = "Window|Telescope")
	void BeginTelescopeView();

	/** End telescope interaction */
	UFUNCTION(BlueprintCallable, Category = "Window|Telescope")
	void EndTelescopeView();

	/** Process PER check during telescope use (reveals details at thresholds) */
	UFUNCTION(BlueprintCallable, Category = "Window|Telescope")
	TArray<FText> ProcessTelescopePerceptionCheck(int32 PerceptionScore) const;

	/** Get exterior temperature for current route position */
	UFUNCTION(BlueprintPure, Category = "Window")
	float GetExteriorTemperature() const;

	/** Check if the view quality allows seeing the exterior */
	UFUNCTION(BlueprintPure, Category = "Window")
	bool CanSeeExterior() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Window|Events")
	FOnThawStageChanged OnThawStageChanged;

	UPROPERTY(BlueprintAssignable, Category = "Window|Events")
	FOnLandmarkVisible OnLandmarkVisible;

	UPROPERTY(BlueprintAssignable, Category = "Window|Events")
	FOnExteriorDiscovery OnExteriorDiscovery;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool bInTelescopeView = false;
	void CheckLandmarkVisibility();
	void UpdateFrostState(float DeltaTime);
};
