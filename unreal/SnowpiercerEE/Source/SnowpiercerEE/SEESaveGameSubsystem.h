#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "SEETypes.h"
#include "SEESaveGameSubsystem.generated.h"

UCLASS()
class SNOWPIERCEREE_API USEESaveGameData : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(SaveGame)
    TMap<int32, FSEECarState> CarStates;
};

UCLASS()
class SNOWPIERCEREE_API USEESaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Save")
    void SetCarState(int32 CarIndex, const FSEECarState& State);

    UFUNCTION(BlueprintCallable, Category="Save")
    bool GetCarState(int32 CarIndex, FSEECarState& OutState) const;

    UFUNCTION(BlueprintCallable, Category="Save")
    bool WriteToSlot();

    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadFromSlot();

private:
    UPROPERTY()
    TMap<int32, FSEECarState> RuntimeCarStates;

    UPROPERTY()
    FString SaveSlotName = TEXT("SnowpiercerEE_Main");

    UPROPERTY()
    int32 UserIndex = 0;
};
