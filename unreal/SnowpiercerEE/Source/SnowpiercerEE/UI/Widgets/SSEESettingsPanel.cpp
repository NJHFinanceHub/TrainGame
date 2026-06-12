// SSEESettingsPanel.cpp - Settings overlay implementation ("Eternal Engine" chrome)
#include "SSEESettingsPanel.h"
#include "SSEEUIStyle.h"
#include "SSEEPanelFrame.h"
#include "SSEEMenuButton.h"

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
		.BorderImage(SEEUIStyle::WhiteBrush())
		.BorderBackgroundColor(SEEUIStyle::Dim(SEEUIStyle::SteelBlack, 0.88f))
		.Padding(0.0f)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(SSEEPanelFrame)
				.Title(NSLOCTEXT("HUD", "SettingsTitle", "SETTINGS - CALIBRATION"))
				.MinWidth(480.0f)
				.ContentPadding(FMargin(28.0f, 24.0f, 28.0f, 28.0f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 22.0f)
					[
						MakeSensitivityRow()
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 0.0f, 30.0f)
					[
						MakeWindowModeRow()
					]

					// Back button
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SSEEMenuButton)
						.Text(NSLOCTEXT("HUD", "SettingsBack", "BACK"))
						.Width(240.0f).Height(44.0f)
						.OnClicked(FSimpleDelegate::CreateLambda([this]() { OnClose.ExecuteIfBound(); }))
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
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "MouseSens", "MOUSE SENSITIVITY"))
				.Font(SEEUIStyle::CaptionFont(11))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.7f)))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					return FText::FromString(FString::Printf(TEXT("%.3f"), SensitivityValue));
				})
				.Font(SEEUIStyle::BodyFont(12))
				.ColorAndOpacity(FSlateColor(SEEUIStyle::EngineAmber))
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
			.SliderBarColor(SEEUIStyle::RivetLine)
			.SliderHandleColor(SEEUIStyle::EngineAmber)
		];
}

TSharedRef<SWidget> SSEESettingsPanel::MakeWindowModeRow()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "WindowMode", "WINDOW MODE"))
			.Font(SEEUIStyle::CaptionFont(11))
			.ColorAndOpacity(FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.7f)))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				MakeWindowModeButton(NSLOCTEXT("HUD", "WMFullscreen", "FULLSCREEN"), EWindowMode::Fullscreen)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 6.0f, 0.0f)
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
	auto IsActiveMode = [Mode]()
	{
		UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
		return Settings && Settings->GetFullscreenMode() == Mode;
	};

	return SNew(SBox)
		.HeightOverride(36.0f)
		[
			SNew(SButton)
			.ButtonStyle(&SEEUIStyle::GetRowButtonStyle())
			.ButtonColorAndOpacity_Lambda([IsActiveMode]()
			{
				// Active: amber-lit panel; inactive: gunmetal
				return IsActiveMode()
					? FLinearColor::LerpUsingHSV(SEEUIStyle::Gunmetal, SEEUIStyle::EngineAmber, 0.35f)
					: SEEUIStyle::Gunmetal;
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
				.Font(SEEUIStyle::CaptionFont(10))
				.ColorAndOpacity_Lambda([IsActiveMode]()
				{
					return IsActiveMode()
						? FSlateColor(FLinearColor::White)
						: FSlateColor(SEEUIStyle::Dim(SEEUIStyle::BoneText, 0.8f));
				})
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
