// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AccessibilityTypes.generated.h"

// ============================================================================
// Accessibility System Type Definitions
// Comprehensive accessibility options for Snowpiercer: Eternal Engine
// ============================================================================

// --- Subtitle & Caption ---

UENUM(BlueprintType)
enum class ESpeakerIDMode : uint8
{
	Off			UMETA(DisplayName = "Off"),
	NameOnly	UMETA(DisplayName = "Name Only"),
	NameColor	UMETA(DisplayName = "Name + Color")
};

UENUM(BlueprintType)
enum class ESpeakerDirection : uint8
{
	Off		UMETA(DisplayName = "Off"),
	Arrow	UMETA(DisplayName = "Arrow"),
	Compass	UMETA(DisplayName = "Compass")
};

UENUM(BlueprintType)
enum class ESubtitleBackground : uint8
{
	None			UMETA(DisplayName = "None"),
	SemiTransparent	UMETA(DisplayName = "Semi-Transparent"),
	Opaque			UMETA(DisplayName = "Opaque")
};

UENUM(BlueprintType)
enum class ESubtitleSize : uint8
{
	Small		UMETA(DisplayName = "Small"),
	Medium		UMETA(DisplayName = "Medium"),
	Large		UMETA(DisplayName = "Large"),
	ExtraLarge	UMETA(DisplayName = "Extra Large")
};

UENUM(BlueprintType)
enum class ECaptionDetail : uint8
{
	Minimal		UMETA(DisplayName = "Minimal"),
	Descriptive	UMETA(DisplayName = "Descriptive")
};

// --- Colorblind ---

UENUM(BlueprintType)
enum class EColorblindMode : uint8
{
	Off				UMETA(DisplayName = "Off"),
	Protanopia		UMETA(DisplayName = "Protanopia (Red-Weak)"),
	Deuteranopia	UMETA(DisplayName = "Deuteranopia (Green-Weak)"),
	Tritanopia		UMETA(DisplayName = "Tritanopia (Blue-Weak)"),
	Custom			UMETA(DisplayName = "Custom")
};

// --- Audio Visualization ---

UENUM(BlueprintType)
enum class EAudioVisualization : uint8
{
	Off			UMETA(DisplayName = "Off"),
	Radar		UMETA(DisplayName = "Radar"),
	Waveform	UMETA(DisplayName = "Waveform")
};

// --- QTE ---

UENUM(BlueprintType)
enum class EQTEMode : uint8
{
	Normal			UMETA(DisplayName = "Normal"),
	Simplified		UMETA(DisplayName = "Simplified"),
	AutoComplete	UMETA(DisplayName = "Auto-Complete")
};

UENUM(BlueprintType)
enum class ERepeatedPressMode : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	HoldToFill	UMETA(DisplayName = "Hold to Fill"),
	Auto		UMETA(DisplayName = "Auto")
};

UENUM(BlueprintType)
enum class EInputTolerance : uint8
{
	Tight		UMETA(DisplayName = "Tight"),
	Normal		UMETA(DisplayName = "Normal"),
	Generous	UMETA(DisplayName = "Generous")
};

// --- Navigation Assist ---

UENUM(BlueprintType)
enum class ENavigationAssist : uint8
{
	Off				UMETA(DisplayName = "Off"),
	BreadcrumbTrail	UMETA(DisplayName = "Breadcrumb Trail"),
	WaypointPulse	UMETA(DisplayName = "Waypoint Pulse")
};

// --- Puzzle Hints ---

UENUM(BlueprintType)
enum class EPuzzleHintMode : uint8
{
	Off			UMETA(DisplayName = "Off"),
	After3Min	UMETA(DisplayName = "After 3 Minutes"),
	After5Min	UMETA(DisplayName = "After 5 Minutes"),
	OnRequest	UMETA(DisplayName = "On Request")
};

// --- Text Weight ---

UENUM(BlueprintType)
enum class ETextWeight : uint8
{
	Normal	UMETA(DisplayName = "Normal"),
	Bold	UMETA(DisplayName = "Bold")
};

// --- Letter Spacing ---

UENUM(BlueprintType)
enum class ELetterSpacing : uint8
{
	Normal		UMETA(DisplayName = "Normal"),
	Wide		UMETA(DisplayName = "Wide"),
	ExtraWide	UMETA(DisplayName = "Extra Wide")
};

// --- Auto-Aim Level ---

UENUM(BlueprintType)
enum class EAutoAimLevel : uint8
{
	Off		UMETA(DisplayName = "Off"),
	Low		UMETA(DisplayName = "Low"),
	Medium	UMETA(DisplayName = "Medium"),
	High	UMETA(DisplayName = "High")
};

// --- Lock-On Switch Mode ---

UENUM(BlueprintType)
enum class ELockOnSwitch : uint8
{
	Nearest			UMETA(DisplayName = "Nearest"),
	LeftRightFlick	UMETA(DisplayName = "Left-Right Flick")
};

// --- Simultaneous Input ---

UENUM(BlueprintType)
enum class ESimultaneousInput : uint8
{
	Required	UMETA(DisplayName = "Required"),
	Sequential	UMETA(DisplayName = "Sequential")
};

// --- One-Handed Presets ---

UENUM(BlueprintType)
enum class EOneHandedPreset : uint8
{
	None				UMETA(DisplayName = "None"),
	LeftHandKeyboard	UMETA(DisplayName = "Left Hand Only (Keyboard)"),
	RightHandKeyboard	UMETA(DisplayName = "Right Hand Only (Keyboard)"),
	LeftHandGamepad		UMETA(DisplayName = "Left Hand Only (Gamepad)"),
	RightHandGamepad	UMETA(DisplayName = "Right Hand Only (Gamepad)")
};

// --- Global Text Size ---

UENUM(BlueprintType)
enum class EGlobalTextSize : uint8
{
	Size80		UMETA(DisplayName = "80%"),
	Size100		UMETA(DisplayName = "100%"),
	Size120		UMETA(DisplayName = "120%"),
	Size140		UMETA(DisplayName = "140%")
};

// --- HUD Scale ---

UENUM(BlueprintType)
enum class EHUDScale : uint8
{
	Scale75		UMETA(DisplayName = "75%"),
	Scale100	UMETA(DisplayName = "100%"),
	Scale125	UMETA(DisplayName = "125%"),
	Scale150	UMETA(DisplayName = "150%")
};

// ============================================================================
// Structs
// ============================================================================

/** Speaker color assignment for subtitle system */
USTRUCT(BlueprintType)
struct FSpeakerColorConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PlayerColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CompanionColor = FLinearColor(0.f, 1.f, 1.f); // Cyan

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HostileColor = FLinearColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor NeutralColor = FLinearColor::Yellow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor IntercomColor = FLinearColor(0.5f, 0.5f, 0.5f); // Grey

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor WilfordColor = FLinearColor(1.f, 0.84f, 0.f); // Gold
};

/** Custom colorblind hue rotation per channel */
USTRUCT(BlueprintType)
struct FCustomColorblindConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "360"))
	float RedHueRotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "360"))
	float GreenHueRotation = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "360"))
	float BlueHueRotation = 0.f;
};

/** Toggle vs hold settings for individual actions */
USTRUCT(BlueprintType)
struct FToggleHoldSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSprintToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrouchToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAimToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBlockToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInteractToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bGrabToggle = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBreathHoldToggle = false;
};

/** Complete accessibility settings snapshot (for save/load) */
USTRUCT(BlueprintType)
struct FAccessibilitySettings
{
	GENERATED_BODY()

	// --- Subtitles ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	bool bSubtitlesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ESpeakerIDMode SpeakerIDMode = ESpeakerIDMode::NameColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ESpeakerDirection SpeakerDirection = ESpeakerDirection::Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ESubtitleBackground SubtitleBackground = ESubtitleBackground::SemiTransparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ESubtitleSize SubtitleSize = ESubtitleSize::Medium;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	int32 SubtitleMaxLines = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	bool bClosedCaptions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	ECaptionDetail CaptionDetail = ECaptionDetail::Descriptive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitles")
	FSpeakerColorConfig SpeakerColors;

	// --- Colorblind ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colorblind")
	EColorblindMode ColorblindMode = EColorblindMode::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Colorblind")
	FCustomColorblindConfig CustomColorblind;

	// --- Visual ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bHighContrastMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "50", ClampMax = "200"))
	float HighContrastIntensity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	EHUDScale HUDScale = EHUDScale::Scale100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "25", ClampMax = "100"))
	float HUDOpacity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0", ClampMax = "100"))
	float ScreenShakeIntensity = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bMotionBlur = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0", ClampMax = "100"))
	float CameraBob = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "60", ClampMax = "120"))
	float FieldOfView = 90.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bFilmGrain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bChromaticAberration = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bFlashingReduction = false;

	// --- Font & Text ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	bool bDyslexiaFriendlyFont = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	EGlobalTextSize GlobalTextSize = EGlobalTextSize::Size100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	ETextWeight TextWeight = ETextWeight::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	ELetterSpacing LetterSpacing = ELetterSpacing::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
	bool bMenuNarration = false;

	// --- Controls ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	FToggleHoldSettings ToggleHoldSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Controls")
	EOneHandedPreset OneHandedPreset = EOneHandedPreset::None;

	// --- Aim Assistance ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	EAutoAimLevel AutoAimLevel = EAutoAimLevel::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bAimAssistSticky = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim", meta = (ClampMin = "0", ClampMax = "100"))
	float AimAssistStrength = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bLockOnTargeting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	ELockOnSwitch LockOnSwitchMode = ELockOnSwitch::Nearest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aim")
	bool bAimSnapToTarget = false;

	// --- Audio ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bMonoAudio = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	EAudioVisualization AudioVisualization = EAudioVisualization::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bDirectionalAudioIndicator = false;

	// --- Motor ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	EQTEMode QTEMode = EQTEMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float QTETimingMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	ERepeatedPressMode RepeatedPressMode = ERepeatedPressMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	EInputTolerance InputTolerance = EInputTolerance::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor", meta = (ClampMin = "25", ClampMax = "200"))
	float MenuCursorSpeed = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor", meta = (ClampMin = "200", ClampMax = "800"))
	float DoubleClickSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motor")
	ESimultaneousInput SimultaneousInputMode = ESimultaneousInput::Required;

	// --- Gameplay Assists ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	ENavigationAssist NavigationAssist = ENavigationAssist::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	EPuzzleHintMode PuzzleHintMode = EPuzzleHintMode::Off;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bSkipCombatEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bInventoryAutoSort = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bItemComparisonTooltip = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bThreatIndicator = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bTutorialReminders = true;

	// --- Photosensitivity ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photosensitivity")
	bool bPhotosensitivityMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photosensitivity")
	bool bReduceBrightFlashes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photosensitivity")
	bool bReduceRapidColorChanges = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photosensitivity")
	bool bScreenBrightnessLimiter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photosensitivity", meta = (ClampMin = "50", ClampMax = "90"))
	float BrightnessLimiterCap = 80.f;
};

// Delegate for settings changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccessibilitySettingsChanged, const FAccessibilitySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorblindModeChanged, EColorblindMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSubtitleSettingsChanged, bool, bEnabled);
