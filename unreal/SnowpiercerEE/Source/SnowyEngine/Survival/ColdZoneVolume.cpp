// ColdZoneVolume.cpp - Cold zone volume implementation
#include "ColdZoneVolume.h"
#include "SurvivalComponent.h"

AColdZoneVolume::AColdZoneVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneVolume"));
	ZoneVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	ZoneVolume->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	ZoneVolume->SetGenerateOverlapEvents(true);
	RootComponent = ZoneVolume;

	ZoneVolume->OnComponentBeginOverlap.AddDynamic(this, &AColdZoneVolume::OnBeginOverlap);
	ZoneVolume->OnComponentEndOverlap.AddDynamic(this, &AColdZoneVolume::OnEndOverlap);
}

void AColdZoneVolume::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsActive || !OtherActor) return;

	USurvivalComponent* Survival = OtherActor->FindComponentByClass<USurvivalComponent>();
	if (Survival)
	{
		// Use the higher of current and this zone's cold level
		float Current = 0.0f; // Would need to track per-actor, simplified here
		Survival->SetEnvironmentColdLevel(FMath::Max(Current, ColdIntensity));
	}
}

void AColdZoneVolume::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	USurvivalComponent* Survival = OtherActor->FindComponentByClass<USurvivalComponent>();
	if (Survival)
	{
		// Reset to no cold when leaving. If overlapping multiple zones,
		// a more robust system would track all overlapping volumes.
		Survival->SetEnvironmentColdLevel(0.0f);
	}
}
