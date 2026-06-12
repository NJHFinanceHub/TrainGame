// SSEEPanelFrame.cpp - Riveted-steel panel chrome implementation
#include "SSEEPanelFrame.h"
#include "SSEEUIStyle.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

void SSEEPanelFrame::Construct(const FArguments& InArgs)
{
	if (InArgs._bShowTexture)
	{
		TextureWashBrush = SEEUIStyle::CreatePanelTextureBrush();
	}

	TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);

	// Title strip: stencil caps + thin amber underline
	if (!InArgs._Title.IsEmpty())
	{
		Body->AddSlot()
		.AutoHeight()
		.Padding(FMargin(28.0f, 20.0f, 28.0f, 0.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(InArgs._Title)
				.Font(SEEUIStyle::HeadingFont(20))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::BoneText))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
			[
				SNew(SBox)
				.HeightOverride(2.0f)
				[
					SNew(SBorder)
					.BorderImage(SEEUIStyle::WhiteBrush())
					.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::EngineAmber, 0.85f))
					.Padding(FMargin(0.0f))
				]
			]
		];
	}

	// Content
	Body->AddSlot()
	.AutoHeight()
	.Padding(InArgs._ContentPadding)
	[
		InArgs._Content.Widget
	];

	TSharedRef<SOverlay> Interior = SNew(SOverlay);

	// Texture wash behind everything (transparent fallback when asset missing)
	if (TextureWashBrush.IsValid())
	{
		Interior->AddSlot()
		[
			SNew(SImage)
			.Image(TextureWashBrush.Get())
		];
	}

	Interior->AddSlot()
	[
		InArgs._MinWidth > 0.0f
			? StaticCastSharedRef<SWidget>(SNew(SBox).MinDesiredWidth(InArgs._MinWidth)[Body])
			: StaticCastSharedRef<SWidget>(Body)
	];

	ChildSlot
	[
		// Outer 2px rivet-line border
		SNew(SBorder)
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::RivetLine)
		.Padding(FMargin(2.0f))
		[
			// Gunmetal interior
			SNew(SBorder)
			.BorderImage(SEEUIStyle::WhiteBrush())
			.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::Gunmetal, 0.98f))
			.Padding(FMargin(0.0f))
			[
				Interior
			]
		]
	];
}
