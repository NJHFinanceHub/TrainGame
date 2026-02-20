#include "SEEColdComponent.h"
#include "SEEHealthComponent.h"

USEEColdComponent::USEEColdComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CurrentTemperature = BodyTemperature;
}

void USEEColdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float PrevTemp = CurrentTemperature;

	if (bInColdZone && !bNearFire)
	{
		float EffectiveCooling = CoolingRate * (1.0f - ColdSuitBonus);
		CurrentTemperature = FMath::Max(ZoneTemperature, CurrentTemperature - EffectiveCooling * DeltaTime);
	}
	else
	{
		CurrentTemperature = FMath::Min(BodyTemperature, CurrentTemperature + WarmingRate * DeltaTime);
	}

	if (FMath::Abs(PrevTemp - CurrentTemperature) > 0.1f)
	{
		OnTemperatureChanged.Broadcast(CurrentTemperature);
	}

	UpdateFrostbiteStage();

	// Blackout stage = health drain
	if (CurrentStage == ESEEFrostbiteStage::Blackout)
	{
		if (AActor* Owner = GetOwner())
		{
			if (USEEHealthComponent* Health = Owner->FindComponentByClass<USEEHealthComponent>())
			{
				Health->TakeDamage(BlackoutHealthDrain * DeltaTime, ESEEDamageType::Cold, nullptr);
			}
		}
	}
}

void USEEColdComponent::EnterColdZone(float InZoneTemperature)
{
	bInColdZone = true;
	ZoneTemperature = InZoneTemperature;
}

void USEEColdComponent::ExitColdZone()
{
	bInColdZone = false;
}

void USEEColdComponent::SetNearFireSource(bool bNearFire_In)
{
	bNearFire = bNearFire_In;
}

void USEEColdComponent::SetColdSuitBonus(float Bonus)
{
	ColdSuitBonus = FMath::Clamp(Bonus, 0.0f, 0.9f);
}

float USEEColdComponent::GetMoveSpeedModifier() const
{
	switch (CurrentStage)
	{
	case ESEEFrostbiteStage::Numbness: return 0.6f;
	case ESEEFrostbiteStage::Blackout: return 0.3f;
	default: return 1.0f;
	}
}

void USEEColdComponent::UpdateFrostbiteStage()
{
	ESEEFrostbiteStage NewStage = ESEEFrostbiteStage::None;

	if (CurrentTemperature <= BlackoutThreshold)
	{
		NewStage = ESEEFrostbiteStage::Blackout;
	}
	else if (CurrentTemperature <= NumbnessThreshold)
	{
		NewStage = ESEEFrostbiteStage::Numbness;
	}
	else if (CurrentTemperature <= ShiverThreshold)
	{
		NewStage = ESEEFrostbiteStage::Shivers;
	}

	if (NewStage != CurrentStage)
	{
		CurrentStage = NewStage;
		OnFrostbiteStageChanged.Broadcast(CurrentStage);
	}
}
