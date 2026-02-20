#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEDialogueWidget.generated.h"

USTRUCT(BlueprintType)
struct FSEEDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FName OptionID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bRequiresSkillCheck = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FName RequiredSkill = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 RequiredSkillLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	int32 PlayerSkillLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	bool bLocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText LockedReason;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSEEDialogueOptionSelected, int32, OptionIndex, FName, OptionID);

UCLASS(Abstract)
class SNOWPIERCEREE_API USEEDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void ShowDialogue(const FText& SpeakerName, const FText& DialogueText, const TArray<FSEEDialogueOption>& Options);

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void HideDialogue();

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SelectOption(int32 OptionIndex);

	UFUNCTION(BlueprintPure, Category="Dialogue")
	bool IsDialogueActive() const { return bDialogueActive; }

	UFUNCTION(BlueprintPure, Category="Dialogue")
	const TArray<FSEEDialogueOption>& GetCurrentOptions() const { return CurrentOptions; }

	UPROPERTY(BlueprintAssignable, Category="Dialogue")
	FOnSEEDialogueOptionSelected OnOptionSelected;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Dialogue")
	TObjectPtr<class UTextBlock> SpeakerNameText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Dialogue")
	TObjectPtr<class UTextBlock> DialogueBodyText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Dialogue")
	TObjectPtr<class UWidget> DialoguePanel;

	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	TArray<FSEEDialogueOption> CurrentOptions;

	UPROPERTY(BlueprintReadOnly, Category="Dialogue")
	bool bDialogueActive = false;

	UFUNCTION(BlueprintImplementableEvent, Category="Dialogue")
	void OnDialogueShown();

	UFUNCTION(BlueprintImplementableEvent, Category="Dialogue")
	void OnDialogueHidden();

	static constexpr int32 MaxDialogueOptions = 4;
};
