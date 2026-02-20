// SSEECodexPanel.cpp - Codex/journal implementation
#include "SSEECodexPanel.h"

#include "SnowpiercerEE/Exploration/CollectibleJournalSubsystem.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"

void SSEECodexPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.88f))
		.Padding(40.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				MakeHeader()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				MakeCategoryTabs()
			]

			// Collectible list (left) + progress sidebar (right)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.65f)
				.Padding(0, 0, 8, 0)
				[
					MakeCollectibleList()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.35f)
				[
					MakeProgressSidebar()
				]
			]
		]
	];
}

void SSEECodexPanel::SetJournalSubsystem(UCollectibleJournalSubsystem* InSubsystem)
{
	JournalSubsystem = InSubsystem;
}

TSharedRef<SWidget> SSEECodexPanel::MakeHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("HUD", "Codex", "CODEX"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!JournalSubsystem.IsValid()) return FText::GetEmpty();
				int32 Unviewed = JournalSubsystem->GetUnviewedCount();
				if (Unviewed > 0)
				{
					return FText::Format(NSLOCTEXT("HUD", "NewEntries", "{0} new"),
						FText::AsNumber(Unviewed));
				}
				return FText::GetEmpty();
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.8f, 0.2f)))
		];
}

TSharedRef<SWidget> SSEECodexPanel::MakeCategoryTabs()
{
	auto MakeTab = [this](const FText& Label, uint8 CategoryValue) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, CategoryValue]()
			{
				return (ActiveCategory == CategoryValue)
					? FLinearColor(0.3f, 0.25f, 0.15f)
					: FLinearColor(0.1f, 0.1f, 0.12f);
			})
			.OnClicked_Lambda([this, CategoryValue]()
			{
				ActiveCategory = CategoryValue;
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
				.Margin(FMargin(8, 4))
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdAll", "All"), 255) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdLogs", "Train Logs"), static_cast<uint8>(ECollectibleType::TrainLog)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdAudio", "Audio"), static_cast<uint8>(ECollectibleType::AudioRecording)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdArtifact", "Artifacts"), static_cast<uint8>(ECollectibleType::Artifact)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdBP", "Blueprints"), static_cast<uint8>(ECollectibleType::Blueprint)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdIntel", "Intel"), static_cast<uint8>(ECollectibleType::FactionIntel)) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "CdManifest", "Manifest"), static_cast<uint8>(ECollectibleType::ManifestPage)) ];
}

TSharedRef<SWidget> SSEECodexPanel::MakeCollectibleList()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(8.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!JournalSubsystem.IsValid())
					{
						return NSLOCTEXT("HUD", "NoCodex", "No journal data available");
					}

					FString Result;

					if (ActiveCategory == 255)
					{
						// Show all types with completion percentages
						for (uint8 i = 0; i < 6; ++i)
						{
							ECollectibleType Type = static_cast<ECollectibleType>(i);
							float Pct = JournalSubsystem->GetCompletionPercentage(Type) * 100.0f;
							TArray<FCollectibleState> Items = JournalSubsystem->GetCollectedByType(Type);

							FText TypeName = GetTypeName(Type);
							Result += FString::Printf(TEXT("%s  (%.0f%%)\n"),
								*TypeName.ToString(), Pct);

							for (const FCollectibleState& State : Items)
							{
								FString NewTag = State.bViewed ? TEXT("") : TEXT(" *NEW*");
								Result += FString::Printf(TEXT("  %s%s\n"),
									*State.CollectibleID.ToString(), *NewTag);
							}
							Result += TEXT("\n");
						}
					}
					else
					{
						ECollectibleType Type = static_cast<ECollectibleType>(ActiveCategory);
						TArray<FCollectibleState> Items = JournalSubsystem->GetCollectedByType(Type);

						if (Items.Num() == 0)
						{
							Result = TEXT("No entries collected yet");
						}
						else
						{
							for (const FCollectibleState& State : Items)
							{
								FString NewTag = State.bViewed ? TEXT("") : TEXT(" *NEW*");
								Result += FString::Printf(TEXT("%s%s\n"),
									*State.CollectibleID.ToString(), *NewTag);
							}
						}
					}

					return FText::FromString(Result);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SSEECodexPanel::MakeProgressSidebar()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "Progress", "PROGRESS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			// Overall completion
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!JournalSubsystem.IsValid()) return FText::GetEmpty();
					float Pct = JournalSubsystem->GetOverallCompletion() * 100.0f;
					FText Title = JournalSubsystem->GetCurrentExplorationTitle();
					return FText::Format(
						NSLOCTEXT("HUD", "Overall", "Overall: {0}%\nTitle: {1}"),
						FText::FromString(FString::Printf(TEXT("%.1f"), Pct)),
						Title);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 16)
			[
				SNew(SBox)
				.HeightOverride(8.0f)
				[
					SNew(SProgressBar)
					.Percent_Lambda([this]()
					{
						if (!JournalSubsystem.IsValid()) return 0.0f;
						return FMath::Clamp(JournalSubsystem->GetOverallCompletion(), 0.0f, 1.0f);
					})
					.FillColorAndOpacity(FLinearColor(0.9f, 0.7f, 0.2f))
					.BackgroundImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.FillImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
					.BorderPadding(FVector2D(0, 0))
				]
			]

			// Web of Power
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 0, 0, 8)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "WebOfPower", "WEB OF POWER"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						if (!JournalSubsystem.IsValid()) return FText::GetEmpty();

						TArray<FFactionIntelProgress> AllProgress = JournalSubsystem->GetAllFactionProgress();
						if (AllProgress.Num() == 0)
						{
							return NSLOCTEXT("HUD", "NoIntel", "No faction intel collected");
						}

						FString Result;
						for (const FFactionIntelProgress& Prog : AllProgress)
						{
							FString Dialogue = Prog.bSpecialDialogueUnlocked ? TEXT(" [DIALOGUE]") : TEXT("");
							FString Weakness = Prog.bCriticalWeaknessRevealed ? TEXT(" [WEAKNESS]") : TEXT("");

							Result += FString::Printf(TEXT("%s: %d/%d%s%s\n"),
								*Prog.FactionID.ToString(),
								Prog.IntelCollected, Prog.IntelTotal,
								*Dialogue, *Weakness);
						}

						// Manifest pages
						int32 ManifestPages = JournalSubsystem->GetManifestPagesCollected();
						bool bTruthUnlocked = JournalSubsystem->IsManifestTruthUnlocked();
						Result += FString::Printf(TEXT("\nManifest Pages: %d/50%s"),
							ManifestPages,
							bTruthUnlocked ? TEXT(" [TRUTH QUEST]") : TEXT(""));

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					.AutoWrapText(true)
				]
			]
		];
}

FText SSEECodexPanel::GetTypeName(ECollectibleType Type) const
{
	switch (Type)
	{
	case ECollectibleType::TrainLog:       return NSLOCTEXT("HUD", "TTrainLog", "Train Logs");
	case ECollectibleType::AudioRecording: return NSLOCTEXT("HUD", "TAudio", "Audio Recordings");
	case ECollectibleType::Artifact:       return NSLOCTEXT("HUD", "TArtifact", "Artifacts");
	case ECollectibleType::Blueprint:      return NSLOCTEXT("HUD", "TBlueprint", "Blueprints");
	case ECollectibleType::FactionIntel:   return NSLOCTEXT("HUD", "TIntel", "Faction Intel");
	case ECollectibleType::ManifestPage:   return NSLOCTEXT("HUD", "TManifest", "Manifest Pages");
	}
	return FText::GetEmpty();
}
