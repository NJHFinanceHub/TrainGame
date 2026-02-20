#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../SEETypes.h"
#include "SEETrainMapWidget.generated.h"

USTRUCT(BlueprintType)
struct FSEEMapCarEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	int32 CarIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	FName CarName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	ESEETrainZone Zone = ESEETrainZone::Tail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	bool bVisited = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	bool bCurrentCar = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Map")
	bool bHasObjective = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSEEMapCarSelected, int32, CarIndex);

UCLASS(Abstract)
class SNOWPIERCEREE_API USEETrainMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Map")
	void SetCarEntries(const TArray<FSEEMapCarEntry>& Entries);

	UFUNCTION(BlueprintCallable, Category="Map")
	void SetCurrentCar(int32 CarIndex);

	UFUNCTION(BlueprintCallable, Category="Map")
	void SetCarVisited(int32 CarIndex, bool bVisited);

	UFUNCTION(BlueprintCallable, Category="Map")
	void SetCarObjective(int32 CarIndex, bool bHasObjective);

	UFUNCTION(BlueprintCallable, Category="Map")
	void ScrollToCurrentCar();

	UFUNCTION(BlueprintCallable, Category="Map")
	void ScrollToCar(int32 CarIndex);

	UFUNCTION(BlueprintPure, Category="Map")
	int32 GetCurrentCarIndex() const { return CurrentCarIndex; }

	UFUNCTION(BlueprintPure, Category="Map")
	int32 GetCarCount() const { return CarEntries.Num(); }

	UPROPERTY(BlueprintAssignable, Category="Map")
	FOnSEEMapCarSelected OnCarSelected;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Map")
	TArray<FSEEMapCarEntry> CarEntries;

	UPROPERTY(BlueprintReadOnly, Category="Map")
	int32 CurrentCarIndex = 0;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidgetOptional), Category="Map")
	TObjectPtr<class UScrollBox> CarScrollBox;

	UFUNCTION(BlueprintImplementableEvent, Category="Map")
	void OnMapUpdated();

	UFUNCTION(BlueprintImplementableEvent, Category="Map")
	void OnCurrentCarChanged(int32 NewCarIndex);
};
