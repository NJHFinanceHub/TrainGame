// SSEETrainMap.h - Train diagram/map showing car progression
#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

/**
 * SSEETrainMap
 *
 * Displays a horizontal diagram of the train showing:
 * - All registered cars as rectangles in a horizontal strip
 * - Color-coded by zone (Tail=dark grey, Third=brown, Second=tan, etc.)
 * - Visited cars highlighted, unvisited dimmed
 * - Current car marked with a pulsing indicator
 * - Completed cars show a checkmark
 * - Car names displayed on hover/selection
 *
 * The map scrolls horizontally as the player progresses. Early cars
 * (Tail) are on the right, Engine is on the left (matching the
 * train's direction of travel).
 */
class SSEETrainMap : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSEETrainMap) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	// Update the car data displayed on the map
	// Takes car name + zone index pairs to avoid circular dependency on SnowpiercerEE module
	void SetCarData(const TArray<FName>& CarNames, const TArray<int32>& ZoneIndices);

	// Update the state of a specific car (visited, completed, etc.)
	void UpdateCarState(int32 CarIndex, bool bVisited, bool bCompleted);

	// Set which car the player is currently in
	void SetCurrentCar(int32 CarIndex);

private:
	TSharedRef<SWidget> MakeMapHeader();
	TSharedRef<SWidget> MakeCarStrip();
	TSharedRef<SWidget> MakeLegend();

	// Get the zone color for a car
	FLinearColor GetZoneColor(int32 ZoneIndex) const;

	// Car display data
	struct FCarDisplayEntry
	{
		FText Name;
		int32 ZoneIndex = 0;
		bool bVisited = false;
		bool bCompleted = false;
	};

	TArray<FCarDisplayEntry> CarEntries;
	int32 CurrentCarIndex = INDEX_NONE;
	float PulseTimer = 0.0f;
	int32 HoveredCarIndex = INDEX_NONE;
};
