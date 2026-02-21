// SEETrainMapWidget.cpp
#include "SEETrainMapWidget.h"
#include "SEESaveGameSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void USEETrainMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshMap();
}

void USEETrainMapWidget::SetCarData(const TArray<FSEECarData>& Cars)
{
	CarDataSource = Cars;
}

void USEETrainMapWidget::SetPlayerCarIndex(int32 CarIndex)
{
	PlayerCarIndex = CarIndex;
}

void USEETrainMapWidget::RefreshMap()
{
	MapEntries.Empty();

	USEESaveGameSubsystem* SaveSub = nullptr;
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		SaveSub = GI->GetSubsystem<USEESaveGameSubsystem>();
	}

	for (const FSEECarData& Car : CarDataSource)
	{
		FSEEMapCarEntry Entry;
		Entry.CarIndex = Car.CarIndex;
		Entry.CarName = Car.CarName;
		Entry.Zone = Car.Zone;
		Entry.bIsPlayerLocation = (Car.CarIndex == PlayerCarIndex);

		if (SaveSub)
		{
			FSEECarState State;
			if (SaveSub->GetCarState(Car.CarIndex, State))
			{
				Entry.bVisited = State.bVisited;
			}
		}

		MapEntries.Add(Entry);
	}

	OnMapRefreshed();
}

void USEETrainMapWidget::ScrollToPlayerCar()
{
	if (!CarScrollBox) return;

	// Scroll proportionally based on player position
	int32 TotalCars = MapEntries.Num();
	if (TotalCars <= 0) return;

	float ScrollPercent = static_cast<float>(PlayerCarIndex) / static_cast<float>(TotalCars);
	CarScrollBox->SetScrollOffset(CarScrollBox->GetScrollOffsetOfEnd() * ScrollPercent);
}

FLinearColor USEETrainMapWidget::GetZoneColor(ESEETrainZone Zone) const
{
	switch (Zone)
	{
	case ESEETrainZone::Tail: return TailColor;
	case ESEETrainZone::ThirdClass: return ThirdClassColor;
	case ESEETrainZone::SecondClass: return SecondClassColor;
	case ESEETrainZone::WorkingSpine: return WorkingSpineColor;
	case ESEETrainZone::FirstClass: return FirstClassColor;
	case ESEETrainZone::Sanctum: return SanctumColor;
	case ESEETrainZone::Engine: return EngineColor;
	default: return FLinearColor::Gray;
	}
}

FText USEETrainMapWidget::GetZoneDisplayName(ESEETrainZone Zone) const
{
	switch (Zone)
	{
	case ESEETrainZone::Tail: return NSLOCTEXT("Map", "Tail", "The Tail");
	case ESEETrainZone::ThirdClass: return NSLOCTEXT("Map", "Third", "Third Class");
	case ESEETrainZone::SecondClass: return NSLOCTEXT("Map", "Second", "Second Class");
	case ESEETrainZone::WorkingSpine: return NSLOCTEXT("Map", "Spine", "Working Spine");
	case ESEETrainZone::FirstClass: return NSLOCTEXT("Map", "First", "First Class");
	case ESEETrainZone::Sanctum: return NSLOCTEXT("Map", "Sanctum", "The Sanctum");
	case ESEETrainZone::Engine: return NSLOCTEXT("Map", "Engine", "The Engine");
	default: return NSLOCTEXT("Map", "Unknown", "Unknown");
	}
}
