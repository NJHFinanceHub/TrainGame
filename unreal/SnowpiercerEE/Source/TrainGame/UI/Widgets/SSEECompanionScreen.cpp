// SSEECompanionScreen.cpp - Companion screen implementation
#include "SSEECompanionScreen.h"

#include "TrainGame/Companions/CompanionComponent.h"
#include "TrainGame/Companions/CompanionRosterSubsystem.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

void SSEECompanionScreen::Construct(const FArguments& InArgs)
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
			.Padding(0, 0, 0, 12)
			[
				MakeHeader()
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.FillWidth(0.4f)
				.Padding(0, 0, 8, 0)
				[
					MakeRosterList()
				]

				+ SHorizontalBox::Slot()
				.FillWidth(0.6f)
				[
					MakeCompanionDetail()
				]
			]
		]
	];
}

TSharedRef<SWidget> SSEECompanionScreen::MakeHeader()
{
	return SNew(STextBlock)
		.Text(NSLOCTEXT("HUD", "Companions", "COMPANIONS"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
		.ColorAndOpacity(FSlateColor(FLinearColor(0.9f, 0.85f, 0.7f)));
}

TSharedRef<SWidget> SSEECompanionScreen::MakeRosterList()
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
					if (!RosterSubsystem.IsValid())
					{
						return NSLOCTEXT("HUD", "NoRoster", "No companion data");
					}

					FString Result;

					// Active party
					TArray<UCompanionComponent*> Active = RosterSubsystem->GetActiveParty();
					Result += TEXT("== ACTIVE PARTY ==\n\n");
					if (Active.Num() == 0)
					{
						Result += TEXT("  (no active companions)\n");
					}
					for (UCompanionComponent* Comp : Active)
					{
						if (!Comp) continue;
						FText LoyaltyLabel = GetLoyaltyStateText(Comp->GetLoyaltyState());
						Result += FString::Printf(TEXT("  %s  [%s]\n  %s  |  %s\n\n"),
							*Comp->GetCompanionName().ToString(),
							*GetRoleText(Comp->GetRole()).ToString(),
							*LoyaltyLabel.ToString(),
							*GetBehaviorText(Comp->GetBehaviorMode()).ToString());
					}

					// Benched
					TArray<UCompanionComponent*> Benched = RosterSubsystem->GetBenchedCompanions();
					if (Benched.Num() > 0)
					{
						Result += TEXT("\n== ROSTER ==\n\n");
						for (UCompanionComponent* Comp : Benched)
						{
							if (!Comp) continue;
							FText LoyaltyLabel = GetLoyaltyStateText(Comp->GetLoyaltyState());
							Result += FString::Printf(TEXT("  %s  [%s]\n  %s\n\n"),
								*Comp->GetCompanionName().ToString(),
								*GetRoleText(Comp->GetRole()).ToString(),
								*LoyaltyLabel.ToString());
						}
					}

					// Dead companions
					TArray<UCompanionComponent*> Dead = RosterSubsystem->GetDeadCompanions();
					if (Dead.Num() > 0)
					{
						Result += TEXT("\n== FALLEN ==\n\n");
						for (UCompanionComponent* Comp : Dead)
						{
							if (!Comp) continue;
							Result += FString::Printf(TEXT("  %s  (deceased)\n"),
								*Comp->GetCompanionName().ToString());
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

TSharedRef<SWidget> SSEECompanionScreen::MakeCompanionDetail()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("GenericWhiteBox"))
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.07f, 0.9f))
		.Padding(12.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("HUD", "CompDetail", "COMPANION DETAILS"))
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
						if (!RosterSubsystem.IsValid())
						{
							return NSLOCTEXT("HUD", "SelectComp", "Select a companion to view details");
						}

						// Show first active companion by default if nothing selected
						TArray<UCompanionComponent*> Active = RosterSubsystem->GetActiveParty();
						UCompanionComponent* Selected = nullptr;
						if (!SelectedCompanionID.IsNone())
						{
							for (UCompanionComponent* Comp : RosterSubsystem->GetFullRoster())
							{
								if (Comp && Comp->GetCompanionID() == SelectedCompanionID)
								{
									Selected = Comp;
									break;
								}
							}
						}
						if (!Selected && Active.Num() > 0)
						{
							Selected = Active[0];
						}

						if (!Selected)
						{
							return NSLOCTEXT("HUD", "NoCompSel", "No companions recruited yet");
						}

						FString Detail;
						Detail += Selected->GetCompanionName().ToString() + TEXT("\n");
						Detail += GetRoleText(Selected->GetRole()).ToString() + TEXT("\n\n");

						// Loyalty
						Detail += TEXT("LOYALTY\n");
						Detail += TEXT("  ") + GetLoyaltyStateText(Selected->GetLoyaltyState()).ToString() + TEXT("\n\n");

						// Behavior
						Detail += TEXT("BEHAVIOR MODE\n");
						Detail += TEXT("  ") + GetBehaviorText(Selected->GetBehaviorMode()).ToString() + TEXT("\n\n");

						// Stats
						FCompanionStats Stats = Selected->GetStats();
						Detail += TEXT("STATS\n");
						Detail += FString::Printf(TEXT("  STR: %d  END: %d  CUN: %d\n"), Stats.Strength, Stats.Endurance, Stats.Cunning);
						Detail += FString::Printf(TEXT("  PER: %d  CHA: %d  SUR: %d\n\n"), Stats.Perception, Stats.Charisma, Stats.Survival);

						// Personal Quest
						FCompanionQuestStep CurrentStep = Selected->GetCurrentQuestStep();
						if (!CurrentStep.QuestID.IsNone())
						{
							Detail += TEXT("PERSONAL QUEST\n");
							Detail += TEXT("  ") + CurrentStep.QuestTitle.ToString() + TEXT("\n");
							Detail += TEXT("  ") + CurrentStep.QuestDescription.ToString() + TEXT("\n");
							Detail += CurrentStep.bCompleted ? TEXT("  [COMPLETED]\n") : TEXT("  [IN PROGRESS]\n");
						}

						return FText::FromString(Detail);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f)))
					.AutoWrapText(true)
				]
			]
		];
}

FText SSEECompanionScreen::GetLoyaltyStateText(ELoyaltyState State) const
{
	switch (State)
	{
	case ELoyaltyState::Hostile:   return NSLOCTEXT("HUD", "LoyHostile", "Hostile");
	case ELoyaltyState::Resentful: return NSLOCTEXT("HUD", "LoyResentful", "Resentful");
	case ELoyaltyState::Cold:      return NSLOCTEXT("HUD", "LoyCold", "Cold");
	case ELoyaltyState::Neutral:   return NSLOCTEXT("HUD", "LoyNeutral", "Neutral");
	case ELoyaltyState::Friendly:  return NSLOCTEXT("HUD", "LoyFriendly", "Friendly");
	case ELoyaltyState::Devoted:   return NSLOCTEXT("HUD", "LoyDevoted", "Devoted");
	case ELoyaltyState::Bonded:    return NSLOCTEXT("HUD", "LoyBonded", "Bonded");
	default: return FText::GetEmpty();
	}
}

FLinearColor SSEECompanionScreen::GetLoyaltyColor(ELoyaltyState State) const
{
	switch (State)
	{
	case ELoyaltyState::Hostile:   return FLinearColor(0.9f, 0.15f, 0.1f);
	case ELoyaltyState::Resentful: return FLinearColor(0.9f, 0.4f, 0.2f);
	case ELoyaltyState::Cold:      return FLinearColor(0.6f, 0.6f, 0.7f);
	case ELoyaltyState::Neutral:   return FLinearColor(0.7f, 0.7f, 0.7f);
	case ELoyaltyState::Friendly:  return FLinearColor(0.3f, 0.7f, 0.3f);
	case ELoyaltyState::Devoted:   return FLinearColor(0.3f, 0.5f, 1.0f);
	case ELoyaltyState::Bonded:    return FLinearColor(0.9f, 0.7f, 0.2f);
	default: return FLinearColor::White;
	}
}

FText SSEECompanionScreen::GetBehaviorText(ECompanionBehavior Behavior) const
{
	switch (Behavior)
	{
	case ECompanionBehavior::Aggressive:   return NSLOCTEXT("HUD", "BehAgg", "Aggressive");
	case ECompanionBehavior::Defensive:    return NSLOCTEXT("HUD", "BehDef", "Defensive");
	case ECompanionBehavior::Passive:      return NSLOCTEXT("HUD", "BehPas", "Passive");
	case ECompanionBehavior::HoldPosition: return NSLOCTEXT("HUD", "BehHold", "Hold Position");
	default: return FText::GetEmpty();
	}
}

FText SSEECompanionScreen::GetRoleText(ECompanionRole Role) const
{
	switch (Role)
	{
	case ECompanionRole::Medic:      return NSLOCTEXT("HUD", "RoleMedic", "Medic");
	case ECompanionRole::MeleeDPS:   return NSLOCTEXT("HUD", "RoleMelee", "Melee DPS");
	case ECompanionRole::Tank:       return NSLOCTEXT("HUD", "RoleTank", "Tank");
	case ECompanionRole::Stealth:    return NSLOCTEXT("HUD", "RoleStealth", "Stealth");
	case ECompanionRole::Support:    return NSLOCTEXT("HUD", "RoleSupport", "Support");
	case ECompanionRole::Utility:    return NSLOCTEXT("HUD", "RoleUtility", "Utility");
	case ECompanionRole::Ranged:     return NSLOCTEXT("HUD", "RoleRanged", "Ranged");
	case ECompanionRole::Social:     return NSLOCTEXT("HUD", "RoleSocial", "Social");
	case ECompanionRole::Alchemist:  return NSLOCTEXT("HUD", "RoleAlch", "Alchemist");
	case ECompanionRole::Scout:      return NSLOCTEXT("HUD", "RoleScout", "Scout");
	case ECompanionRole::Electronic: return NSLOCTEXT("HUD", "RoleEW", "Electronic Warfare");
	default: return FText::GetEmpty();
	}
}
