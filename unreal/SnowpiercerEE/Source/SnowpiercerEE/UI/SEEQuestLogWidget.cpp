#include "SEEQuestLogWidget.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SProgressBar.h"

// ============================================================================
// SSEEQuestLogWidget
// ============================================================================

void SSEEQuestLogWidget::Construct(const FArguments& InArgs)
{
	QuestManager = InArgs._QuestManager;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.02f, 0.04f, 0.95f))
		.Padding(20.0f)
		[
			SNew(SBox)
			.WidthOverride(PanelWidth)
			.HeightOverride(PanelHeight)
			[
				SNew(SVerticalBox)

				// Header
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 12)
				[
					MakeHeader()
				]

				// Category tabs
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0, 0, 0, 8)
				[
					MakeCategoryTabs()
				]

				// Quest list + detail split
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.4f)
					[
						MakeQuestList()
					]

					+ SSplitter::Slot()
					.Value(0.6f)
					[
						MakeQuestDetail()
					]
				]
			]
		]
	];

	RefreshQuests();
}

void SSEEQuestLogWidget::RefreshQuests()
{
	if (!QuestManager.IsValid())
	{
		return;
	}

	CachedQuests.Empty();

	// Combine active and completed quests
	TArray<FSEEQuest> Active = QuestManager->GetActiveQuests();
	TArray<FSEEQuest> Completed = QuestManager->GetCompletedQuests();

	CachedQuests.Append(Active);
	CachedQuests.Append(Completed);
}

void SSEEQuestLogWidget::SetCategory(const FString& Category)
{
	ActiveCategory = Category;
	RefreshQuests();
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeHeader()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("QuestLog", "Title", "QUEST LOG"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				int32 ActiveCount = 0;
				for (const FSEEQuest& Q : CachedQuests)
				{
					if (Q.State == ESEEQuestState::Active) ActiveCount++;
				}
				return FText::Format(
					NSLOCTEXT("QuestLog", "ActiveCount", "{0} Active"),
					FText::AsNumber(ActiveCount));
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
		];
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeCategoryTabs()
{
	auto MakeTab = [this](const FString& Label, const FString& Category) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.OnClicked_Lambda([this, Category]()
			{
				SetCategory(Category);
				return FReply::Handled();
			})
			[
				SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
				.BorderBackgroundColor_Lambda([this, Category]()
				{
					return ActiveCategory == Category
						? FLinearColor(0.3f, 0.25f, 0.15f, 0.9f)
						: FLinearColor(0.1f, 0.1f, 0.12f, 0.7f);
				})
				.Padding(FMargin(12, 6))
				[
					SNew(STextBlock)
					.Text(FText::FromString(Label))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
				]
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(TEXT("All"), TEXT("All")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(TEXT("Main"), TEXT("Main")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(TEXT("Side"), TEXT("Side")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(TEXT("Faction"), TEXT("Faction")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)[ MakeTab(TEXT("Companion"), TEXT("Companion")) ];
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeQuestList()
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
					FString Result;
					for (const FSEEQuest& Quest : CachedQuests)
					{
						// Category filtering
						if (ActiveCategory == TEXT("Main") && !Quest.bIsMainQuest) continue;
						if (ActiveCategory == TEXT("Side") && Quest.bIsMainQuest) continue;

						FString QuestIDStr = Quest.QuestID.ToString();
						if (ActiveCategory == TEXT("Faction") && !QuestIDStr.StartsWith(TEXT("FQ_"))) continue;
						if (ActiveCategory == TEXT("Companion") && !QuestIDStr.StartsWith(TEXT("CQ_"))) continue;

						// Quest entry
						FText StateText = GetQuestStateText(Quest.State);
						bool bIsTracked = QuestManager.IsValid() &&
							QuestManager->GetTrackedQuestID() == Quest.QuestID;

						Result += FString::Printf(TEXT("%s[%s] %s%s\n"),
							bIsTracked ? TEXT("> ") : TEXT("  "),
							*StateText.ToString(),
							*Quest.QuestName.ToString(),
							Quest.bIsMainQuest ? TEXT(" [MAIN]") : TEXT(""));

						// Show objectives for active quests
						if (Quest.State == ESEEQuestState::Active)
						{
							for (const FSEEQuestObjective& Obj : Quest.Objectives)
							{
								FString Check = Obj.bCompleted ? TEXT("[x]") : TEXT("[ ]");
								FString Optional = Obj.bOptional ? TEXT(" (Optional)") : TEXT("");
								Result += FString::Printf(TEXT("    %s %s%s"),
									*Check,
									*Obj.Description.ToString(),
									*Optional);

								if (Obj.RequiredCount > 1)
								{
									Result += FString::Printf(TEXT(" (%d/%d)"),
										Obj.CurrentCount, Obj.RequiredCount);
								}

								Result += TEXT("\n");
							}
						}

						Result += TEXT("\n");
					}

					if (Result.IsEmpty())
					{
						return NSLOCTEXT("QuestLog", "NoQuests", "No quests in this category.");
					}

					return FText::FromString(Result);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
			]
		];
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeQuestEntry(const FSEEQuest& Quest)
{
	// Individual quest entry — used when dynamic list items are needed
	return SNew(STextBlock)
		.Text(Quest.QuestName)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11));
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeObjectiveEntry(const FSEEQuestObjective& Objective)
{
	return SNew(STextBlock)
		.Text(Objective.Description)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10));
}

TSharedRef<SWidget> SSEEQuestLogWidget::MakeQuestDetail()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.06f, 0.06f, 0.08f, 0.9f))
		.Padding(16.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!QuestManager.IsValid())
				{
					return NSLOCTEXT("QuestLog", "NoManager", "Quest system unavailable.");
				}

				FName TrackedID = QuestManager->GetTrackedQuestID();
				if (TrackedID == NAME_None)
				{
					return NSLOCTEXT("QuestLog", "SelectQuest", "Select a quest to view details.\n\nTrack a quest to see its objectives on the HUD.");
				}

				FSEEQuest Quest = QuestManager->GetQuest(TrackedID);
				FString Detail;

				Detail += FString::Printf(TEXT("%s\n"), *Quest.QuestName.ToString());
				Detail += TEXT("---\n\n");
				Detail += FString::Printf(TEXT("%s\n\n"), *Quest.Description.ToString());

				Detail += TEXT("OBJECTIVES:\n");
				for (const FSEEQuestObjective& Obj : Quest.Objectives)
				{
					FString Check = Obj.bCompleted ? TEXT("[DONE]") : TEXT("[    ]");
					Detail += FString::Printf(TEXT("  %s %s"), *Check, *Obj.Description.ToString());
					if (Obj.RequiredCount > 1)
					{
						Detail += FString::Printf(TEXT(" (%d/%d)"), Obj.CurrentCount, Obj.RequiredCount);
					}
					Detail += TEXT("\n");
				}

				if (Quest.XPReward > 0)
				{
					Detail += FString::Printf(TEXT("\nREWARD: %d XP"), Quest.XPReward);
				}

				if (Quest.ItemRewards.Num() > 0)
				{
					Detail += TEXT("\nITEMS: ");
					for (int32 i = 0; i < Quest.ItemRewards.Num(); i++)
					{
						if (i > 0) Detail += TEXT(", ");
						Detail += Quest.ItemRewards[i].ToString();
					}
				}

				return FText::FromString(Detail);
			})
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
			.AutoWrapText(true)
		];
}

FText SSEEQuestLogWidget::GetQuestStateText(ESEEQuestState State) const
{
	switch (State)
	{
	case ESEEQuestState::Available:	return NSLOCTEXT("QuestLog", "Available", "NEW");
	case ESEEQuestState::Active:	return NSLOCTEXT("QuestLog", "Active", "ACTIVE");
	case ESEEQuestState::Completed:	return NSLOCTEXT("QuestLog", "Completed", "DONE");
	case ESEEQuestState::Failed:	return NSLOCTEXT("QuestLog", "Failed", "FAILED");
	default: return FText::GetEmpty();
	}
}

FSlateColor SSEEQuestLogWidget::GetQuestStateColor(ESEEQuestState State) const
{
	switch (State)
	{
	case ESEEQuestState::Available:	return FSlateColor(FLinearColor(1.0f, 0.85f, 0.3f));
	case ESEEQuestState::Active:	return FSlateColor(FLinearColor(0.3f, 0.7f, 1.0f));
	case ESEEQuestState::Completed:	return FSlateColor(FLinearColor(0.3f, 0.8f, 0.3f));
	case ESEEQuestState::Failed:	return FSlateColor(FLinearColor(0.9f, 0.2f, 0.2f));
	default: return FSlateColor(FLinearColor::Gray);
	}
}

FText SSEEQuestLogWidget::GetObjectiveTypeIcon(ESEEObjectiveType Type) const
{
	switch (Type)
	{
	case ESEEObjectiveType::GoTo:		return NSLOCTEXT("QuestLog", "GoTo", "[GO]");
	case ESEEObjectiveType::Interact:	return NSLOCTEXT("QuestLog", "Interact", "[USE]");
	case ESEEObjectiveType::Kill:		return NSLOCTEXT("QuestLog", "Kill", "[KILL]");
	case ESEEObjectiveType::Collect:	return NSLOCTEXT("QuestLog", "Collect", "[GET]");
	case ESEEObjectiveType::Escort:		return NSLOCTEXT("QuestLog", "Escort", "[ESC]");
	case ESEEObjectiveType::Dialogue:	return NSLOCTEXT("QuestLog", "Dialogue", "[TALK]");
	case ESEEObjectiveType::Custom:		return NSLOCTEXT("QuestLog", "Custom", "[OBJ]");
	default: return FText::GetEmpty();
	}
}

// ============================================================================
// SSEEQuestTrackerWidget
// ============================================================================

void SSEEQuestTrackerWidget::Construct(const FArguments& InArgs)
{
	QuestManager = InArgs._QuestManager;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f))
		.Padding(FMargin(12, 8))
		.Visibility_Lambda([this]()
		{
			return (QuestManager.IsValid() && QuestManager->GetTrackedQuestID() != NAME_None)
				? EVisibility::Visible
				: EVisibility::Collapsed;
		})
		[
			SNew(SBox)
			.WidthOverride(300.0f)
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
				{
					if (!QuestManager.IsValid()) return FText::GetEmpty();

					FName TrackedID = QuestManager->GetTrackedQuestID();
					if (TrackedID == NAME_None) return FText::GetEmpty();

					FSEEQuest Quest = QuestManager->GetQuest(TrackedID);
					FString Display;

					Display += Quest.QuestName.ToString() + TEXT("\n");

					// Show next incomplete objective
					for (const FSEEQuestObjective& Obj : Quest.Objectives)
					{
						if (!Obj.bCompleted && !Obj.bOptional)
						{
							Display += TEXT("> ") + Obj.Description.ToString();
							if (Obj.RequiredCount > 1)
							{
								Display += FString::Printf(TEXT(" (%d/%d)"),
									Obj.CurrentCount, Obj.RequiredCount);
							}
							break;
						}
					}

					return FText::FromString(Display);
				})
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)))
				.AutoWrapText(true)
			]
		]
	];
}

void SSEEQuestTrackerWidget::UpdateTrackedQuest()
{
	if (QuestManager.IsValid())
	{
		TrackedQuestID = QuestManager->GetTrackedQuestID();
	}
// SEEQuestLogWidget.cpp
#include "SEEQuestLogWidget.h"
#include "SEEQuestManager.h"
#include "Components/TextBlock.h"

void USEEQuestLogWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshQuests();
}

USEEQuestManager* USEEQuestLogWidget::GetQuestManager() const
{
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<USEEQuestManager>() : nullptr;
}

void USEEQuestLogWidget::RefreshQuests()
{
	TArray<FSEEQuest> Quests = GetFilteredQuests();

	if (FilterText)
	{
		switch (CurrentFilter)
		{
		case ESEEQuestFilter::Active: FilterText->SetText(NSLOCTEXT("Quest", "Active", "Active Quests")); break;
		case ESEEQuestFilter::Completed: FilterText->SetText(NSLOCTEXT("Quest", "Completed", "Completed")); break;
		case ESEEQuestFilter::Failed: FilterText->SetText(NSLOCTEXT("Quest", "Failed", "Failed")); break;
		case ESEEQuestFilter::All: FilterText->SetText(NSLOCTEXT("Quest", "All", "All Quests")); break;
		}
	}

	OnQuestsRefreshed(Quests);

	if (!SelectedQuestID.IsNone())
	{
		UpdateQuestDetails();
	}
}

void USEEQuestLogWidget::SetFilter(ESEEQuestFilter NewFilter)
{
	CurrentFilter = NewFilter;
	RefreshQuests();
}

TArray<FSEEQuest> USEEQuestLogWidget::GetFilteredQuests() const
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM) return {};

	TArray<FSEEQuest> Result;

	if (CurrentFilter == ESEEQuestFilter::Active || CurrentFilter == ESEEQuestFilter::All)
	{
		Result.Append(QM->GetActiveQuests());
	}
	if (CurrentFilter == ESEEQuestFilter::Completed || CurrentFilter == ESEEQuestFilter::All)
	{
		Result.Append(QM->GetCompletedQuests());
	}

	// Sort: main quests first, then by name
	Result.Sort([](const FSEEQuest& A, const FSEEQuest& B)
	{
		if (A.bIsMainQuest != B.bIsMainQuest) return A.bIsMainQuest;
		return A.QuestName.CompareTo(B.QuestName) < 0;
	});

	return Result;
}

void USEEQuestLogWidget::SelectQuest(FName QuestID)
{
	SelectedQuestID = QuestID;
	UpdateQuestDetails();
}

void USEEQuestLogWidget::UpdateQuestDetails()
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM || SelectedQuestID.IsNone()) return;

	FSEEQuest Quest = QM->GetQuest(SelectedQuestID);

	if (QuestTitleText)
	{
		FString Title = Quest.QuestName.ToString();
		if (Quest.bIsMainQuest) Title = TEXT("[MAIN] ") + Title;
		QuestTitleText->SetText(FText::FromString(Title));
	}

	if (QuestDescText)
	{
		QuestDescText->SetText(Quest.Description);
	}

	if (ObjectivesText)
	{
		FString ObjStr;
		for (const FSEEQuestObjective& Obj : Quest.Objectives)
		{
			FString Prefix = Obj.bCompleted ? TEXT("[X] ") : TEXT("[ ] ");
			if (Obj.bOptional) Prefix += TEXT("(Optional) ");

			ObjStr += Prefix + Obj.Description.ToString();

			if (Obj.RequiredCount > 1)
			{
				ObjStr += FString::Printf(TEXT(" (%d/%d)"), Obj.CurrentCount, Obj.RequiredCount);
			}
			ObjStr += TEXT("\n");
		}
		ObjectivesText->SetText(FText::FromString(ObjStr));
	}

	if (RewardsText)
	{
		FString Rewards;
		if (Quest.XPReward > 0)
		{
			Rewards += FString::Printf(TEXT("XP: %d\n"), Quest.XPReward);
		}
		for (const FName& ItemID : Quest.ItemRewards)
		{
			Rewards += FString::Printf(TEXT("Item: %s\n"), *ItemID.ToString());
		}
		RewardsText->SetText(FText::FromString(Rewards));
	}

	if (TrackingText)
	{
		bool bIsTracked = IsTracked(SelectedQuestID);
		TrackingText->SetText(bIsTracked
			? NSLOCTEXT("Quest", "Tracked", "[TRACKING]")
			: NSLOCTEXT("Quest", "NotTracked", "Press T to Track"));
	}

	OnQuestSelected(Quest);
}

void USEEQuestLogWidget::ToggleTracking(FName QuestID)
{
	USEEQuestManager* QM = GetQuestManager();
	if (!QM) return;

	if (QM->GetTrackedQuestID() == QuestID)
	{
		QM->SetTrackedQuest(NAME_None);
	}
	else
	{
		QM->SetTrackedQuest(QuestID);
	}

	UpdateQuestDetails();
}

bool USEEQuestLogWidget::IsTracked(FName QuestID) const
{
	USEEQuestManager* QM = GetQuestManager();
	return QM && QM->GetTrackedQuestID() == QuestID;
}
