#include "SEEDoorTransitionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

USEEDoorTransitionComponent::USEEDoorTransitionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void USEEDoorTransitionComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for car load completions
    UWorld* World = GetWorld();
    if (World)
    {
        USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
        if (StreamingSub)
        {
            StreamingSub->OnCarLoadComplete.AddDynamic(this, &USEEDoorTransitionComponent::OnDestinationCarLoaded);
        }
    }
}

void USEEDoorTransitionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    switch (CurrentState)
    {
    case EDoorTransitionState::Opening:
    {
        TransitionTimer += DeltaTime;
        float Duration = GetAnimationDuration();
        float Progress = (Duration > 0.0f) ? TransitionTimer / Duration : 1.0f;

        // At 75% through animation, trigger streaming
        if (!bStreamingTriggered && Progress >= StreamingTriggerThreshold)
        {
            bStreamingTriggered = true;
            UWorld* World = GetWorld();
            if (World)
            {
                USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
                if (StreamingSub)
                {
                    StreamingSub->SetDoorLoadMaskActive(true);
                    StreamingSub->EnterCar(DestinationCarIndex);
                }
            }
        }

        // Animation complete
        if (Progress >= 1.0f)
        {
            if (bDestinationLoaded || IsDestinationReady())
            {
                TriggerCarEntry();
            }
            else
            {
                SetTransitionState(EDoorTransitionState::WaitingForLoad);
            }
        }
        break;
    }

    case EDoorTransitionState::WaitingForLoad:
    {
        LoadWaitTimer += DeltaTime;

        // Check if destination loaded while we waited
        if (bDestinationLoaded || IsDestinationReady())
        {
            TriggerCarEntry();
            break;
        }

        // "Stuck door" animation: hold at 95% for up to MaxLoadWaitTime
        if (LoadWaitTimer >= MaxLoadWaitTime)
        {
            if (bAllowFadeToBlack && !bFadeTriggered)
            {
                TriggerFadeToBlack();
            }
        }
        break;
    }

    case EDoorTransitionState::Open:
    {
        // Door stays open briefly then closes
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= 2.0f)
        {
            SetTransitionState(EDoorTransitionState::Closing);
            TransitionTimer = 0.0f;
        }
        break;
    }

    case EDoorTransitionState::Closing:
    {
        TransitionTimer += DeltaTime;
        if (TransitionTimer >= GetAnimationDuration() * 0.5f)
        {
            SetTransitionState(EDoorTransitionState::Closed);
            SetComponentTickEnabled(false);
        }
        break;
    }

    default:
        break;
    }
}

float USEEDoorTransitionComponent::GetAnimationDuration() const
{
    if (AnimationDurationOverride > 0.0f)
    {
        return AnimationDurationOverride;
    }

    switch (DoorType)
    {
    case EDoorType::Standard:  return 0.8f;
    case EDoorType::Bulkhead:  return 2.0f;
    case EDoorType::Security:  return 1.5f;
    case EDoorType::Emergency: return 1.2f;
    case EDoorType::Exterior:  return 3.0f;
    default:                   return 1.0f;
    }
}

float USEEDoorTransitionComponent::GetTransitionProgress() const
{
    if (CurrentState == EDoorTransitionState::Closed)
    {
        return 0.0f;
    }
    if (CurrentState == EDoorTransitionState::Open)
    {
        return 1.0f;
    }

    float Duration = GetAnimationDuration();
    return (Duration > 0.0f) ? FMath::Clamp(TransitionTimer / Duration, 0.0f, 1.0f) : 1.0f;
}

bool USEEDoorTransitionComponent::IsDestinationReady() const
{
    if (DestinationCarIndex == INDEX_NONE) return false;

    UWorld* World = GetWorld();
    if (!World) return false;

    USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
    if (!StreamingSub) return false;

    ECarStreamingState State = StreamingSub->GetCarState(DestinationCarIndex);
    return State == ECarStreamingState::Loaded || State == ECarStreamingState::Active;
}

void USEEDoorTransitionComponent::BeginTransition()
{
    if (CurrentState != EDoorTransitionState::Closed)
    {
        return;
    }

    if (DestinationCarIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Warning, TEXT("DoorTransition: No destination car set."));
        return;
    }

    TransitionTimer = 0.0f;
    LoadWaitTimer = 0.0f;
    bDestinationLoaded = false;
    bFadeTriggered = false;
    bStreamingTriggered = false;

    // Check if destination is already loaded
    bDestinationLoaded = IsDestinationReady();

    SetTransitionState(EDoorTransitionState::Opening);
    SetComponentTickEnabled(true);
}

void USEEDoorTransitionComponent::CancelTransition()
{
    if (CurrentState == EDoorTransitionState::Closed)
    {
        return;
    }

    // Release load mask
    UWorld* World = GetWorld();
    if (World)
    {
        USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
        if (StreamingSub)
        {
            StreamingSub->SetDoorLoadMaskActive(false);
        }
    }

    SetTransitionState(EDoorTransitionState::Closed);
    SetComponentTickEnabled(false);
}

void USEEDoorTransitionComponent::SetTransitionState(EDoorTransitionState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    OnTransitionStateChanged.Broadcast(NewState);
}

void USEEDoorTransitionComponent::OnDestinationCarLoaded(int32 CarIndex)
{
    if (CarIndex == DestinationCarIndex)
    {
        bDestinationLoaded = true;

        // If we're waiting for load, proceed
        if (CurrentState == EDoorTransitionState::WaitingForLoad)
        {
            TriggerCarEntry();
        }
    }
}

void USEEDoorTransitionComponent::TriggerCarEntry()
{
    // Release the load mask
    UWorld* World = GetWorld();
    if (World)
    {
        USEECarStreamingSubsystem* StreamingSub = World->GetSubsystem<USEECarStreamingSubsystem>();
        if (StreamingSub)
        {
            StreamingSub->SetDoorLoadMaskActive(false);
        }
    }

    SetTransitionState(EDoorTransitionState::Open);
    TransitionTimer = 0.0f;
}

void USEEDoorTransitionComponent::TriggerFadeToBlack()
{
    bFadeTriggered = true;
    OnLoadMaskFailed.Broadcast();

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APlayerCameraManager* CamMgr = PC->PlayerCameraManager;
    if (CamMgr)
    {
        CamMgr->StartCameraFade(0.0f, 1.0f, FadeToBlackDuration, FLinearColor::Black, false, true);
    }

    // Schedule fade-in after a brief pause to allow loading
    FTimerHandle FadeInTimer;
    World->GetTimerManager().SetTimer(FadeInTimer, [this]()
    {
        if (bDestinationLoaded || IsDestinationReady())
        {
            TriggerCarEntry();

            UWorld* W = GetWorld();
            if (W)
            {
                APlayerController* Controller = W->GetFirstPlayerController();
                if (Controller && Controller->PlayerCameraManager)
                {
                    Controller->PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeToBlackDuration,
                                                                      FLinearColor::Black, false, false);
                }
            }
        }
    }, FadeToBlackDuration + 0.5f, false);
}
