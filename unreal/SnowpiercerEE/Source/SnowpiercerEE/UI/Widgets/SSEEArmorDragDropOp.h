// SSEEArmorDragDropOp.h - Drag payload for dragging armor items from the inventory list
//                         onto the paper-doll slot cells.
#pragma once

#include "CoreMinimal.h"
#include "Input/DragAndDrop.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "UI/Widgets/SSEEUIStyle.h"
#include "Styling/CoreStyle.h"

/**
 * SSEEArmorDragDropOp
 *
 * Carries an armor item's FName ItemID and its display text from the
 * inventory list to an SSEEArmorSlotWidget drop target.  The decorator
 * widget is a small frosted label so the player can see what they're dragging.
 */
class SSEEArmorDragDropOp : public FDragDropOperation
{
public:
	DRAG_DROP_OPERATOR_TYPE(SSEEArmorDragDropOp, FDragDropOperation)

	/** The inventory item being dragged. */
	FName ItemID;

	/** Human-readable label for the decorator. */
	FText DisplayName;

	static TSharedRef<SSEEArmorDragDropOp> New(FName InItemID, const FText& InDisplayName)
	{
		TSharedRef<SSEEArmorDragDropOp> Op = MakeShared<SSEEArmorDragDropOp>();
		Op->ItemID      = InItemID;
		Op->DisplayName = InDisplayName;
		Op->Construct();
		return Op;
	}

	virtual TSharedPtr<SWidget> GetDefaultDecorator() const override
	{
		return SNew(SBorder)
			.BorderImage(SEEUIStyle::WhiteBrush())
			.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::Gunmetal, 0.92f))
			.Padding(FMargin(10.0f, 6.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 6, 0)
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "DragArmorBadge", "A"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::AccentSteel))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(DisplayName)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(SEEUIStyle::BoneText))
				]
			];
	}
};
