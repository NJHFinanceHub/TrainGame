#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SEETypes.h"
#include "SEEAccessibilitySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAccessibilitySettingsChanged);

UCLASS()
class SNOWPIERCEREE_API USEEAccessibilitySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Subtitles ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Subtitles")
    void SetSubtitlesEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Subtitles")
    bool AreSubtitlesEnabled() const { return bSubtitlesEnabled; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Subtitles")
    void SetSpeakerIDMode(ESEESpeakerIDMode Mode);

    UFUNCTION(BlueprintPure, Category="Accessibility|Subtitles")
    ESEESpeakerIDMode GetSpeakerIDMode() const { return SpeakerIDMode; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Subtitles")
    void SetSubtitleSize(ESEESubtitleSize Size);

    UFUNCTION(BlueprintPure, Category="Accessibility|Subtitles")
    ESEESubtitleSize GetSubtitleSize() const { return SubtitleSize; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Subtitles")
    void SetSubtitleBackground(float Opacity);

    UFUNCTION(BlueprintPure, Category="Accessibility|Subtitles")
    float GetSubtitleBackground() const { return SubtitleBackgroundOpacity; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Subtitles")
    void SetClosedCaptionsEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Subtitles")
    bool AreClosedCaptionsEnabled() const { return bClosedCaptionsEnabled; }

    // ── Colorblind ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Colorblind")
    void SetColorblindMode(ESEEColorblindMode Mode);

    UFUNCTION(BlueprintPure, Category="Accessibility|Colorblind")
    ESEEColorblindMode GetColorblindMode() const { return ColorblindMode; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Colorblind")
    void SetColorblindStrength(float Strength);

    UFUNCTION(BlueprintPure, Category="Accessibility|Colorblind")
    float GetColorblindStrength() const { return ColorblindStrength; }

    // ── Visual ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetHighContrastMode(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    bool IsHighContrastEnabled() const { return bHighContrastMode; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetHUDScale(float Scale);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    float GetHUDScale() const { return HUDScale; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetScreenShakeIntensity(float Intensity);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    float GetScreenShakeIntensity() const { return ScreenShakeIntensity; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetMotionBlurEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    bool IsMotionBlurEnabled() const { return bMotionBlurEnabled; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetCameraBobEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    bool IsCameraBobEnabled() const { return bCameraBobEnabled; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Visual")
    void SetPhotosensitivityMode(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Visual")
    bool IsPhotosensitivityModeEnabled() const { return bPhotosensitivityMode; }

    // ── Font ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Font")
    void SetDyslexiaFontEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Font")
    bool IsDyslexiaFontEnabled() const { return bDyslexiaFont; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Font")
    void SetGlobalTextScale(float Scale);

    UFUNCTION(BlueprintPure, Category="Accessibility|Font")
    float GetGlobalTextScale() const { return GlobalTextScale; }

    // ── Controls ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Controls")
    void SetActionToggleMode(FName ActionName, bool bToggle);

    UFUNCTION(BlueprintPure, Category="Accessibility|Controls")
    bool IsActionToggle(FName ActionName) const;

    UFUNCTION(BlueprintCallable, Category="Accessibility|Controls")
    void RemapAction(FName ActionName, FKey NewKey);

    UFUNCTION(BlueprintPure, Category="Accessibility|Controls")
    FKey GetRemappedKey(FName ActionName) const;

    // ── Audio ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Audio")
    void SetMonoAudio(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Audio")
    bool IsMonoAudioEnabled() const { return bMonoAudio; }

    // ── Motor ──

    UFUNCTION(BlueprintCallable, Category="Accessibility|Motor")
    void SetAutoCompleteQTE(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="Accessibility|Motor")
    bool IsAutoCompleteQTEEnabled() const { return bAutoCompleteQTE; }

    UFUNCTION(BlueprintCallable, Category="Accessibility|Motor")
    void SetQTETimingMultiplier(float Multiplier);

    UFUNCTION(BlueprintPure, Category="Accessibility|Motor")
    float GetQTETimingMultiplier() const { return QTETimingMultiplier; }

    // ── Persistence ──

    UFUNCTION(BlueprintCallable, Category="Accessibility")
    void SaveSettings();

    UFUNCTION(BlueprintCallable, Category="Accessibility")
    void LoadSettings();

    UFUNCTION(BlueprintCallable, Category="Accessibility")
    void ResetToDefaults();

    // ── Events ──

    UPROPERTY(BlueprintAssignable, Category="Accessibility")
    FOnAccessibilitySettingsChanged OnSettingsChanged;

private:
    void BroadcastChange();

    // Subtitles
    bool bSubtitlesEnabled = true;
    ESEESpeakerIDMode SpeakerIDMode = ESEESpeakerIDMode::NameAndColor;
    ESEESubtitleSize SubtitleSize = ESEESubtitleSize::Medium;
    float SubtitleBackgroundOpacity = 0.5f;
    bool bClosedCaptionsEnabled = false;

    // Colorblind
    ESEEColorblindMode ColorblindMode = ESEEColorblindMode::None;
    float ColorblindStrength = 1.0f;

    // Visual
    bool bHighContrastMode = false;
    float HUDScale = 1.0f;
    float ScreenShakeIntensity = 1.0f;
    bool bMotionBlurEnabled = true;
    bool bCameraBobEnabled = true;
    bool bPhotosensitivityMode = false;

    // Font
    bool bDyslexiaFont = false;
    float GlobalTextScale = 1.0f;

    // Controls
    TMap<FName, bool> ActionToggleModes;
    TMap<FName, FKey> RemappedKeys;

    // Audio
    bool bMonoAudio = false;

    // Motor
    bool bAutoCompleteQTE = false;
    float QTETimingMultiplier = 1.0f;

    static const FString SettingsSaveSlot;
};
