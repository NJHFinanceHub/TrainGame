// SEETrainMapWidget.h - Train map: 100-car linear diagram, scrollable, zone colors, fog of war
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SEETypes.h"
#include "SEETrainMapWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UImage;
class UCanvasPanel;
class USEESaveGameSubsystem;

USTRUCT(BlueprintType)
struct FSEEMapCarEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CarIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CarName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEETrainZone Zone = ESEETrainZone::Tail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVisited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPlayerLocation = false;
};

UCLASS(Abstract, Blueprintable)
class SNOWPIERCEREE_API USEETrainMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "TrainMap")
	void RefreshMap();

	UFUNCTION(BlueprintCallable, Category = "TrainMap")
	void SetPlayerCarIndex(int32 CarIndex);

	UFUNCTION(BlueprintCallable, Category = "TrainMap")
	void ScrollToPlayerCar();

	UFUNCTION(BlueprintCallable, Category = "TrainMap")
	void SetCarData(const TArray<FSEECarData>& Cars);

	UFUNCTION(BlueprintPure, Category = "TrainMap")
	FLinearColor GetZoneColor(ESEETrainZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "TrainMap")
	FText GetZoneDisplayName(ESEETrainZone Zone) const;

	UFUNCTION(BlueprintPure, Category = "TrainMap")
	TArray<FSEEMapCarEntry> GetMapEntries() const { return MapEntries; }

	UFUNCTION(BlueprintPure, Category = "TrainMap")
	int32 GetPlayerCarIndex() const { return PlayerCarIndex; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UScrollBox> CarScrollBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ZoneNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CarNameText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CarInfoText;

	UFUNCTION(BlueprintImplementableEvent, Category = "TrainMap")
	void OnMapRefreshed();

	UFUNCTION(BlueprintImplementableEvent, Category = "TrainMap")
	void OnCarSelected(int32 CarIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor TailColor = FLinearColor(0.4f, 0.2f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor ThirdClassColor = FLinearColor(0.6f, 0.4f, 0.2f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor SecondClassColor = FLinearColor(0.3f, 0.5f, 0.7f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor WorkingSpineColor = FLinearColor(0.5f, 0.5f, 0.3f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor FirstClassColor = FLinearColor(0.8f, 0.7f, 0.3f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor SanctumColor = FLinearColor(0.6f, 0.2f, 0.6f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor EngineColor = FLinearColor(0.8f, 0.1f, 0.1f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrainMap|Colors")
	FLinearColor FogOfWarColor = FLinearColor(0.15f, 0.15f, 0.15f, 0.8f);

private:
	TArray<FSEEMapCarEntry> MapEntries;
	TArray<FSEECarData> CarDataSource;
	int32 PlayerCarIndex = 0;
};
