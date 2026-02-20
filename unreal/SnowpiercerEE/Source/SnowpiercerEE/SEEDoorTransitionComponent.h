#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEECarStreamingSubsystem.h"
#include "SEEDoorTransitionComponent.generated.h"

UENUM(BlueprintType)
enum class EDoorType : uint8
{
    Standard     UMETA(DisplayName = "Standard Interior (0.8s)"),
    Bulkhead     UMETA(DisplayName = "Bulkhead/Zone Boundary (2.0s)"),
    Security     UMETA(DisplayName = "Security Door (1.5s + interaction)"),
    Emergency    UMETA(DisplayName = "Emergency Hatch (1.2s)"),
    Exterior     UMETA(DisplayName = "Exterior Access (3.0s)")
};

UENUM(BlueprintType)
enum class EDoorTransitionState : uint8
{
    Closed,
    Opening,
    WaitingForLoad,
    Open,
    Closing
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDoorTransitionStateChanged, EDoorTransitionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDoorLoadMaskFailed);

UCLASS(ClassGroup=(Streaming), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEEDoorTransitionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USEEDoorTransitionComponent();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    EDoorType DoorType = EDoorType::Standard;

    /** Car index on the other side of this door */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door")
    int32 DestinationCarIndex = INDEX_NONE;

    /** Override animation duration (0 = use default for door type) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door", meta = (ClampMin = "0.0"))
    float AnimationDurationOverride = 0.0f;

    /** Maximum time to wait for load before fallback (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Fallback")
    float MaxLoadWaitTime = 1.0f;

    /** Whether to show fade-to-black as last resort */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Fallback")
    bool bAllowFadeToBlack = true;

    /** Fade-to-black duration if triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Fallback")
    float FadeToBlackDuration = 0.5f;

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Door")
    EDoorTransitionState CurrentState = EDoorTransitionState::Closed;

    // --- Interface ---

    /** Begin door opening transition. Call when player interacts with door. */
    UFUNCTION(BlueprintCallable, Category = "Door")
    void BeginTransition();

    /** Cancel an in-progress transition */
    UFUNCTION(BlueprintCallable, Category = "Door")
    void CancelTransition();

    /** Get the animation duration for this door type */
    UFUNCTION(BlueprintPure, Category = "Door")
    float GetAnimationDuration() const;

    /** Get normalized progress of the door animation (0-1) */
    UFUNCTION(BlueprintPure, Category = "Door")
    float GetTransitionProgress() const;

    /** Is the destination car loaded and ready? */
    UFUNCTION(BlueprintPure, Category = "Door")
    bool IsDestinationReady() const;

    // --- Events ---

    UPROPERTY(BlueprintAssignable, Category = "Door|Events")
    FOnDoorTransitionStateChanged OnTransitionStateChanged;

    /** Fired when load mask failed and fade-to-black was needed */
    UPROPERTY(BlueprintAssignable, Category = "Door|Events")
    FOnDoorLoadMaskFailed OnLoadMaskFailed;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void SetTransitionState(EDoorTransitionState NewState);
    void OnDestinationCarLoaded(int32 CarIndex);
    void TriggerCarEntry();
    void TriggerFadeToBlack();

    float TransitionTimer = 0.0f;
    float LoadWaitTimer = 0.0f;
    bool bDestinationLoaded = false;
    bool bFadeTriggered = false;

    /** Threshold at which we signal the streaming subsystem (75% through animation) */
    static constexpr float StreamingTriggerThreshold = 0.75f;
    bool bStreamingTriggered = false;
};
