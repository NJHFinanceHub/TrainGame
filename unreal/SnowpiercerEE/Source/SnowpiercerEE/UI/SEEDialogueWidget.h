// SEEDialogueWidget.h - Dialogue UI: speaker portrait, text, response options, timed responses
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEDialogueManager.h"
#include "SEEDialogueWidget.generated.h"

class UTextBlock;
class UImage;
class UVerticalBox;
class UProgressBar;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEEDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowDialogueNode(const FSEEDialogueNode& Node);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void ShowChoices(const TArray<FSEEDialogueChoice>& Choices);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(int32 ChoiceIndex);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SetSpeakerPortrait(UTexture2D* Portrait);

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsWaitingForChoice() const { return bWaitingForChoice; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool HasTimedResponse() const { return TimedResponseDuration > 0.0f; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SpeakerNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DialogueText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> SpeakerPortraitImage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> ChoicesBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TimerBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ContinuePromptText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkillCheckText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueNodeShown(const FSEEDialogueNode& Node);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnChoicesShown(const TArray<FSEEDialogueChoice>& Choices);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnChoiceSelected(int32 ChoiceIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnTimerExpired();

	UFUNCTION(BlueprintImplementableEvent, Category = "Dialogue")
	void OnDialogueEnded();

private:
	USEEDialogueManager* GetDialogueManager() const;

	bool bWaitingForChoice = false;
	float TimedResponseDuration = 0.0f;
	float TimedResponseTimer = 0.0f;
};
