// SEEHUDTypes.h - Type definitions for the HUD/UI system
#pragma once

#include "CoreMinimal.h"
#include "SEEHUDTypes.generated.h"

// A single dialogue choice presented to the player
USTRUCT(BlueprintType)
struct FDialogueChoice
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ChoiceText;

	// Gameplay tag or ID for the choice outcome
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChoiceID;

	// Optional: required stat check (e.g., "Morale >= 50")
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StatRequirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RequiredValue = 0.0f;

	// Whether this choice is currently available (stat check passed)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAvailable = true;

	// Optional tooltip explaining why unavailable
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnavailableReason;
};

// A dialogue line with speaker info and optional choices
USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	// If non-empty, this line presents choices to the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDialogueChoice> Choices;
};

// Faction reputation data for display
USTRUCT(BlueprintType)
struct FFactionReputation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FactionName;

	// Current reputation value (-100 to 100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Reputation = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor FactionColor = FLinearColor::White;
};

// Delegate fired when the player selects a dialogue choice
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueChoiceSelected, FName, ChoiceID);

// Delegate fired when dialogue is dismissed (no choices, player pressed continue)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueDismissed);
