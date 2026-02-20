// SEECodexWidget.h - Codex/journal: lore entries, collectibles tracker, completion percentages
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Exploration/CollectibleTypes.h"
#include "SEECodexWidget.generated.h"

class UCollectibleJournalSubsystem;
class UTextBlock;
class UProgressBar;

UENUM(BlueprintType)
enum class ESEECodexTab : uint8
{
	TrainLogs		UMETA(DisplayName = "Train Logs"),
	AudioRecordings	UMETA(DisplayName = "Audio Recordings"),
	Artifacts		UMETA(DisplayName = "Artifacts"),
	Blueprints		UMETA(DisplayName = "Blueprints"),
	FactionIntel	UMETA(DisplayName = "Faction Intel"),
	ManifestPages	UMETA(DisplayName = "Manifest Pages"),
	CodexEntries	UMETA(DisplayName = "Codex")
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEECodexWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Codex")
	void RefreshCodex();

	UFUNCTION(BlueprintCallable, Category = "Codex")
	void SetActiveTab(ESEECodexTab Tab);

	UFUNCTION(BlueprintCallable, Category = "Codex")
	void SelectEntry(FName EntryID);

	UFUNCTION(BlueprintCallable, Category = "Codex")
	void MarkEntryViewed(FName EntryID);

	UFUNCTION(BlueprintPure, Category = "Codex")
	ESEECodexTab GetActiveTab() const { return ActiveTab; }

	UFUNCTION(BlueprintPure, Category = "Codex")
	float GetTabCompletion(ESEECodexTab Tab) const;

	UFUNCTION(BlueprintPure, Category = "Codex")
	float GetOverallCompletion() const;

	UFUNCTION(BlueprintPure, Category = "Codex")
	int32 GetUnviewedCount() const;

	UFUNCTION(BlueprintPure, Category = "Codex")
	TArray<FCollectibleState> GetCurrentTabEntries() const;

	UFUNCTION(BlueprintPure, Category = "Codex")
	FText GetExplorationTitle() const;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TabNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> TabCompletionBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CompletionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EntryTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EntryBodyText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> OverallCompletionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> UnviewedBadgeText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Codex")
	void OnCodexRefreshed(ESEECodexTab Tab, const TArray<FCollectibleState>& Entries);

	UFUNCTION(BlueprintImplementableEvent, Category = "Codex")
	void OnEntrySelected(FName EntryID);

private:
	UCollectibleJournalSubsystem* GetJournal() const;
	ECollectibleType TabToCollectibleType(ESEECodexTab Tab) const;
	FText GetTabDisplayName(ESEECodexTab Tab) const;

	ESEECodexTab ActiveTab = ESEECodexTab::TrainLogs;
	FName SelectedEntryID;
};
