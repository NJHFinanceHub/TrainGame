// SSEECompanionPanel.cpp - Companion panel implementation
#include "SSEECompanionPanel.h"

#include "TrainGame/Companions/CompanionRosterSubsystem.h"
#include "TrainGame/Companions/CompanionComponent.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SSEECompanionPanel::Construct(const FArguments& InArgs)
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
			.Padding(0, 0, 0, 16)
			[
				MakeHeader()
			]

			// Active party (top) + roster/benched (bottom)
			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			.Padding(0, 0, 0, 8)
			[
				MakeActiveParty()
			]

			+ SVerticalBox::Slot()
			.FillHeight(0.5f)
			[
				MakeRosterList()
			]
		]
	];
}

void SSEECompanionPanel::SetRosterSubsystem(UCompanionRosterSubsystem* InSubsystem)
{
	RosterSubsystem = InSubsystem;
}

TSharedRef<SWidget> SSEECompanionPanel::MakeHeader()
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("HUD", "Companions", "COMPANIONS"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)));
}

TSharedRef<SWidget> SSEECompanionPanel::MakeActiveParty()
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
				.Text(NSLOCTEXT("HUD", "ActiveParty", "ACTIVE PARTY"))
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
						if (!RosterSubsystem.IsValid())
						{
							return NSLOCTEXT("HUD", "NoCompData", "No companion data available");
						}

						TArray<UCompanionComponent*> Party = RosterSubsystem->GetActiveParty();
						if (Party.Num() == 0)
						{
							return NSLOCTEXT("HUD", "NoParty", "No active companions");
						}

						FString Result;
						for (UCompanionComponent* Comp : Party)
						{
							if (!Comp) continue;

							FText Name = Comp->GetCompanionName();
							ELoyaltyState Loyalty = Comp->GetLoyaltyState();
							FText LoyaltyLabel = GetLoyaltyText(Comp->GetLoyaltyScore());
							FText RoleLabel = GetRoleText(static_cast<uint8>(Comp->GetRole()));
							ECompanionBehavior Behavior = Comp->GetBehaviorMode();

							// Behavior mode text
							FString BehaviorStr;
							switch (Behavior)
							{
							case ECompanionBehavior::Aggressive: BehaviorStr = TEXT("Aggressive"); break;
							case ECompanionBehavior::Defensive:  BehaviorStr = TEXT("Defensive"); break;
							case ECompanionBehavior::Passive:    BehaviorStr = TEXT("Passive"); break;
							case ECompanionBehavior::HoldPosition: BehaviorStr = TEXT("Hold"); break;
							}

							Result += FString::Printf(TEXT("%s  [%s]\n"),
								*Name.ToString(), *RoleLabel.ToString());
							Result += FString::Printf(TEXT("  Loyalty: %s  |  Mode: %s\n"),
								*LoyaltyLabel.ToString(), *BehaviorStr);

							// Personal quest progress
							FCompanionQuestStep CurrentStep = Comp->GetCurrentQuestStep();
							if (!CurrentStep.QuestID.IsNone())
							{
								FString Completed = CurrentStep.bCompleted ? TEXT("[DONE]") : TEXT("[...]");
								Result += FString::Printf(TEXT("  Quest: %s %s\n"),
									*CurrentStep.QuestTitle.ToString(), *Completed);
							}

							// Stats summary
							FCompanionStats Stats = Comp->GetStats();
							Result += FString::Printf(
								TEXT("  STR:%d END:%d CUN:%d PER:%d CHA:%d SUR:%d\n"),
								Stats.Strength, Stats.Endurance, Stats.Cunning,
								Stats.Perception, Stats.Charisma, Stats.Survival);

							Result += TEXT("\n");
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.85f, 0.85f, 0.85f)))
					.AutoWrapText(true)
				]
			]
		];
}

TSharedRef<SWidget> SSEECompanionPanel::MakeRosterList()
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
				.Text(NSLOCTEXT("HUD", "Roster", "ROSTER"))
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
						if (!RosterSubsystem.IsValid())
						{
							return FText::GetEmpty();
						}

						FString Result;

						// Benched companions
						TArray<UCompanionComponent*> Benched = RosterSubsystem->GetBenchedCompanions();
						if (Benched.Num() > 0)
						{
							Result += TEXT("Available:\n");
							for (UCompanionComponent* Comp : Benched)
							{
								if (!Comp) continue;
								FText RoleLabel = GetRoleText(static_cast<uint8>(Comp->GetRole()));
								Result += FString::Printf(TEXT("  %s  [%s]  - %s\n"),
									*Comp->GetCompanionName().ToString(),
									*RoleLabel.ToString(),
									*GetLoyaltyText(Comp->GetLoyaltyScore()).ToString());
							}
							Result += TEXT("\n");
						}

						// Dead companions
						TArray<UCompanionComponent*> Dead = RosterSubsystem->GetDeadCompanions();
						if (Dead.Num() > 0)
						{
							Result += TEXT("Fallen:\n");
							for (UCompanionComponent* Comp : Dead)
							{
								if (!Comp) continue;
								Result += FString::Printf(TEXT("  %s  [DEAD]\n"),
									*Comp->GetCompanionName().ToString());
							}
						}

						if (Result.IsEmpty())
						{
							return NSLOCTEXT("HUD", "EmptyRoster", "No other companions recruited");
						}

						return FText::FromString(Result);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					.AutoWrapText(true)
				]
			]
		];
}

FText SSEECompanionPanel::GetLoyaltyText(int32 LoyaltyScore) const
{
	if (LoyaltyScore <= -60) return NSLOCTEXT("HUD", "LoyHostile", "Hostile");
	if (LoyaltyScore <= -30) return NSLOCTEXT("HUD", "LoyResentful", "Resentful");
	if (LoyaltyScore <= 0)   return NSLOCTEXT("HUD", "LoyCold", "Cold");
	if (LoyaltyScore <= 30)  return NSLOCTEXT("HUD", "LoyNeutral", "Neutral");
	if (LoyaltyScore <= 60)  return NSLOCTEXT("HUD", "LoyFriendly", "Friendly");
	if (LoyaltyScore <= 85)  return NSLOCTEXT("HUD", "LoyDevoted", "Devoted");
	return NSLOCTEXT("HUD", "LoyBonded", "Bonded");
}

FLinearColor SSEECompanionPanel::GetLoyaltyColor(int32 LoyaltyScore) const
{
	if (LoyaltyScore <= -60) return FLinearColor(0.9f, 0.15f, 0.1f);
	if (LoyaltyScore <= -30) return FLinearColor(0.9f, 0.4f, 0.2f);
	if (LoyaltyScore <= 0)   return FLinearColor(0.6f, 0.6f, 0.7f);
	if (LoyaltyScore <= 30)  return FLinearColor(0.7f, 0.7f, 0.7f);
	if (LoyaltyScore <= 60)  return FLinearColor(0.3f, 0.7f, 0.3f);
	if (LoyaltyScore <= 85)  return FLinearColor(0.3f, 0.5f, 1.0f);
	return FLinearColor(0.9f, 0.7f, 0.2f);
}

FText SSEECompanionPanel::GetRoleText(uint8 RoleValue) const
{
	switch (static_cast<ECompanionRole>(RoleValue))
	{
	case ECompanionRole::Medic:      return NSLOCTEXT("HUD", "RMedic", "Medic");
	case ECompanionRole::MeleeDPS:   return NSLOCTEXT("HUD", "RDPS", "Melee DPS");
	case ECompanionRole::Tank:       return NSLOCTEXT("HUD", "RTank", "Tank");
	case ECompanionRole::Stealth:    return NSLOCTEXT("HUD", "RStealth", "Stealth");
	case ECompanionRole::Support:    return NSLOCTEXT("HUD", "RSupport", "Support");
	case ECompanionRole::Utility:    return NSLOCTEXT("HUD", "RUtility", "Utility");
	case ECompanionRole::Ranged:     return NSLOCTEXT("HUD", "RRanged", "Ranged");
	case ECompanionRole::Social:     return NSLOCTEXT("HUD", "RSocial", "Social");
	case ECompanionRole::Alchemist:  return NSLOCTEXT("HUD", "RAlch", "Alchemist");
	case ECompanionRole::Scout:      return NSLOCTEXT("HUD", "RScout", "Scout");
	case ECompanionRole::Electronic: return NSLOCTEXT("HUD", "RElec", "Electronic");
	}
	return NSLOCTEXT("HUD", "RUnknown", "Unknown");
}
