// SSEESettingsPanel.cpp - Settings overlay implementation
#include "SSEESettingsPanel.h"
#include "SSEEUIStyle.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"

void SSEESettingsPanel::Construct(const FArguments& InArgs)
{
	OwnerPC = InArgs._PlayerController;
	OnClose = InArgs._OnClose;
	SensitivityValue = ReadCurrentSensitivity();

	ChildSlot
	[
		// Full-screen backdrop so clicks don't fall through to the menu behind
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(SEEUIStyle::ScreenBackdrop)
		.Padding(0.0f)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor(SEEUIStyle::PanelDark)
				.Padding(40.0f)
				[
					SNew(SBox)
					.WidthOverride(460.0f)
					[
						SNew(SVerticalBox)

						// Title
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.Padding(0, 0, 0, 24)
						[
							SNew(STextBlock)
							.Text(NSLOCTEXT("HUD", "SettingsTitle", "SETTINGS"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
							.ColorAndOpacity(FSlateColor(SEEUIStyle::TextHeader))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 0, 0, 20)
						[
							MakeSensitivityRow()
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0, 0, 0, 28)
						[
							MakeWindowModeRow()
						]

						// Back button
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(220.0f)
							.HeightOverride(42.0f)
							[
								SNew(SButton)
								.ButtonStyle(&SEEUIStyle::GetMenuButtonStyle())
								.OnClicked_Lambda([this]()
								{
									OnClose.ExecuteIfBound();
									return FReply::Handled();
								})
								.HAlign(HAlign_Center)
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.Text(NSLOCTEXT("HUD", "SettingsBack", "BACK"))
									.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
									.ColorAndOpacity(FSlateColor(SEEUIStyle::TextPrimary))
								]
							]
						]
					]
				]
			]
		]
	];
}

FReply SSEESettingsPanel::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnClose.ExecuteIfBound();
		return FReply::Handled();
	}
	return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
}

TSharedRef<SWidget> SSEESettingsPanel::MakeSensitivityRow()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 6)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "MouseSens", "MOUSE SENSITIVITY"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(FString::Printf(TEXT("%.3f"), SensitivityValue));
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::AccentBrass))
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SSlider)
			.MinValue(0.01f)
			.MaxValue(0.30f)
			.Value_Lambda([this]() { return SensitivityValue; })
			.OnValueChanged_Lambda([this](float NewValue) { ApplySensitivity(NewValue); })
			.SliderBarColor(SEEUIStyle::PanelMid)
			.SliderHandleColor(SEEUIStyle::AccentBrass)
		];
}

TSharedRef<SWidget> SSEESettingsPanel::MakeWindowModeRow()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0, 0, 0, 6)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "WindowMode", "WINDOW MODE"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::TextDim))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[
				MakeWindowModeButton(NSLOCTEXT("HUD", "WMFullscreen", "FULLSCREEN"), EWindowMode::Fullscreen)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0, 0, 4, 0)
			[
				MakeWindowModeButton(NSLOCTEXT("HUD", "WMBorderless", "BORDERLESS"), EWindowMode::WindowedFullscreen)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f)
			[
				MakeWindowModeButton(NSLOCTEXT("HUD", "WMWindowed", "WINDOWED"), EWindowMode::Windowed)
			]
		];
}

TSharedRef<SWidget> SSEESettingsPanel::MakeWindowModeButton(const FText& Label, EWindowMode::Type Mode)
{
	return SNew(SBox)
		.HeightOverride(34.0f)
		[
			SNew(SButton)
			.ButtonStyle(&SEEUIStyle::GetRowButtonStyle())
			.ButtonColorAndOpacity_Lambda([Mode]()
			{
				UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
				const bool bActive = Settings && Settings->GetFullscreenMode() == Mode;
				return bActive ? SEEUIStyle::TabActive : SEEUIStyle::TabInactive;
			})
			.OnClicked_Lambda([Mode]()
			{
				if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
				{
					Settings->SetFullscreenMode(Mode);
					Settings->ApplySettings(false);
				}
				return FReply::Handled();
			})
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::TextPrimary))
			]
		];
}

void SSEESettingsPanel::ApplySensitivity(float NewValue)
{
	SensitivityValue = NewValue;

	if (OwnerPC.IsValid() && OwnerPC->PlayerInput)
	{
		OwnerPC->PlayerInput->SetMouseSensitivity(NewValue);
	}
}

float SSEESettingsPanel::ReadCurrentSensitivity() const
{
	if (OwnerPC.IsValid() && OwnerPC->PlayerInput)
	{
		FInputAxisProperties AxisProps;
		if (OwnerPC->PlayerInput->GetAxisProperties(EKeys::MouseX, AxisProps))
		{
			return AxisProps.Sensitivity;
		}
	}
	return 0.07f;
}
