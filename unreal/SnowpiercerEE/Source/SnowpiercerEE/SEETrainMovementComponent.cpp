#include "SEETrainMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

USEETrainMovementComponent::USEETrainMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void USEETrainMovementComponent::BeginPlay()
{
    Super::BeginPlay();

    if (TrainLoopSound && GetOwner())
    {
        TrainLoopAudio = UGameplayStatics::SpawnSoundAttached(TrainLoopSound, GetOwner()->GetRootComponent());
    }
}

void USEETrainMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    ElapsedTime += DeltaTime;
    const float NewOffsetZ = FMath::Sin(ElapsedTime * SwayFrequencyHz * 2.0f * PI) * SwayAmplitudeCm;
    const float DeltaZ = NewOffsetZ - PreviousOffsetZ;
    PreviousOffsetZ = NewOffsetZ;

    Owner->AddActorWorldOffset(FVector(0.0f, 0.0f, DeltaZ), false);
}
