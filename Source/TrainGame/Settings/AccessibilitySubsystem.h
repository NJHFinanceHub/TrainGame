// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AccessibilityTypes.h"
#include "AccessibilitySubsystem.generated.h"

// ============================================================================
// USEEAccessibilitySubsystem
//
// Manages all accessibility settings. Settings persist to local config file
// (not in save data — they are per-player, not per-save).
// All changes take effect immediately.
// ============================================================================
UCLASS()
class TRAINGAME_API USEEAccessibilitySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	USEEAccessibilitySubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Full Settings ---

	/** Get current accessibility settings. */
	UFUNCTION(BlueprintPure, Category = "Accessibility")
	const FAccessibilitySettings& GetSettings() const { return CurrentSettings; }

	/** Apply a full settings struct (e.g., from a UI panel). */
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void ApplySettings(const FAccessibilitySettings& NewSettings);

	/** Reset all settings to defaults. */
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void ResetToDefaults();

	// --- Subtitle Settings ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
	void SetSubtitlesEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Accessibility|Subtitles")
	bool AreSubtitlesEnabled() const { return CurrentSettings.bSubtitlesEnabled; }

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
	void SetSpeakerIDMode(ESpeakerIDMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
	void SetSubtitleSize(ESubtitleSize Size);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
	void SetSubtitleBackground(ESubtitleBackground Background);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Subtitles")
	void SetClosedCaptions(bool bEnabled);

	/** Get the speaker color for a given speaker role. Respects colorblind mode. */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Subtitles")
	FLinearColor GetSpeakerColor(FName SpeakerRole) const;

	// --- Colorblind Settings ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colorblind")
	void SetColorblindMode(EColorblindMode Mode);

	UFUNCTION(BlueprintPure, Category = "Accessibility|Colorblind")
	EColorblindMode GetColorblindMode() const { return CurrentSettings.ColorblindMode; }

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Colorblind")
	void SetCustomColorblindConfig(const FCustomColorblindConfig& Config);

	/** Check if colorblind mode is active (any mode other than Off). */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Colorblind")
	bool IsColorblindModeActive() const { return CurrentSettings.ColorblindMode != EColorblindMode::Off; }

	// --- Visual Settings ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetHighContrastMode(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetScreenShakeIntensity(float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetMotionBlur(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetFieldOfView(float FOV);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetHUDScale(EHUDScale Scale);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Visual")
	void SetFlashingReduction(bool bEnabled);

	// --- Font & Text ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Text")
	void SetDyslexiaFriendlyFont(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Text")
	void SetGlobalTextSize(EGlobalTextSize Size);

	/** Get the actual text size multiplier from the enum. */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Text")
	float GetTextSizeMultiplier() const;

	/** Get the actual HUD scale multiplier from the enum. */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Visual")
	float GetHUDScaleMultiplier() const;

	// --- Controls ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Controls")
	void SetOneHandedPreset(EOneHandedPreset Preset);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Controls")
	void SetToggleHoldSettings(const FToggleHoldSettings& Settings);

	/** Check if a specific action should use toggle mode. */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Controls")
	bool IsActionToggle(FName ActionName) const;

	// --- Aim Assistance ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Aim")
	void SetAutoAimLevel(EAutoAimLevel Level);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Aim")
	void SetLockOnTargeting(bool bEnabled);

	/** Get aim magnetism angle in degrees based on auto-aim level. */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Aim")
	float GetAimMagnetismAngle() const;

	// --- Motor Accessibility ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Motor")
	void SetQTEMode(EQTEMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Motor")
	void SetSimultaneousInputMode(ESimultaneousInput Mode);

	/** Get the sequential input window in ms (only relevant when Sequential mode). */
	UFUNCTION(BlueprintPure, Category = "Accessibility|Motor")
	float GetSequentialInputWindow() const;

	// --- Gameplay Assists ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Gameplay")
	void SetNavigationAssist(ENavigationAssist Mode);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Gameplay")
	void SetSkipCombatEnabled(bool bEnabled);

	// --- Photosensitivity ---

	/** Enable photosensitivity mode (auto-enables related sub-options). */
	UFUNCTION(BlueprintCallable, Category = "Accessibility|Photosensitivity")
	void SetPhotosensitivityMode(bool bEnabled);

	// --- Audio Accessibility ---

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Audio")
	void SetMonoAudio(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Accessibility|Audio")
	void SetAudioVisualization(EAudioVisualization Mode);

	// --- Persistence ---

	/** Save settings to config file. */
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void SaveSettingsToConfig();

	/** Load settings from config file. */
	UFUNCTION(BlueprintCallable, Category = "Accessibility")
	void LoadSettingsFromConfig();

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Accessibility")
	FOnAccessibilitySettingsChanged OnSettingsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Accessibility")
	FOnColorblindModeChanged OnColorblindModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Accessibility")
	FOnSubtitleSettingsChanged OnSubtitleSettingsChanged;

private:
	void BroadcastSettingsChanged();
	void ApplyVisualSettings();
	void ApplyAudioSettings();
	void ApplyControlSettings();

	FAccessibilitySettings CurrentSettings;

	static const FString ConfigFileName;
};
