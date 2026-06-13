// SSEEArmorSlotWidget.cpp
#include "SSEEArmorSlotWidget.h"
#include "SSEEArmorDragDropOp.h"
#include "SSEEUIStyle.h"
#include "SSEEMenuButton.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Input/DragAndDrop.h"
#include "Styling/CoreStyle.h"

void SSEEArmorSlotWidget::Construct(const FArguments& InArgs)
{
	SlotType          = InArgs._Slot;
	ArmorComp         = InArgs._ArmorComponent;
	OnDropDelegate    = InArgs._OnDrop;
	OnUnequipDelegate = InArgs._OnUnequip;

	ChildSlot
	[
		SNew(SBox)
		.WidthOverride(220.0f)
		.HeightOverride(62.0f)
		[
			// Outer border – color toggles on drag-over / occupied state
			SNew(SBorder)
			.BorderImage(SEEUIStyle::WhiteBrush())
			.BorderBackgroundColor_Lambda([this]() -> FLinearColor { return GetBorderColor(); })
			.Padding(FMargin(1.0f))
			[
				SNew(SBorder)
				.BorderImage(SEEUIStyle::WhiteBrush())
				.BorderBackgroundColor(SEEUIStyle::Gunmetal)
				.Padding(FMargin(10.0f, 6.0f))
				[
					SNew(SVerticalBox)

					// Slot label row (HEAD / TORSO / SHIELD) + unequip affordance
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetSlotLabel(); })
							.Font(SEEUIStyle::CaptionFont(9))
							.ColorAndOpacity(FSlateColor(SEEUIStyle::TextFaint))
						]

						// UNEQUIP link – only visible when a piece is equipped
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.Visibility_Lambda([this]() { return GetUnequipVisibility(); })
							[
								SNew(SButton)
								.ButtonStyle(&SEEUIStyle::GetHollowButtonStyle())
								.OnClicked_Lambda([this]()
								{
									return HandleUnequipClicked();
								})
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("HUD", "ArmorUnequip", "UNEQUIP"))
									.Font(SEEUIStyle::CaptionFont(8))
									.ColorAndOpacity(FSlateColor(SEEUIStyle::BloodRed))
								]
							]
						]
					]

					// Piece name (occupied state)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 2, 0, 0)
					[
						SNew(SHorizontalBox)
						.Visibility_Lambda([this]() { return GetOccupiedVisibility(); })

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetPieceName(); })
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
							.ColorAndOpacity_Lambda([this]() -> FSlateColor { return GetPieceNameColor(); })
							.AutoWrapText(false)
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(6, 0, 0, 0)
						[
							SNew(STextBlock)
							.Text_Lambda([this]() { return GetDRText(); })
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
							.ColorAndOpacity(FSlateColor(SEEUIStyle::EngineAmber))
						]
					]

					// EMPTY label (unoccupied state)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0, 2, 0, 0)
					[
						SNew(STextBlock)
						.Text(NSLOCTEXT("HUD", "ArmorSlotEmpty", "EMPTY"))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDisabled))
						.Visibility_Lambda([this]() { return GetEmptyVisibility(); })
					]
				]
			]
		]
	];
}

// --- Drag-and-drop ---

FReply SSEEArmorSlotWidget::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	if (DragDropEvent.GetOperationAs<SSEEArmorDragDropOp>().IsValid())
	{
		bDragOver = true;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply SSEEArmorSlotWidget::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	bDragOver = false;

	TSharedPtr<SSEEArmorDragDropOp> Op = DragDropEvent.GetOperationAs<SSEEArmorDragDropOp>();
	if (Op.IsValid() && !Op->ItemID.IsNone())
	{
		OnDropDelegate.ExecuteIfBound(SlotType, Op->ItemID);
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void SSEEArmorSlotWidget::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	bDragOver = false;
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

// --- Private helpers ---

FText SSEEArmorSlotWidget::GetSlotLabel() const
{
	switch (SlotType)
	{
	case EArmorSlot::Head:   return NSLOCTEXT("HUD", "SlotHead",   "HEAD");
	case EArmorSlot::Torso:  return NSLOCTEXT("HUD", "SlotTorso",  "TORSO");
	case EArmorSlot::Shield: return NSLOCTEXT("HUD", "SlotShield", "SHIELD");
	default:                 return NSLOCTEXT("HUD", "SlotUnknown","—");
	}
}

FText SSEEArmorSlotWidget::GetPieceName() const
{
	if (!ArmorComp.IsValid()) return FText::GetEmpty();

	FEquippedArmor Piece;
	if (!ArmorComp->GetArmorInSlot(SlotType, Piece)) return FText::GetEmpty();
	return FText::FromName(Piece.ArmorItemID);
}

FText SSEEArmorSlotWidget::GetDRText() const
{
	if (!ArmorComp.IsValid()) return FText::GetEmpty();

	FEquippedArmor Piece;
	if (!ArmorComp->GetArmorInSlot(SlotType, Piece)) return FText::GetEmpty();

	if (SlotType == EArmorSlot::Shield)
	{
		return FText::FromString(FString::Printf(TEXT("BLK %.0f%%"), Piece.BlockBonus));
	}
	return FText::FromString(FString::Printf(TEXT("DR %.0f%%"), Piece.DamageReduction));
}

FLinearColor SSEEArmorSlotWidget::GetBorderColor() const
{
	if (bDragOver) return SEEUIStyle::FrostBlue;
	if (ArmorComp.IsValid() && ArmorComp->HasArmorInSlot(SlotType))
		return SEEUIStyle::EngineAmber;
	return SEEUIStyle::RivetLine;
}

FLinearColor SSEEArmorSlotWidget::GetPieceNameColor() const
{
	if (!ArmorComp.IsValid()) return SEEUIStyle::TextDim;
	FEquippedArmor Piece;
	if (ArmorComp->GetArmorInSlot(SlotType, Piece) && Piece.IsBroken())
		return SEEUIStyle::BloodRed;
	return SEEUIStyle::BoneText;
}

EVisibility SSEEArmorSlotWidget::GetOccupiedVisibility() const
{
	return (ArmorComp.IsValid() && ArmorComp->HasArmorInSlot(SlotType))
		? EVisibility::SelfHitTestInvisible
		: EVisibility::Collapsed;
}

EVisibility SSEEArmorSlotWidget::GetEmptyVisibility() const
{
	return (!ArmorComp.IsValid() || !ArmorComp->HasArmorInSlot(SlotType))
		? EVisibility::SelfHitTestInvisible
		: EVisibility::Collapsed;
}

EVisibility SSEEArmorSlotWidget::GetUnequipVisibility() const
{
	return (ArmorComp.IsValid() && ArmorComp->HasArmorInSlot(SlotType))
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

FReply SSEEArmorSlotWidget::HandleUnequipClicked()
{
	OnUnequipDelegate.ExecuteIfBound(SlotType);
	return FReply::Handled();
}
