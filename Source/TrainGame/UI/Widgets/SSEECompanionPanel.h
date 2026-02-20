// SSEECompanionPanel.h - Companion roster, loyalty, and personal quest display
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UCompanionRosterSubsystem;

/**
 * SSEECompanionPanel
 *
 * Full-screen companion management showing:
 * - Active party members with loyalty state, role, behavior mode
 * - Benched companions in roster
 * - Per-companion detail: stats, personal quest progress, formation
 * - Dead companions memorial section
 *
 * Reads from UCompanionRosterSubsystem (GameInstance subsystem).
 * Toggled via ASEEGameHUD::ToggleCompanionPanel().
 */
class SSEECompanionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEECompanionPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void SetRosterSubsystem(UCompanionRosterSubsystem* InSubsystem);

private:
	TSharedRef<SWidget> MakeHeader();
	TSharedRef<SWidget> MakeActiveParty();
	TSharedRef<SWidget> MakeRosterList();

	FText GetLoyaltyText(int32 LoyaltyScore) const;
	FLinearColor GetLoyaltyColor(int32 LoyaltyScore) const;
	FText GetRoleText(uint8 RoleValue) const;

	TWeakObjectPtr<UCompanionRosterSubsystem> RosterSubsystem;
};
