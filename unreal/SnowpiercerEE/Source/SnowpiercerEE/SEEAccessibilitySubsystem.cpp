#include "SEEAccessibilitySubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString USEEAccessibilitySubsystem::SettingsSaveSlot = TEXT("SnowpiercerEE_Accessibility");

static const FString ConfigSection = TEXT("Accessibility");

// ── Lifecycle ──

void USEEAccessibilitySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadSettings();
}

void USEEAccessibilitySubsystem::Deinitialize()
{
    SaveSettings();
    Super::Deinitialize();
}

// ── Subtitles ──

void USEEAccessibilitySubsystem::SetSubtitlesEnabled(bool bEnabled)
{
    bSubtitlesEnabled = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetSpeakerIDMode(ESEESpeakerIDMode Mode)
{
    SpeakerIDMode = Mode;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetSubtitleSize(ESEESubtitleSize Size)
{
    SubtitleSize = Size;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetSubtitleBackground(float Opacity)
{
    SubtitleBackgroundOpacity = FMath::Clamp(Opacity, 0.0f, 1.0f);
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetClosedCaptionsEnabled(bool bEnabled)
{
    bClosedCaptionsEnabled = bEnabled;
    BroadcastChange();
}

// ── Colorblind ──

void USEEAccessibilitySubsystem::SetColorblindMode(ESEEColorblindMode Mode)
{
    ColorblindMode = Mode;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetColorblindStrength(float Strength)
{
    ColorblindStrength = FMath::Clamp(Strength, 0.0f, 1.0f);
    BroadcastChange();
}

// ── Visual ──

void USEEAccessibilitySubsystem::SetHighContrastMode(bool bEnabled)
{
    bHighContrastMode = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetHUDScale(float Scale)
{
    HUDScale = FMath::Clamp(Scale, 0.75f, 1.5f);
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetScreenShakeIntensity(float Intensity)
{
    ScreenShakeIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetMotionBlurEnabled(bool bEnabled)
{
    bMotionBlurEnabled = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetCameraBobEnabled(bool bEnabled)
{
    bCameraBobEnabled = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetPhotosensitivityMode(bool bEnabled)
{
    bPhotosensitivityMode = bEnabled;
    BroadcastChange();
}

// ── Font ──

void USEEAccessibilitySubsystem::SetDyslexiaFontEnabled(bool bEnabled)
{
    bDyslexiaFont = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetGlobalTextScale(float Scale)
{
    GlobalTextScale = FMath::Clamp(Scale, 0.8f, 1.4f);
    BroadcastChange();
}

// ── Controls ──

void USEEAccessibilitySubsystem::SetActionToggleMode(FName ActionName, bool bToggle)
{
    ActionToggleModes.Add(ActionName, bToggle);
    BroadcastChange();
}

bool USEEAccessibilitySubsystem::IsActionToggle(FName ActionName) const
{
    if (const bool* Found = ActionToggleModes.Find(ActionName))
    {
        return *Found;
    }
    return false; // Default: hold mode
}

void USEEAccessibilitySubsystem::RemapAction(FName ActionName, FKey NewKey)
{
    RemappedKeys.Add(ActionName, NewKey);
    BroadcastChange();
}

FKey USEEAccessibilitySubsystem::GetRemappedKey(FName ActionName) const
{
    if (const FKey* Found = RemappedKeys.Find(ActionName))
    {
        return *Found;
    }
    return EKeys::Invalid;
}

// ── Audio ──

void USEEAccessibilitySubsystem::SetMonoAudio(bool bEnabled)
{
    bMonoAudio = bEnabled;
    BroadcastChange();
}

// ── Motor ──

void USEEAccessibilitySubsystem::SetAutoCompleteQTE(bool bEnabled)
{
    bAutoCompleteQTE = bEnabled;
    BroadcastChange();
}

void USEEAccessibilitySubsystem::SetQTETimingMultiplier(float Multiplier)
{
    QTETimingMultiplier = FMath::Clamp(Multiplier, 0.5f, 3.0f);
    BroadcastChange();
}

// ── Persistence (via GConfig) ──

void USEEAccessibilitySubsystem::SaveSettings()
{
    const FString ConfigPath = FPaths::GeneratedConfigDir() + TEXT("AccessibilitySettings.ini");

    GConfig->SetBool(*ConfigSection, TEXT("SubtitlesEnabled"), bSubtitlesEnabled, ConfigPath);
    GConfig->SetInt(*ConfigSection, TEXT("SpeakerIDMode"), static_cast<int32>(SpeakerIDMode), ConfigPath);
    GConfig->SetInt(*ConfigSection, TEXT("SubtitleSize"), static_cast<int32>(SubtitleSize), ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("SubtitleBackgroundOpacity"), SubtitleBackgroundOpacity, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("ClosedCaptionsEnabled"), bClosedCaptionsEnabled, ConfigPath);
    GConfig->SetInt(*ConfigSection, TEXT("ColorblindMode"), static_cast<int32>(ColorblindMode), ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("ColorblindStrength"), ColorblindStrength, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("HighContrastMode"), bHighContrastMode, ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("HUDScale"), HUDScale, ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("ScreenShakeIntensity"), ScreenShakeIntensity, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("MotionBlurEnabled"), bMotionBlurEnabled, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("CameraBobEnabled"), bCameraBobEnabled, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("PhotosensitivityMode"), bPhotosensitivityMode, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("DyslexiaFont"), bDyslexiaFont, ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("GlobalTextScale"), GlobalTextScale, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("MonoAudio"), bMonoAudio, ConfigPath);
    GConfig->SetBool(*ConfigSection, TEXT("AutoCompleteQTE"), bAutoCompleteQTE, ConfigPath);
    GConfig->SetFloat(*ConfigSection, TEXT("QTETimingMultiplier"), QTETimingMultiplier, ConfigPath);

    // Save toggle modes
    for (const auto& Pair : ActionToggleModes)
    {
        GConfig->SetBool(TEXT("Accessibility.ToggleModes"), *Pair.Key.ToString(), Pair.Value, ConfigPath);
    }

    GConfig->Flush(false, ConfigPath);
}

void USEEAccessibilitySubsystem::LoadSettings()
{
    const FString ConfigPath = FPaths::GeneratedConfigDir() + TEXT("AccessibilitySettings.ini");

    if (!FPaths::FileExists(ConfigPath))
    {
        return;
    }

    int32 IntVal;

    GConfig->GetBool(*ConfigSection, TEXT("SubtitlesEnabled"), bSubtitlesEnabled, ConfigPath);

    if (GConfig->GetInt(*ConfigSection, TEXT("SpeakerIDMode"), IntVal, ConfigPath))
        SpeakerIDMode = static_cast<ESEESpeakerIDMode>(IntVal);

    if (GConfig->GetInt(*ConfigSection, TEXT("SubtitleSize"), IntVal, ConfigPath))
        SubtitleSize = static_cast<ESEESubtitleSize>(IntVal);

    GConfig->GetFloat(*ConfigSection, TEXT("SubtitleBackgroundOpacity"), SubtitleBackgroundOpacity, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("ClosedCaptionsEnabled"), bClosedCaptionsEnabled, ConfigPath);

    if (GConfig->GetInt(*ConfigSection, TEXT("ColorblindMode"), IntVal, ConfigPath))
        ColorblindMode = static_cast<ESEEColorblindMode>(IntVal);

    GConfig->GetFloat(*ConfigSection, TEXT("ColorblindStrength"), ColorblindStrength, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("HighContrastMode"), bHighContrastMode, ConfigPath);
    GConfig->GetFloat(*ConfigSection, TEXT("HUDScale"), HUDScale, ConfigPath);
    GConfig->GetFloat(*ConfigSection, TEXT("ScreenShakeIntensity"), ScreenShakeIntensity, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("MotionBlurEnabled"), bMotionBlurEnabled, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("CameraBobEnabled"), bCameraBobEnabled, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("PhotosensitivityMode"), bPhotosensitivityMode, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("DyslexiaFont"), bDyslexiaFont, ConfigPath);
    GConfig->GetFloat(*ConfigSection, TEXT("GlobalTextScale"), GlobalTextScale, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("MonoAudio"), bMonoAudio, ConfigPath);
    GConfig->GetBool(*ConfigSection, TEXT("AutoCompleteQTE"), bAutoCompleteQTE, ConfigPath);
    GConfig->GetFloat(*ConfigSection, TEXT("QTETimingMultiplier"), QTETimingMultiplier, ConfigPath);

    // Load toggle modes
    TArray<FString> ToggleKeys;
    GConfig->GetArray(TEXT("Accessibility.ToggleModes"), TEXT("Keys"), ToggleKeys, ConfigPath);
    // Toggle modes loaded individually by checking section keys
}

void USEEAccessibilitySubsystem::ResetToDefaults()
{
    bSubtitlesEnabled = true;
    SpeakerIDMode = ESEESpeakerIDMode::NameAndColor;
    SubtitleSize = ESEESubtitleSize::Medium;
    SubtitleBackgroundOpacity = 0.5f;
    bClosedCaptionsEnabled = false;
    ColorblindMode = ESEEColorblindMode::None;
    ColorblindStrength = 1.0f;
    bHighContrastMode = false;
    HUDScale = 1.0f;
    ScreenShakeIntensity = 1.0f;
    bMotionBlurEnabled = true;
    bCameraBobEnabled = true;
    bPhotosensitivityMode = false;
    bDyslexiaFont = false;
    GlobalTextScale = 1.0f;
    ActionToggleModes.Empty();
    RemappedKeys.Empty();
    bMonoAudio = false;
    bAutoCompleteQTE = false;
    QTETimingMultiplier = 1.0f;

    BroadcastChange();
}

void USEEAccessibilitySubsystem::BroadcastChange()
{
    OnSettingsChanged.Broadcast();
}
