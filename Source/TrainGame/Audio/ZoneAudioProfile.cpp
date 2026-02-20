// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ZoneAudioProfile.h"

FInt32Range UZoneAudioProfile::GetCarRange() const
{
	switch (Zone)
	{
	case EAudioZone::Tail:			return FInt32Range(1, 15);
	case EAudioZone::ThirdClass:	return FInt32Range(16, 30);
	case EAudioZone::SecondClass:	return FInt32Range(31, 48);
	case EAudioZone::WorkingSpine:	return FInt32Range(49, 62);
	case EAudioZone::FirstClass:	return FInt32Range(63, 82);
	case EAudioZone::Sanctum:		return FInt32Range(83, 95);
	case EAudioZone::Engine:		return FInt32Range(96, 103);
	default:						return FInt32Range(0, 0);
	}
}

bool UZoneAudioProfile::ContainsCar(int32 CarNumber) const
{
	FInt32Range Range = GetCarRange();
	return CarNumber >= Range.GetLowerBoundValue() && CarNumber <= Range.GetUpperBoundValue();
}
