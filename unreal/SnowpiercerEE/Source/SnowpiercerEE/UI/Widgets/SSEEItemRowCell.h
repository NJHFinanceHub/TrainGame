// SSEEItemRowCell.h - Rarity-framed inventory row cell.
//
// A self-contained presentation wrapper used by SSEEInventoryScreen's item list.
// It gives each inventory row the "framed cell" look:
//   - a thick rarity-colored accent strip down the left edge
//   - a 1px rarity-tinted border that brightens when the row is the selected one
//   - a gunmetal interior that lifts toward the rarity tint while selected
//
// All visual state is driven by TAttributes so the parent never has to rebuild
// the list to reflect selection changes - it just flips the selected index and
// the lambdas re-evaluate on the next paint. Pure presentation; carries no
// inventory logic, no drag/equip behavior (that stays in the parent's content).
//
// UE 5.7-safe: only uses SBorder/SBox/SHorizontalBox declarative args that exist
// in 5.7 (BorderBackgroundColor_Lambda is a real declarative arg on SBorder).
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Styling/SlateColor.h"
#include "SSEEUIStyle.h"

class SSEEItemRowCell : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEItemRowCell)
		: _RarityColor(SEEUIStyle::RarityCommon)
		, _RowHeight(36.0f)
	{}
		/** Accent / border color for this row (the item's rarity color). */
		SLATE_ARGUMENT(FLinearColor, RarityColor)
		/** Fixed row height. */
		SLATE_ARGUMENT(float, RowHeight)
		/** True while this row is the selected one (drives the bright border). */
		SLATE_ATTRIBUTE(bool, IsSelected)
		/** The row's interactive content (the parent's button + drag overlay). */
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		RarityColor = InArgs._RarityColor;
		IsSelected  = InArgs._IsSelected;

		ChildSlot
		[
			SNew(SBox)
			.HeightOverride(InArgs._RowHeight)
			.Padding(FMargin(0.0f, 1.0f))
			[
				// Outer border = rarity-tinted frame; brightens when selected.
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor_Lambda([this]() -> FLinearColor { return GetFrameColor(); })
				.Padding(FMargin(1.0f))
				[
					SNew(SHorizontalBox)

					// Left rarity accent strip.
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(3.0f)
						[
							SNew(SBorder)
							.BorderImage(SEEUIStyle::WhiteBrush())
							.BorderBackgroundColor(RarityColor)
							.Padding(FMargin(0.0f))
							[ SNullWidget::NullWidget ]
						]
					]

					// Interior body (gunmetal, lifts toward rarity when selected).
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SBorder)
						.BorderImage(SEEUIStyle::WhiteBrush())
						.BorderBackgroundColor_Lambda([this]() -> FLinearColor { return GetBodyColor(); })
						.Padding(FMargin(0.0f))
						.VAlign(VAlign_Fill)
						[
							InArgs._Content.Widget
						]
					]
				]
			]
		];
	}

private:
	bool IsRowSelected() const { return IsSelected.Get(false); }

	/** Frame: dim rarity tint normally, bright rarity when selected. */
	FLinearColor GetFrameColor() const
	{
		return IsRowSelected()
			? RarityColor
			: SEEUIStyle::Dim(RarityColor, 0.30f);
	}

	/** Body: gunmetal normally, a low-alpha rarity wash over gunmetal when selected. */
	FLinearColor GetBodyColor() const
	{
		if (IsRowSelected())
		{
			// Blend a touch of the rarity hue into the selected-row fill.
			const FLinearColor Base = SEEUIStyle::RowSelected;
			return FLinearColor(
				FMath::Lerp(Base.R, RarityColor.R, 0.18f),
				FMath::Lerp(Base.G, RarityColor.G, 0.18f),
				FMath::Lerp(Base.B, RarityColor.B, 0.18f),
				1.0f);
		}
		return SEEUIStyle::RowNormal;
	}

	FLinearColor       RarityColor = SEEUIStyle::RarityCommon;
	TAttribute<bool>   IsSelected;
};
