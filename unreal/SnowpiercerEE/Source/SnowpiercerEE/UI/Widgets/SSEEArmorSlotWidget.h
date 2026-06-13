// SSEEArmorSlotWidget.h - Single paper-doll slot cell (Head / Torso / Shield).
//
// Displays the equipped piece name + DR%, or "EMPTY".
// Accepts SSEEArmorDragDropOp drops; rejects non-armor drags.
// Right-click OR a small "UNEQUIP" button returns the piece to inventory.
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/Economy/ArmorComponent.h"

class USEEInventoryComponent;

DECLARE_DELEGATE_TwoParams(FOnArmorSlotDrop,   EArmorSlot /*Slot*/, FName /*ItemID*/);
DECLARE_DELEGATE_OneParam (FOnArmorSlotUnequip, EArmorSlot /*Slot*/);

/**
 * SSEEArmorSlotWidget
 *
 * One bordered cell in the paper-doll column.  Owned by SSEEInventoryScreen.
 * The parent passes an ArmorComponent pointer at construction; the cell
 * re-reads it on every paint tick for live updates without polling.
 *
 * Visual states:
 *  - Empty  : slot label in TextFaint, "EMPTY" in TextDisabled, RivetLine border
 *  - Occupied: piece name in BoneText, DR% in EngineAmber, solid Gunmetal border
 *  - DragOver: border flashes FrostBlue while a valid drag hovers
 */
class SSEEArmorSlotWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEEArmorSlotWidget)
		: _Slot(EArmorSlot::Torso)
		, _ArmorComponent(nullptr)
	{}
		/** Which slot this cell represents. */
		SLATE_ARGUMENT(EArmorSlot, Slot)
		/** Read-only pointer to the pawn's armor component. */
		SLATE_ARGUMENT(UArmorComponent*, ArmorComponent)
		/** Called when a valid SSEEArmorDragDropOp is dropped on this cell. */
		SLATE_EVENT(FOnArmorSlotDrop,    OnDrop)
		/** Called when the user clicks Unequip. */
		SLATE_EVENT(FOnArmorSlotUnequip, OnUnequip)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// Drag-and-drop overrides
	virtual FReply OnDragOver (const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop     (const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void   OnDragLeave(const FDragDropEvent& DragDropEvent) override;

private:
	FText  GetSlotLabel()    const;
	FText  GetPieceName()    const;
	FText  GetDRText()       const;
	FLinearColor GetBorderColor() const;
	FLinearColor GetPieceNameColor() const;
	EVisibility  GetOccupiedVisibility() const;
	EVisibility  GetEmptyVisibility()    const;
	EVisibility  GetUnequipVisibility()  const;

	FReply HandleUnequipClicked();

	EArmorSlot                  SlotType;
	TWeakObjectPtr<UArmorComponent> ArmorComp;
	FOnArmorSlotDrop            OnDropDelegate;
	FOnArmorSlotUnequip         OnUnequipDelegate;

	bool bDragOver = false;
};
