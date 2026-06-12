// SSEEPanelFrame.h - Riveted-steel bordered panel shared by all SEE menus.
// Outer rivet-line border, gunmetal body, optional darkened metal-plate wash,
// stencil-caps title strip with a thin amber underline.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

struct FSlateBrush;

/**
 * SSEEPanelFrame
 *
 * The standard panel chrome of the Eternal Engine design system:
 * - 2px rivet-line outer border
 * - Gunmetal interior with an optional low-opacity metal_plates texture wash
 *   (flat-color fallback when the texture asset is missing)
 * - Top title strip: stencil-caps title + thin engine-amber underline
 * - Content slot below
 */
class SSEEPanelFrame : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEPanelFrame)
		: _Title()
		, _bShowTexture(true)
		, _ContentPadding(FMargin(28.0f, 22.0f, 28.0f, 26.0f))
		, _MinWidth(0.0f)
	{}
		/** Stencil-caps title shown in the top strip. Empty hides the strip. */
		SLATE_ARGUMENT(FText, Title)
		/** Draw the darkened metal texture wash behind the content. */
		SLATE_ARGUMENT(bool, bShowTexture)
		SLATE_ARGUMENT(FMargin, ContentPadding)
		/** Optional minimum interior width (0 = size to content). */
		SLATE_ARGUMENT(float, MinWidth)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	/** Owns the (optional) texture wash brush for the widget's lifetime. */
	TSharedPtr<FSlateBrush> TextureWashBrush;
};
