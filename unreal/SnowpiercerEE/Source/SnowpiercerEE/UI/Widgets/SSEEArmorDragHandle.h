// SSEEArmorDragHandle.h - a transparent wrapper that makes its content draggable.
// UE 5.7's SBorder/SButton don't expose OnMouseButtonDown/OnDragDetected as
// declarative FArguments, so drag detection must be done by overriding the
// widget virtuals. This thin SCompoundWidget does exactly that and emits an
// SSEEArmorDragDropOp carrying the armor item's ID.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Input/Events.h"
#include "Framework/Application/SlateApplication.h"
#include "SSEEArmorDragDropOp.h"

DECLARE_DELEGATE(FOnArmorDragHandlePressed);

class SSEEArmorDragHandle : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEArmorDragHandle) {}
		SLATE_ARGUMENT(FName, ItemID)
		SLATE_ARGUMENT(FText, DisplayName)
		SLATE_EVENT(FOnArmorDragHandlePressed, OnPressed)
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		ItemID = InArgs._ItemID;
		DisplayName = InArgs._DisplayName;
		OnPressed = InArgs._OnPressed;
		ChildSlot
		[
			InArgs._Content.Widget
		];
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			OnPressed.ExecuteIfBound();
			return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
		}
		return FReply::Unhandled();
	}

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (ItemID.IsNone())
		{
			return FReply::Unhandled();
		}
		return FReply::Handled().BeginDragDrop(SSEEArmorDragDropOp::New(ItemID, DisplayName));
	}

private:
	FName ItemID;
	FText DisplayName;
	FOnArmorDragHandlePressed OnPressed;
};
