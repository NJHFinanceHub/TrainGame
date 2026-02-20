#include "SEETrainMapWidget.h"
#include "Components/ScrollBox.h"

void USEETrainMapWidget::SetCarEntries(const TArray<FSEEMapCarEntry>& Entries)
{
	CarEntries = Entries;
	OnMapUpdated();
}

void USEETrainMapWidget::SetCurrentCar(int32 CarIndex)
{
	// Clear old current flag
	for (FSEEMapCarEntry& Entry : CarEntries)
	{
		Entry.bCurrentCar = (Entry.CarIndex == CarIndex);
	}
	CurrentCarIndex = CarIndex;
	OnCurrentCarChanged(CarIndex);
}

void USEETrainMapWidget::SetCarVisited(int32 CarIndex, bool bVisited)
{
	for (FSEEMapCarEntry& Entry : CarEntries)
	{
		if (Entry.CarIndex == CarIndex)
		{
			Entry.bVisited = bVisited;
			break;
		}
	}
	OnMapUpdated();
}

void USEETrainMapWidget::SetCarObjective(int32 CarIndex, bool bHasObjective)
{
	for (FSEEMapCarEntry& Entry : CarEntries)
	{
		if (Entry.CarIndex == CarIndex)
		{
			Entry.bHasObjective = bHasObjective;
			break;
		}
	}
	OnMapUpdated();
}

void USEETrainMapWidget::ScrollToCurrentCar()
{
	ScrollToCar(CurrentCarIndex);
}

void USEETrainMapWidget::ScrollToCar(int32 CarIndex)
{
	if (CarScrollBox && CarScrollBox->GetChildrenCount() > 0)
	{
		// Find the child widget corresponding to this car index
		for (int32 i = 0; i < CarScrollBox->GetChildrenCount(); ++i)
		{
			if (i < CarEntries.Num() && CarEntries[i].CarIndex == CarIndex)
			{
				UWidget* ChildWidget = CarScrollBox->GetChildAt(i);
				if (ChildWidget)
				{
					CarScrollBox->ScrollWidgetIntoView(ChildWidget, true);
				}
				break;
			}
		}
	}
}
