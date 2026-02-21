// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "AccessibilitySubsystem.h"
#include "Misc/ConfigCacheIni.h"
#include "GameFramework/GameUserSettings.h"

const FString USEEAccessibilitySubsystem::ConfigFileName = TEXT("Accessibility");

USEEAccessibilitySubsystem::USEEAccessibilitySubsystem()
{
}

void USEEAccessibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadSettingsFromConfig();
}

void USEEAccessibilitySubsystem::Deinitialize()
{
	SaveSettingsToConfig();
	Super::Deinitialize();
}

// ============================================================================
// Full Settings
// ============================================================================

void USEEAccessibilitySubsystem::ApplySettings(const FAccessibilitySettings& NewSettings)
{
	CurrentSettings = NewSettings;
	ApplyVisualSettings();
	ApplyAudioSettings();
	ApplyControlSettings();
	BroadcastSettingsChanged();
	SaveSettingsToConfig();
}

void USEEAccessibilitySubsystem::ResetToDefaults()
{
	CurrentSettings = FAccessibilitySettings();
	ApplyVisualSettings();
	ApplyAudioSettings();
	ApplyControlSettings();
	BroadcastSettingsChanged();
	SaveSettingsToConfig();
}

// ============================================================================
// Subtitle Settings
// ============================================================================

void USEEAccessibilitySubsystem::SetSubtitlesEnabled(bool bEnabled)
{
	CurrentSettings.bSubtitlesEnabled = bEnabled;
	OnSubtitleSettingsChanged.Broadcast(bEnabled);
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetSpeakerIDMode(ESpeakerIDMode Mode)
{
	CurrentSettings.SpeakerIDMode = Mode;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetSubtitleSize(ESubtitleSize Size)
{
	CurrentSettings.SubtitleSize = Size;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetSubtitleBackground(ESubtitleBackground Background)
{
	CurrentSettings.SubtitleBackground = Background;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetClosedCaptions(bool bEnabled)
{
	CurrentSettings.bClosedCaptions = bEnabled;
	BroadcastSettingsChanged();
}

FLinearColor USEEAccessibilitySubsystem::GetSpeakerColor(FName SpeakerRole) const
{
	if (CurrentSettings.SpeakerIDMode == ESpeakerIDMode::Off)
	{
		return FLinearColor::White;
	}

	// When colorblind mode is active, fall back to white (UI uses shape/icon differentiation)
	if (IsColorblindModeActive() && CurrentSettings.SpeakerIDMode == ESpeakerIDMode::NameColor)
	{
		return FLinearColor::White;
	}

	const FSpeakerColorConfig& Colors = CurrentSettings.SpeakerColors;

	if (SpeakerRole == FName("Player")) return Colors.PlayerColor;
	if (SpeakerRole == FName("Companion")) return Colors.CompanionColor;
	if (SpeakerRole == FName("Hostile")) return Colors.HostileColor;
	if (SpeakerRole == FName("Neutral")) return Colors.NeutralColor;
	if (SpeakerRole == FName("Intercom")) return Colors.IntercomColor;
	if (SpeakerRole == FName("Wilford")) return Colors.WilfordColor;

	return FLinearColor::White;
}

// ============================================================================
// Colorblind Settings
// ============================================================================

void USEEAccessibilitySubsystem::SetColorblindMode(EColorblindMode Mode)
{
	const EColorblindMode OldMode = CurrentSettings.ColorblindMode;
	CurrentSettings.ColorblindMode = Mode;

	if (OldMode != Mode)
	{
		OnColorblindModeChanged.Broadcast(Mode);
		ApplyVisualSettings();
		BroadcastSettingsChanged();
	}
}

void USEEAccessibilitySubsystem::SetCustomColorblindConfig(const FCustomColorblindConfig& Config)
{
	CurrentSettings.CustomColorblind = Config;
	if (CurrentSettings.ColorblindMode == EColorblindMode::Custom)
	{
		ApplyVisualSettings();
		BroadcastSettingsChanged();
	}
}

// ============================================================================
// Visual Settings
// ============================================================================

void USEEAccessibilitySubsystem::SetHighContrastMode(bool bEnabled)
{
	CurrentSettings.bHighContrastMode = bEnabled;
	ApplyVisualSettings();
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetScreenShakeIntensity(float Intensity)
{
	CurrentSettings.ScreenShakeIntensity = FMath::Clamp(Intensity, 0.f, 100.f);
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetMotionBlur(bool bEnabled)
{
	CurrentSettings.bMotionBlur = bEnabled;
	ApplyVisualSettings();
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetFieldOfView(float FOV)
{
	CurrentSettings.FieldOfView = FMath::Clamp(FOV, 60.f, 120.f);
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetHUDScale(EHUDScale Scale)
{
	CurrentSettings.HUDScale = Scale;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetFlashingReduction(bool bEnabled)
{
	CurrentSettings.bFlashingReduction = bEnabled;
	ApplyVisualSettings();
	BroadcastSettingsChanged();
}

// ============================================================================
// Font & Text
// ============================================================================

void USEEAccessibilitySubsystem::SetDyslexiaFriendlyFont(bool bEnabled)
{
	CurrentSettings.bDyslexiaFriendlyFont = bEnabled;

	// Auto-increase letter spacing when dyslexia font enabled
	if (bEnabled && CurrentSettings.LetterSpacing == ELetterSpacing::Normal)
	{
		CurrentSettings.LetterSpacing = ELetterSpacing::Wide;
	}

	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetGlobalTextSize(EGlobalTextSize Size)
{
	CurrentSettings.GlobalTextSize = Size;
	BroadcastSettingsChanged();
}

float USEEAccessibilitySubsystem::GetTextSizeMultiplier() const
{
	switch (CurrentSettings.GlobalTextSize)
	{
	case EGlobalTextSize::Size80: return 0.8f;
	case EGlobalTextSize::Size100: return 1.0f;
	case EGlobalTextSize::Size120: return 1.2f;
	case EGlobalTextSize::Size140: return 1.4f;
	default: return 1.0f;
	}
}

float USEEAccessibilitySubsystem::GetHUDScaleMultiplier() const
{
	switch (CurrentSettings.HUDScale)
	{
	case EHUDScale::Scale75: return 0.75f;
	case EHUDScale::Scale100: return 1.0f;
	case EHUDScale::Scale125: return 1.25f;
	case EHUDScale::Scale150: return 1.5f;
	default: return 1.0f;
	}
}

// ============================================================================
// Controls
// ============================================================================

void USEEAccessibilitySubsystem::SetOneHandedPreset(EOneHandedPreset Preset)
{
	CurrentSettings.OneHandedPreset = Preset;
	ApplyControlSettings();
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetToggleHoldSettings(const FToggleHoldSettings& Settings)
{
	CurrentSettings.ToggleHoldSettings = Settings;
	BroadcastSettingsChanged();
}

bool USEEAccessibilitySubsystem::IsActionToggle(FName ActionName) const
{
	const FToggleHoldSettings& TH = CurrentSettings.ToggleHoldSettings;

	if (ActionName == FName("Sprint")) return TH.bSprintToggle;
	if (ActionName == FName("Crouch")) return TH.bCrouchToggle;
	if (ActionName == FName("Aim")) return TH.bAimToggle;
	if (ActionName == FName("Block")) return TH.bBlockToggle;
	if (ActionName == FName("Interact")) return TH.bInteractToggle;
	if (ActionName == FName("Grab")) return TH.bGrabToggle;
	if (ActionName == FName("BreathHold")) return TH.bBreathHoldToggle;

	return false;
}

// ============================================================================
// Aim Assistance
// ============================================================================

void USEEAccessibilitySubsystem::SetAutoAimLevel(EAutoAimLevel Level)
{
	CurrentSettings.AutoAimLevel = Level;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetLockOnTargeting(bool bEnabled)
{
	CurrentSettings.bLockOnTargeting = bEnabled;
	BroadcastSettingsChanged();
}

float USEEAccessibilitySubsystem::GetAimMagnetismAngle() const
{
	switch (CurrentSettings.AutoAimLevel)
	{
	case EAutoAimLevel::Low: return 5.f;
	case EAutoAimLevel::Medium: return 10.f;
	case EAutoAimLevel::High: return 15.f;
	default: return 0.f;
	}
}

// ============================================================================
// Motor Accessibility
// ============================================================================

void USEEAccessibilitySubsystem::SetQTEMode(EQTEMode Mode)
{
	CurrentSettings.QTEMode = Mode;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetSimultaneousInputMode(ESimultaneousInput Mode)
{
	CurrentSettings.SimultaneousInputMode = Mode;
	BroadcastSettingsChanged();
}

float USEEAccessibilitySubsystem::GetSequentialInputWindow() const
{
	if (CurrentSettings.SimultaneousInputMode == ESimultaneousInput::Sequential)
	{
		// Base 500ms, scaled by input tolerance
		switch (CurrentSettings.InputTolerance)
		{
		case EInputTolerance::Tight: return 200.f;
		case EInputTolerance::Normal: return 500.f;
		case EInputTolerance::Generous: return 1000.f;
		default: return 500.f;
		}
	}
	return 0.f;
}

// ============================================================================
// Gameplay Assists
// ============================================================================

void USEEAccessibilitySubsystem::SetNavigationAssist(ENavigationAssist Mode)
{
	CurrentSettings.NavigationAssist = Mode;
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetSkipCombatEnabled(bool bEnabled)
{
	CurrentSettings.bSkipCombatEnabled = bEnabled;
	BroadcastSettingsChanged();
}

// ============================================================================
// Photosensitivity
// ============================================================================

void USEEAccessibilitySubsystem::SetPhotosensitivityMode(bool bEnabled)
{
	CurrentSettings.bPhotosensitivityMode = bEnabled;

	if (bEnabled)
	{
		CurrentSettings.bReduceBrightFlashes = true;
		CurrentSettings.bReduceRapidColorChanges = true;
		CurrentSettings.bFlashingReduction = true;
	}

	ApplyVisualSettings();
	BroadcastSettingsChanged();
}

// ============================================================================
// Audio Accessibility
// ============================================================================

void USEEAccessibilitySubsystem::SetMonoAudio(bool bEnabled)
{
	CurrentSettings.bMonoAudio = bEnabled;
	ApplyAudioSettings();
	BroadcastSettingsChanged();
}

void USEEAccessibilitySubsystem::SetAudioVisualization(EAudioVisualization Mode)
{
	CurrentSettings.AudioVisualization = Mode;
	BroadcastSettingsChanged();
}

// ============================================================================
// Persistence
// ============================================================================

void USEEAccessibilitySubsystem::SaveSettingsToConfig()
{
	GConfig->SetBool(TEXT("Accessibility"), TEXT("SubtitlesEnabled"), CurrentSettings.bSubtitlesEnabled, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("SpeakerIDMode"), static_cast<int32>(CurrentSettings.SpeakerIDMode), GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("SubtitleSize"), static_cast<int32>(CurrentSettings.SubtitleSize), GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("SubtitleBackground"), static_cast<int32>(CurrentSettings.SubtitleBackground), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("ClosedCaptions"), CurrentSettings.bClosedCaptions, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("ColorblindMode"), static_cast<int32>(CurrentSettings.ColorblindMode), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("HighContrastMode"), CurrentSettings.bHighContrastMode, GGameIni);
	GConfig->SetFloat(TEXT("Accessibility"), TEXT("ScreenShakeIntensity"), CurrentSettings.ScreenShakeIntensity, GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("MotionBlur"), CurrentSettings.bMotionBlur, GGameIni);
	GConfig->SetFloat(TEXT("Accessibility"), TEXT("FieldOfView"), CurrentSettings.FieldOfView, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("HUDScale"), static_cast<int32>(CurrentSettings.HUDScale), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("FlashingReduction"), CurrentSettings.bFlashingReduction, GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("DyslexiaFriendlyFont"), CurrentSettings.bDyslexiaFriendlyFont, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("GlobalTextSize"), static_cast<int32>(CurrentSettings.GlobalTextSize), GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("QTEMode"), static_cast<int32>(CurrentSettings.QTEMode), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("MonoAudio"), CurrentSettings.bMonoAudio, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("AudioVisualization"), static_cast<int32>(CurrentSettings.AudioVisualization), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("PhotosensitivityMode"), CurrentSettings.bPhotosensitivityMode, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("AutoAimLevel"), static_cast<int32>(CurrentSettings.AutoAimLevel), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("LockOnTargeting"), CurrentSettings.bLockOnTargeting, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("NavigationAssist"), static_cast<int32>(CurrentSettings.NavigationAssist), GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("SkipCombat"), CurrentSettings.bSkipCombatEnabled, GGameIni);
	GConfig->SetBool(TEXT("Accessibility"), TEXT("MenuNarration"), CurrentSettings.bMenuNarration, GGameIni);
	GConfig->SetInt(TEXT("Accessibility"), TEXT("OneHandedPreset"), static_cast<int32>(CurrentSettings.OneHandedPreset), GGameIni);

	GConfig->Flush(false, GGameIni);
}

void USEEAccessibilitySubsystem::LoadSettingsFromConfig()
{
	bool bBool;
	int32 IntVal;
	float FloatVal;

	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("SubtitlesEnabled"), bBool, GGameIni))
		CurrentSettings.bSubtitlesEnabled = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("SpeakerIDMode"), IntVal, GGameIni))
		CurrentSettings.SpeakerIDMode = static_cast<ESpeakerIDMode>(IntVal);
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("SubtitleSize"), IntVal, GGameIni))
		CurrentSettings.SubtitleSize = static_cast<ESubtitleSize>(IntVal);
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("SubtitleBackground"), IntVal, GGameIni))
		CurrentSettings.SubtitleBackground = static_cast<ESubtitleBackground>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("ClosedCaptions"), bBool, GGameIni))
		CurrentSettings.bClosedCaptions = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("ColorblindMode"), IntVal, GGameIni))
		CurrentSettings.ColorblindMode = static_cast<EColorblindMode>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("HighContrastMode"), bBool, GGameIni))
		CurrentSettings.bHighContrastMode = bBool;
	if (GConfig->GetFloat(TEXT("Accessibility"), TEXT("ScreenShakeIntensity"), FloatVal, GGameIni))
		CurrentSettings.ScreenShakeIntensity = FloatVal;
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("MotionBlur"), bBool, GGameIni))
		CurrentSettings.bMotionBlur = bBool;
	if (GConfig->GetFloat(TEXT("Accessibility"), TEXT("FieldOfView"), FloatVal, GGameIni))
		CurrentSettings.FieldOfView = FloatVal;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("HUDScale"), IntVal, GGameIni))
		CurrentSettings.HUDScale = static_cast<EHUDScale>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("FlashingReduction"), bBool, GGameIni))
		CurrentSettings.bFlashingReduction = bBool;
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("DyslexiaFriendlyFont"), bBool, GGameIni))
		CurrentSettings.bDyslexiaFriendlyFont = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("GlobalTextSize"), IntVal, GGameIni))
		CurrentSettings.GlobalTextSize = static_cast<EGlobalTextSize>(IntVal);
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("QTEMode"), IntVal, GGameIni))
		CurrentSettings.QTEMode = static_cast<EQTEMode>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("MonoAudio"), bBool, GGameIni))
		CurrentSettings.bMonoAudio = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("AudioVisualization"), IntVal, GGameIni))
		CurrentSettings.AudioVisualization = static_cast<EAudioVisualization>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("PhotosensitivityMode"), bBool, GGameIni))
		CurrentSettings.bPhotosensitivityMode = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("AutoAimLevel"), IntVal, GGameIni))
		CurrentSettings.AutoAimLevel = static_cast<EAutoAimLevel>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("LockOnTargeting"), bBool, GGameIni))
		CurrentSettings.bLockOnTargeting = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("NavigationAssist"), IntVal, GGameIni))
		CurrentSettings.NavigationAssist = static_cast<ENavigationAssist>(IntVal);
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("SkipCombat"), bBool, GGameIni))
		CurrentSettings.bSkipCombatEnabled = bBool;
	if (GConfig->GetBool(TEXT("Accessibility"), TEXT("MenuNarration"), bBool, GGameIni))
		CurrentSettings.bMenuNarration = bBool;
	if (GConfig->GetInt(TEXT("Accessibility"), TEXT("OneHandedPreset"), IntVal, GGameIni))
		CurrentSettings.OneHandedPreset = static_cast<EOneHandedPreset>(IntVal);

	ApplyVisualSettings();
	ApplyAudioSettings();
	ApplyControlSettings();
}

// ============================================================================
// Internal Application
// ============================================================================

void USEEAccessibilitySubsystem::BroadcastSettingsChanged()
{
	OnSettingsChanged.Broadcast(CurrentSettings);
}

void USEEAccessibilitySubsystem::ApplyVisualSettings()
{
	// Apply motion blur, film grain, chromatic aberration via console commands
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			PC->ConsoleCommand(
				FString::Printf(TEXT("r.MotionBlurQuality %d"), CurrentSettings.bMotionBlur ? 3 : 0));
			PC->ConsoleCommand(
				FString::Printf(TEXT("r.FilmGrain %d"), CurrentSettings.bFilmGrain ? 1 : 0));
			PC->ConsoleCommand(
				FString::Printf(TEXT("r.SceneColorFringeQuality %d"), CurrentSettings.bChromaticAberration ? 1 : 0));
		}
	}
}

void USEEAccessibilitySubsystem::ApplyAudioSettings()
{
	// Mono audio is handled by the audio engine's channel mapping
	// Audio visualization is handled by the HUD widget layer
}

void USEEAccessibilitySubsystem::ApplyControlSettings()
{
	// One-handed presets are applied through the Enhanced Input subsystem
	// The actual input mapping changes are handled by the input configuration layer
}
