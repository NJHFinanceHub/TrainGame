#include "SEEDifficultySubsystem.h"

void USEEDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ApplyModifiersForDifficulty(CurrentDifficulty);
}

void USEEDifficultySubsystem::SetDifficulty(ESEEDifficulty NewDifficulty)
{
    if (CurrentDifficulty == NewDifficulty) return;

    CurrentDifficulty = NewDifficulty;
    ApplyModifiersForDifficulty(NewDifficulty);
    OnDifficultyChanged.Broadcast(NewDifficulty);
}

void USEEDifficultySubsystem::ApplyModifiersForDifficulty(ESEEDifficulty Difficulty)
{
    FSEEDifficultyModifiers Mods;

    switch (Difficulty)
    {
    case ESEEDifficulty::Passenger:
        Mods.EnemyHealthMult = 0.7f;
        Mods.EnemyDamageMult = 0.6f;
        Mods.DetectionRangeMult = 0.75f;
        Mods.DetectionSpeedMult = 0.7f;
        Mods.ResourceDropMult = 1.5f;
        Mods.ResourceDegradationMult = 0.5f;
        Mods.ColdExposureTimeMult = 1.5f;   // More time before frostbite
        Mods.StaminaDrainMult = 0.75f;
        Mods.XPGainMult = 1.25f;
        Mods.FallDamageMult = 0.5f;
        Mods.CompanionDownTimer = 60.0f;
        Mods.StatCheckBonus = 3;
        Mods.ResourceLossOnDeath = 0.0f;
        Mods.bPermadeath = false;
        Mods.bCompanionPermadeath = false;
        Mods.bShowQuestMarkers = true;
        Mods.bFriendlyFire = false;
        break;

    case ESEEDifficulty::Survivor:
        // All defaults (1.0x baseline)
        Mods.CompanionDownTimer = 30.0f;
        Mods.ResourceLossOnDeath = 0.1f;
        Mods.bCompanionPermadeath = true;
        break;

    case ESEEDifficulty::EternalEngine:
        Mods.EnemyHealthMult = 1.25f;
        Mods.EnemyDamageMult = 1.3f;
        Mods.DetectionRangeMult = 1.2f;
        Mods.DetectionSpeedMult = 1.25f;
        Mods.ResourceDropMult = 0.7f;
        Mods.ResourceDegradationMult = 1.25f;
        Mods.ColdExposureTimeMult = 0.75f;   // Less time before frostbite
        Mods.StaminaDrainMult = 1.2f;
        Mods.XPGainMult = 0.85f;
        Mods.FallDamageMult = 1.3f;
        Mods.CompanionDownTimer = 15.0f;
        Mods.StatCheckBonus = -2;
        Mods.ResourceLossOnDeath = 0.25f;
        Mods.bPermadeath = false;
        Mods.bCompanionPermadeath = true;
        Mods.bShowQuestMarkers = false;
        Mods.bFriendlyFire = true;
        break;

    case ESEEDifficulty::MrWilford:
        Mods.EnemyHealthMult = 1.5f;
        Mods.EnemyDamageMult = 1.6f;
        Mods.DetectionRangeMult = 1.35f;
        Mods.DetectionSpeedMult = 1.4f;
        Mods.ResourceDropMult = 0.5f;
        Mods.ResourceDegradationMult = 1.5f;
        Mods.ColdExposureTimeMult = 0.6f;
        Mods.StaminaDrainMult = 1.35f;
        Mods.XPGainMult = 0.75f;
        Mods.FallDamageMult = 1.6f;
        Mods.CompanionDownTimer = 8.0f;
        Mods.StatCheckBonus = -4;
        Mods.ResourceLossOnDeath = 1.0f;     // Lose everything (permadeath)
        Mods.bPermadeath = true;
        Mods.bCompanionPermadeath = true;
        Mods.bShowQuestMarkers = false;
        Mods.bFriendlyFire = true;
        break;
    }

    ActiveModifiers = Mods;
}
