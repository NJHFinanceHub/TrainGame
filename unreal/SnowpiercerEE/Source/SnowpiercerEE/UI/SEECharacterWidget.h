// SEECharacterWidget.h - Character screen: stats, skills, perks, equipment, level/XP
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEEStatsComponent.h"
#include "SEECharacterWidget.generated.h"

class USEEHealthComponent;
class USEEInventoryComponent;
class USEECombatComponent;
class ASEECharacter;
class UTextBlock;
class UProgressBar;

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEECharacterWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Character")
	void InitCharacter(ASEECharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void RefreshStats();

	UFUNCTION(BlueprintCallable, Category = "Character")
	bool SpendSkillPoint(ESEEStat Stat);

	UFUNCTION(BlueprintPure, Category = "Character")
	int32 GetStatValue(ESEEStat Stat) const;

	UFUNCTION(BlueprintPure, Category = "Character")
	FText GetStatDisplayName(ESEEStat Stat) const;

protected:
	virtual void NativeConstruct() override;

	// Stat displays
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> StrengthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> AgilityText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> EnduranceText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CunningText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> PerceptionText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CharismaText;

	// Level / XP
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> LevelText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> XPText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> XPBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SkillPointsText;

	// Health / Survival
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HealthSummaryText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ArmorSummaryText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> InjurySummaryText;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void OnStatsRefreshed();

private:
	void UpdateStatText(UTextBlock* Block, ESEEStat Stat);

	TWeakObjectPtr<ASEECharacter> CachedCharacter;
	TWeakObjectPtr<USEEStatsComponent> StatsComp;
	TWeakObjectPtr<USEEHealthComponent> HealthComp;
};
