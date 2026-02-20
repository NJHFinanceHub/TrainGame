#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SEETrainMovementComponent.generated.h"

class UAudioComponent;
class USoundBase;

UCLASS(ClassGroup=(Snowpiercer), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEETrainMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USEETrainMovementComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Train Motion")
    float SwayAmplitudeCm = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Train Motion")
    float SwayFrequencyHz = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Train Audio")
    USoundBase* TrainLoopSound = nullptr;

private:
    UPROPERTY(Transient)
    UAudioComponent* TrainLoopAudio = nullptr;

    float ElapsedTime = 0.0f;
    float PreviousOffsetZ = 0.0f;
};
