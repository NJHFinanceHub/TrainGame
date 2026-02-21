// SSEEQuestLog.cpp - Quest log implementation
#include "SSEEQuestLog.h"

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
					MakeQuestDetail()
				]
			]
		]
	];
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
	auto MakeTab = [this](const FText& Label, ESEEQuestState FilterState) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, FilterState]()
			{
				return ActiveFilter == FilterState
					? FLinearColor(0.3f, 0.25f, 0.15f)
					: FLinearColor(0.1f, 0.1f, 0.12f);
			})
			.OnClicked_Lambda([this, FilterState]()
			{
				ActiveFilter = FilterState;
				SelectedQuestID = NAME_None;
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
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QActive", "Active"), ESEEQuestState::Active) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QCompleted", "Completed"), ESEEQuestState::Completed) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(NSLOCTEXT("HUD", "QFailed", "Failed"), ESEEQuestState::Failed) ];
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
					TArray<FSEEQuest> Quests = GetFilteredQuests();
					if (Quests.Num() == 0)
					{
						return NSLOCTEXT("HUD", "NoQuests", "No quests");
					}

					FString Result;
					for (const FSEEQuest& Quest : Quests)
					{
						FString TypeTag = Quest.bIsMainQuest ? TEXT("[MAIN] ") : TEXT("[SIDE] ");
						FString TrackTag;
						if (QuestManager.IsValid() && QuestManager->GetTrackedQuestID() == Quest.QuestID)
						{
							TrackTag = TEXT(" << TRACKED");
						}

						Result += FString::Printf(TEXT("%s%s%s\n"),
							*TypeTag,
							*Quest.QuestName.ToString(),
							*TrackTag);

						// Show objective summary for active quests
						if (Quest.State == ESEEQuestState::Active)
						{
							for (const FSEEQuestObjective& Obj : Quest.Objectives)
							{
								FString Check = Obj.bCompleted ? TEXT("[x]") : TEXT("[ ]");
								FString Optional = Obj.bOptional ? TEXT(" (optional)") : TEXT("");
								if (Obj.RequiredCount > 1)
								{
									Result += FString::Printf(TEXT("  %s %s (%d/%d)%s\n"),
										*Check,
										*Obj.Description.ToString(),
										Obj.CurrentCount, Obj.RequiredCount,
										*Optional);
								}
								else
								{
									Result += FString::Printf(TEXT("  %s %s%s\n"),
										*Check,
										*Obj.Description.ToString(),
										*Optional);
								}
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

TSharedRef<SWidget> SSEEQuestLog::MakeQuestDetail()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("HUD", "QuestDetail", "QUEST DETAILS"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.65f, 0.5f)))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 8, 0, 0)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!QuestManager.IsValid() || SelectedQuestID.IsNone())
					{
						return NSLOCTEXT("HUD", "SelectQuest", "Select a quest to view details");
					}

					FSEEQuest Quest = QuestManager->GetQuest(SelectedQuestID);
					FString Detail;
					Detail += Quest.QuestName.ToString() + TEXT("\n\n");
					Detail += Quest.Description.ToString() + TEXT("\n\n");

					// Rewards
					if (Quest.XPReward > 0)
					{
						Detail += FString::Printf(TEXT("XP Reward: %d\n"), Quest.XPReward);
					}
					if (Quest.ItemRewards.Num() > 0)
					{
						Detail += FString::Printf(TEXT("Item Rewards: %d items\n"), Quest.ItemRewards.Num());
					}

					return FText::FromString(Detail);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
				.AutoWrapText(true)
			]
		];
}

TArray<FSEEQuest> SSEEQuestLog::GetFilteredQuests() const
{
	if (!QuestManager.IsValid())
	{
		return {};
	}

	switch (ActiveFilter)
	{
	case ESEEQuestState::Active:
		return QuestManager->GetActiveQuests();
	case ESEEQuestState::Completed:
		return QuestManager->GetCompletedQuests();
	case ESEEQuestState::Failed:
	{
		// QuestManager doesn't have GetFailedQuests, but we can filter
		// For now return empty - would need to add GetFailedQuests() to manager
		return {};
	}
	default:
		return QuestManager->GetActiveQuests();
	}
}

FText SSEEQuestLog::GetQuestStateLabel(ESEEQuestState State) const
{
	switch (State)
	{
	case ESEEQuestState::Active:    return NSLOCTEXT("HUD", "StateActive", "ACTIVE");
	case ESEEQuestState::Completed: return NSLOCTEXT("HUD", "StateComplete", "COMPLETED");
	case ESEEQuestState::Failed:    return NSLOCTEXT("HUD", "StateFailed", "FAILED");
	default: return FText::GetEmpty();
	}
}

FLinearColor SSEEQuestLog::GetQuestStateColor(ESEEQuestState State) const
{
	switch (State)
	{
	case ESEEQuestState::Active:    return FLinearColor(0.9f, 0.8f, 0.3f);
	case ESEEQuestState::Completed: return FLinearColor(0.3f, 0.8f, 0.3f);
	case ESEEQuestState::Failed:    return FLinearColor(0.9f, 0.3f, 0.2f);
	default: return FLinearColor::White;
	}
}
