// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ZoneVFXProfile.h"

FInt32Range UZoneVFXProfile::GetCarRange() const
{
	switch (Zone)
	{
	case EVFXZone::Tail:			return FInt32Range(1, 15);
	case EVFXZone::ThirdClass:		return FInt32Range(16, 30);
	case EVFXZone::SecondClass:		return FInt32Range(31, 48);
	case EVFXZone::WorkingSpine:	return FInt32Range(49, 62);
	case EVFXZone::FirstClass:		return FInt32Range(63, 82);
	case EVFXZone::Sanctum:			return FInt32Range(83, 95);
	case EVFXZone::Engine:			return FInt32Range(96, 103);
	default:						return FInt32Range(0, 0);
	}
}

bool UZoneVFXProfile::ContainsCar(int32 CarNumber) const
{
	const FInt32Range Range = GetCarRange();
	return CarNumber >= Range.GetLowerBoundValue() && CarNumber <= Range.GetUpperBoundValue();
}

const FImpactVFXConfig* UZoneVFXProfile::FindImpactConfig(EImpactSurface Surface) const
{
	for (const FImpactVFXConfig& Config : ImpactProfiles)
	{
		if (Config.Surface == Surface)
		{
			return &Config;
		}
	}
	return nullptr;
}
