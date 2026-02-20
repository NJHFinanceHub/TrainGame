#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEETypes.h"
#include "SEEDifficultySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDifficultyChanged, ESEEDifficulty, NewDifficulty);

UCLASS()
class SNOWPIERCEREE_API USEEDifficultySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Difficulty")
    void SetDifficulty(ESEEDifficulty NewDifficulty);

    UFUNCTION(BlueprintPure, Category="Difficulty")
    ESEEDifficulty GetDifficulty() const { return CurrentDifficulty; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    FSEEDifficultyModifiers GetModifiers() const { return ActiveModifiers; }

    // Convenience accessors for commonly queried modifiers
    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetEnemyHealthMultiplier() const { return ActiveModifiers.EnemyHealthMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetEnemyDamageMultiplier() const { return ActiveModifiers.EnemyDamageMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetDetectionRangeMultiplier() const { return ActiveModifiers.DetectionRangeMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetDetectionSpeedMultiplier() const { return ActiveModifiers.DetectionSpeedMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetResourceDropMultiplier() const { return ActiveModifiers.ResourceDropMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetResourceDegradationMultiplier() const { return ActiveModifiers.ResourceDegradationMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetXPGainMultiplier() const { return ActiveModifiers.XPGainMult; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetCompanionDownTimer() const { return ActiveModifiers.CompanionDownTimer; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    int32 GetStatCheckBonus() const { return ActiveModifiers.StatCheckBonus; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    float GetResourceLossOnDeath() const { return ActiveModifiers.ResourceLossOnDeath; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    bool IsPermadeath() const { return ActiveModifiers.bPermadeath; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    bool HasCompanionPermadeath() const { return ActiveModifiers.bCompanionPermadeath; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    bool ShowQuestMarkers() const { return ActiveModifiers.bShowQuestMarkers; }

    UFUNCTION(BlueprintPure, Category="Difficulty")
    bool HasFriendlyFire() const { return ActiveModifiers.bFriendlyFire; }

    UPROPERTY(BlueprintAssignable, Category="Difficulty")
    FOnDifficultyChanged OnDifficultyChanged;

private:
    void ApplyModifiersForDifficulty(ESEEDifficulty Difficulty);

    ESEEDifficulty CurrentDifficulty = ESEEDifficulty::Survivor;
    FSEEDifficultyModifiers ActiveModifiers;
};
