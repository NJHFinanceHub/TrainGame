// SSEEUIStyle.h - Shared dark-industrial palette and button styles for all SEE Slate UI.
// "The Eternal Engine" design system: riveted steel, frost, engine-amber vs frozen-blue.
// Textures are optional (every loader is null-guarded with a flat-color fallback);
// fonts come from FCoreStyle's default font used bold at display sizes with letter-spacing.
#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateImageBrush.h"
#include "Brushes/SlateNoResource.h"
#include "Fonts/SlateFontInfo.h"
#include "Engine/Texture2D.h"
#include "UObject/UObjectGlobals.h"

namespace SEEUIStyle
{
	// --- Palette (dark industrial: cold steel, brass accents) ---
	inline const FLinearColor ScreenBackdrop = FLinearColor(0.008f, 0.010f, 0.014f, 0.92f);
	inline const FLinearColor MenuBackdrop   = FLinearColor(0.015f, 0.018f, 0.028f, 1.00f);
	inline const FLinearColor PanelDark      = FLinearColor(0.035f, 0.040f, 0.052f, 0.95f);
	inline const FLinearColor PanelMid       = FLinearColor(0.070f, 0.078f, 0.095f, 1.00f);
	inline const FLinearColor RowNormal      = FLinearColor(0.085f, 0.092f, 0.110f, 1.00f);
	inline const FLinearColor RowSelected    = FLinearColor(0.300f, 0.250f, 0.140f, 1.00f);
	inline const FLinearColor TabActive      = FLinearColor(0.300f, 0.250f, 0.150f, 1.00f);
	inline const FLinearColor TabInactive    = FLinearColor(0.100f, 0.100f, 0.120f, 1.00f);

	inline const FLinearColor AccentBrass    = FLinearColor(0.850f, 0.720f, 0.380f);
	inline const FLinearColor AccentSteel    = FLinearColor(0.550f, 0.650f, 0.800f);
	inline const FLinearColor TitleIce       = FLinearColor(0.850f, 0.900f, 1.000f);

	inline const FLinearColor TextPrimary    = FLinearColor(0.880f, 0.880f, 0.920f);
	inline const FLinearColor TextHeader     = FLinearColor(0.900f, 0.850f, 0.700f);
	inline const FLinearColor TextDim        = FLinearColor(0.550f, 0.550f, 0.600f);
	inline const FLinearColor TextFaint      = FLinearColor(0.350f, 0.350f, 0.400f);
	inline const FLinearColor TextDisabled   = FLinearColor(0.300f, 0.300f, 0.350f);

	inline const FLinearColor DangerRed      = FLinearColor(0.850f, 0.200f, 0.120f);
	inline const FLinearColor WarnAmber      = FLinearColor(0.900f, 0.650f, 0.150f);
	inline const FLinearColor OkGreen        = FLinearColor(0.350f, 0.750f, 0.350f);

	// --- Rarity colors (matches ESEEItemRarity order) ---
	inline const FLinearColor RarityCommon    = FLinearColor(0.75f, 0.75f, 0.75f);
	inline const FLinearColor RarityUncommon  = FLinearColor(0.35f, 0.80f, 0.35f);
	inline const FLinearColor RarityRare      = FLinearColor(0.35f, 0.55f, 1.00f);
	inline const FLinearColor RarityLegendary = FLinearColor(1.00f, 0.70f, 0.10f);

	// =====================================================================
	// "ETERNAL ENGINE" design system - riveted steel / frost / holy machine
	// =====================================================================

	// --- Core palette (sRGB hex spec) ---
	/** #0B0D10 - near-black steel, full-screen backgrounds. */
	inline const FLinearColor SteelBlack  = FLinearColor::FromSRGBColor(FColor(0x0B, 0x0D, 0x10));
	/** #161A1F - gunmetal panel fill. */
	inline const FLinearColor Gunmetal    = FLinearColor::FromSRGBColor(FColor(0x16, 0x1A, 0x1F));
	/** #2A313A - rivet-line panel borders and rules. */
	inline const FLinearColor RivetLine   = FLinearColor::FromSRGBColor(FColor(0x2A, 0x31, 0x3A));
	/** #7FB8D4 - frost blue accent: the frozen world outside. */
	inline const FLinearColor FrostBlue   = FLinearColor::FromSRGBColor(FColor(0x7F, 0xB8, 0xD4));
	/** #E8A33D - engine amber: the warm heart of the holy machine. */
	inline const FLinearColor EngineAmber = FLinearColor::FromSRGBColor(FColor(0xE8, 0xA3, 0x3D));
	/** #B3392E - blood red: danger, death, the tail. */
	inline const FLinearColor BloodRed    = FLinearColor::FromSRGBColor(FColor(0xB3, 0x39, 0x2E));
	/** #D8D4C8 - bone: primary reading text. */
	inline const FLinearColor BoneText    = FLinearColor::FromSRGBColor(FColor(0xD8, 0xD4, 0xC8));

	/** Gunmetal lightened for hover states. */
	inline const FLinearColor GunmetalHover = FLinearColor::FromSRGBColor(FColor(0x23, 0x2A, 0x32));

	/** Convenience: same color, different alpha. */
	inline FLinearColor Dim(const FLinearColor& In, float Alpha)
	{
		return FLinearColor(In.R, In.G, In.B, Alpha);
	}

	// --- Type scale (engine default font, bold + letter-spaced for stencil feel) ---
	// LetterSpacing is in 1/1000 em.
	inline FSlateFontInfo MakeFont(const FName& Weight, int32 Size, int32 LetterSpacing)
	{
		FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(Weight, Size);
		Font.LetterSpacing = LetterSpacing;
		return Font;
	}

	/** TITLE - 42-48 bold spaced caps. The masthead. */
	inline FSlateFontInfo TitleFont(int32 Size = 46)   { return MakeFont("Bold", Size, 240); }
	/** HEADING - 22 bold caps with stencil spacing. Panel titles. */
	inline FSlateFontInfo HeadingFont(int32 Size = 22) { return MakeFont("Bold", Size, 180); }
	/** BODY - 16 regular. Reading text. */
	inline FSlateFontInfo BodyFont(int32 Size = 16)    { return MakeFont("Regular", Size, 40); }
	/** CAPTION - 12 bold caps, wide tracking. Signage, version strips, taglines. */
	inline FSlateFontInfo CaptionFont(int32 Size = 12) { return MakeFont("Bold", Size, 160); }
	/** Small stencil overline - tiny, very wide tracking. */
	inline FSlateFontInfo OverlineFont(int32 Size = 13) { return MakeFont("Bold", Size, 420); }

	// --- Texture brushes (optional content; flat-color fallback when missing) ---

	/**
	 * Wrap an imported UTexture2D in a tiling FSlateImageBrush.
	 * The texture is rooted so GC cannot collect it out from under Slate.
	 * Returns a flat-color brush when the asset is absent (zero hard dependencies).
	 */
	inline TSharedRef<FSlateBrush> CreateTextureBrush(
		const TCHAR* AssetPath,
		const FLinearColor& Tint,
		const FLinearColor& FallbackColor,
		const FVector2D& ImageSize = FVector2D(512.0, 512.0))
	{
		if (UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, AssetPath))
		{
			// Slate brushes do not hold strong UObject references; root the texture
			// for the lifetime of the process (these are small, shared UI textures).
			if (!Texture->IsRooted())
			{
				Texture->AddToRoot();
			}
			return MakeShared<FSlateImageBrush>(Texture, ImageSize, Tint, ESlateBrushTileType::Both);
		}
		return MakeShared<FSlateColorBrush>(FallbackColor);
	}

	/** Dark metal-plate wash for panel interiors (very low alpha; layered over Gunmetal). */
	inline TSharedRef<FSlateBrush> CreatePanelTextureBrush()
	{
		return CreateTextureBrush(
			TEXT("/Game/Textures/Metals/metal_plates.metal_plates"),
			FLinearColor(0.16f, 0.18f, 0.21f, 0.10f),
			FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	}

	/** Full-screen scratched-steel wash for the main menu backdrop. */
	inline TSharedRef<FSlateBrush> CreateBackdropTextureBrush()
	{
		return CreateTextureBrush(
			TEXT("/Game/Textures/Metals/scratchy_metal_dark.scratchy_metal_dark"),
			FLinearColor(0.10f, 0.13f, 0.17f, 0.07f),
			FLinearColor(0.0f, 0.0f, 0.0f, 0.0f),
			FVector2D(1024.0, 1024.0));
	}

	/** Plain white box brush (tint via BorderBackgroundColor / ColorAndOpacity). */
	inline const FSlateBrush* WhiteBrush()
	{
		return FCoreStyle::Get().GetBrush("GenericWhiteBox");
	}

	// --- Button styles ---

	/** Solid-color menu button with hover/pressed feedback (no tint lambdas needed). */
	inline const FButtonStyle& GetMenuButtonStyle()
	{
		static const FButtonStyle Style = []()
		{
			FButtonStyle S = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			S.SetNormal(FSlateColorBrush(FLinearColor(0.095f, 0.105f, 0.135f, 1.0f)));
			S.SetHovered(FSlateColorBrush(FLinearColor(0.170f, 0.185f, 0.235f, 1.0f)));
			S.SetPressed(FSlateColorBrush(FLinearColor(0.050f, 0.055f, 0.075f, 1.0f)));
			S.SetDisabled(FSlateColorBrush(FLinearColor(0.060f, 0.062f, 0.075f, 1.0f)));
			S.SetNormalPadding(FMargin(2.0f));
			S.SetPressedPadding(FMargin(2.0f, 3.0f, 2.0f, 1.0f));
			return S;
		}();
		return Style;
	}

	/** White-brush button intended to be tinted via ButtonColorAndOpacity (list rows, tabs).
	 *  Hover brightens, press darkens the supplied tint. */
	inline const FButtonStyle& GetRowButtonStyle()
	{
		static const FButtonStyle Style = []()
		{
			FButtonStyle S = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			S.SetNormal(FSlateColorBrush(FLinearColor::White));
			S.SetHovered(FSlateColorBrush(FLinearColor(1.35f, 1.35f, 1.35f, 1.0f)));
			S.SetPressed(FSlateColorBrush(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f)));
			S.SetDisabled(FSlateColorBrush(FLinearColor(0.40f, 0.40f, 0.40f, 1.0f)));
			S.SetNormalPadding(FMargin(0.0f));
			S.SetPressedPadding(FMargin(0.0f));
			return S;
		}();
		return Style;
	}

	/** Fully transparent button chrome - for widgets that draw their own states
	 *  (SSEEMenuButton). Keeps click/focus handling without double-drawing. */
	inline const FButtonStyle& GetHollowButtonStyle()
	{
		static const FButtonStyle Style = []()
		{
			FButtonStyle S = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
			S.SetNormal(FSlateNoResource());
			S.SetHovered(FSlateNoResource());
			S.SetPressed(FSlateNoResource());
			S.SetDisabled(FSlateNoResource());
			S.SetNormalPadding(FMargin(0.0f));
			S.SetPressedPadding(FMargin(0.0f));
			return S;
		}();
		return Style;
	}
}
