// SSEEUIStyle.h - Shared dark-industrial palette and button styles for all SEE Slate UI.
// No external assets: colors via FSlateColorBrush, fonts via FCoreStyle's default font.
#pragma once

#include "CoreMinimal.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Brushes/SlateColorBrush.h"

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
}
