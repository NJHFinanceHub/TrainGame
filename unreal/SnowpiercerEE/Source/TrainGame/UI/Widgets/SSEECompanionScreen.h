// SSEECompanionScreen.h - Companion roster and detail display
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "TrainGame/Companions/CompanionTypes.h"

class UCompanionRosterSubsystem;
class UCompanionComponent;

/**
 * SSEECompanionScreen
 *
 * Full-screen companion management panel showing:
 * - Active party roster (up to 2 companions)
 * - Benched companions list
 * - Selected companion detail: loyalty state, behavior mode, role, stats
 * - Personal quest progress
 * - Unique ability info
 *
 * Reads from UCompanionRosterSubsystem and individual UCompanionComponent data.
 */
class SSEECompanionScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECompanionScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetRosterSubsystem(UCompanionRosterSubsystem* InRoster) { RosterSubsystem = InRoster; }

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeRosterList();
	TSharedRef<SWidget> MakeCompanionDetail();

	FText GetLoyaltyStateText(ELoyaltyState State) const;
	FLinearColor GetLoyaltyColor(ELoyaltyState State) const;
	FText GetBehaviorText(ECompanionBehavior Behavior) const;
	FText GetRoleText(ECompanionRole Role) const;

	TWeakObjectPtr<UCompanionRosterSubsystem> RosterSubsystem;
	FName SelectedCompanionID;
};
