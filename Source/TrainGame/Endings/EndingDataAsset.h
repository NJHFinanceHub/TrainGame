// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EndingTypes.h"
#include "EndingDataAsset.generated.h"

// ============================================================================
// Ending Data Asset
// Blueprint-configurable asset containing cinematic data for all 6 endings.
// Create one instance in Content/DataAssets/ and reference it from the
// EndingCalculatorSubsystem during game initialization.
// ============================================================================

UCLASS(BlueprintType)
class TRAINGAME_API UEndingDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Cinematic configuration for each ending. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Endings")
	TArray<FEndingCinematicData> Endings;

	/** Look up cinematic data by ending type. Returns nullptr if not configured. */
	UFUNCTION(BlueprintCallable, Category = "Endings")
	bool GetCinematicData(EEndingType EndingType, FEndingCinematicData& OutData) const
	{
		for (const FEndingCinematicData& Data : Endings)
		{
			if (Data.EndingType == EndingType)
			{
				OutData = Data;
				return true;
			}
		}
		return false;
	}

	/** Get the default credits music mapping. */
	UFUNCTION(BlueprintPure, Category = "Endings")
	static ECreditsMusic GetDefaultCreditsMusic(EEndingType Ending)
	{
		switch (Ending)
		{
		case EEndingType::TheNewWilford:	return ECreditsMusic::Triumphant;
		case EEndingType::TheRevolution:	return ECreditsMusic::Triumphant;
		case EEndingType::TheDerailment:	return ECreditsMusic::Somber;
		case EEndingType::TheExodus:			return ECreditsMusic::Hopeful;
		case EEndingType::TheEternalLoop:	return ECreditsMusic::Unsettling;
		case EEndingType::TheBridge:			return ECreditsMusic::Mysterious;
		default:							return ECreditsMusic::Default;
		}
	}

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("EndingData", GetFName());
	}
};
