// SSEEQuestLog.cpp - Quest log implementation
#include "SSEEQuestLog.h"

#include "SnowpiercerEE/SEEQuestManager.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

void SSEEQuestLog::Construct(const FArguments& InArgs)
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
				MakeFilterTabs()
			]

			// Quest list (left) + detail (right)
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				.Padding(0, 0, 8, 0)
				[
					MakeQuestList()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					MakeDetailPanel()
				]
			]
		]
	];
}

void SSEEQuestLog::SetQuestManager(USEEQuestManager* InManager)
{
	QuestManager = InManager;
}

TSharedRef<SWidget> SSEEQuestLog::MakeHeader()
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("HUD", "QuestLog", "QUEST LOG"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)));
}

TSharedRef<SWidget> SSEEQuestLog::MakeFilterTabs()
{
	auto MakeTab = [this](const FText& Label, int32 FilterValue) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, FilterValue]()
			{
				return (ActiveFilter == FilterValue)
					? FLinearColor(0.3f, 0.25f, 0.15f)
					: FLinearColor(0.1f, 0.1f, 0.12f);
			})
			.OnClicked_Lambda([this, FilterValue]()
			{
				ActiveFilter = FilterValue;
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
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QActive", "Active"), 0) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QComplete", "Completed"), 1) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QFailed", "Failed"), 2) ];
}

TSharedRef<SWidget> SSEEQuestLog::MakeQuestList()
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
					if (!QuestManager.IsValid())
					{
						return NSLOCTEXT("HUD", "NoQuests", "No quest data available");
					}

					TArray<FSEEQuest> Quests;
					switch (ActiveFilter)
					{
					case 0: // Active
						Quests = QuestManager->GetActiveQuests();
						break;
					case 1: // Completed
						Quests = QuestManager->GetCompletedQuests();
						break;
					case 2: // Failed
					{
						// No dedicated GetFailedQuests — would need to iterate
						// For now show a message
						return NSLOCTEXT("HUD", "NoFailed", "No failed quests");
					}
					}

					if (Quests.Num() == 0)
					{
						static const FText EmptyMessages[] = {
							NSLOCTEXT("HUD", "NoActive", "No active quests"),
							NSLOCTEXT("HUD", "NoCompleted", "No completed quests"),
							NSLOCTEXT("HUD", "NoFailed2", "No failed quests")
						};
						return EmptyMessages[FMath::Clamp(ActiveFilter, 0, 2)];
					}

					FName TrackedID = QuestManager->GetTrackedQuestID();
					FString Result;

					for (const FSEEQuest& Quest : Quests)
					{
						bool bTracked = (Quest.QuestID == TrackedID);
						FString TypeTag = Quest.bIsMainQuest ? TEXT("[MAIN]") : TEXT("[SIDE]");
						FString TrackTag = bTracked ? TEXT(" *TRACKED*") : TEXT("");

						Result += FString::Printf(TEXT("%s %s%s\n"),
							*TypeTag, *Quest.QuestName.ToString(), *TrackTag);

						// Show objectives for active quests
						if (ActiveFilter == 0)
						{
							for (const FSEEQuestObjective& Obj : Quest.Objectives)
							{
								FString Check = Obj.bCompleted ? TEXT("[x]") : TEXT("[ ]");
								FString Optional = Obj.bOptional ? TEXT(" (optional)") : TEXT("");
								Result += FString::Printf(TEXT("  %s %s %d/%d%s\n"),
									*Check, *Obj.Description.ToString(),
									Obj.CurrentCount, Obj.RequiredCount, *Optional);
							}
						}

						Result += TEXT("\n");
					}

					return FText::FromString(Result);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> SSEEQuestLog::MakeDetailPanel()
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
				.Text(NSLOCTEXT("HUD", "QuestDetail", "QUEST DETAILS"))
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
						if (!QuestManager.IsValid())
						{
							return FText::GetEmpty();
						}

						FName TrackedID = QuestManager->GetTrackedQuestID();
						if (TrackedID.IsNone())
						{
							return NSLOCTEXT("HUD", "SelectQuest", "Track a quest to view details");
						}

						FSEEQuest Quest = QuestManager->GetQuest(TrackedID);
						if (Quest.QuestID.IsNone())
						{
							return NSLOCTEXT("HUD", "QuestNotFound", "Quest not found");
						}

						FString Result;
						Result += Quest.QuestName.ToString() + TEXT("\n");
						Result += TEXT("---\n");
						Result += Quest.Description.ToString() + TEXT("\n\n");

						Result += TEXT("Rewards:\n");
						Result += FString::Printf(TEXT("  XP: %d\n"), Quest.XPReward);

						if (Quest.ItemRewards.Num() > 0)
						{
							Result += FString::Printf(TEXT("  Items: %d\n"), Quest.ItemRewards.Num());
						}

						for (const auto& FacRep : Quest.FactionRepRewards)
						{
							Result += FString::Printf(TEXT("  %s: %+d rep\n"),
								*FacRep.Key.ToString(), FacRep.Value);
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 0.8f)))
					.AutoWrapText(true)
				]
			]
		];
}
