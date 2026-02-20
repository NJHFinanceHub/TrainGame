// ColdZoneVolume.h - Level volume that applies cold exposure to characters inside it
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "ColdZoneVolume.generated.h"

class USurvivalComponent;

/**
 * AColdZoneVolume
 *
 * Place in levels to define areas with cold exposure (hull breaches, exterior sections,
 * damaged cars, freezer breach). Characters with a USurvivalComponent that enter the
 * volume will have their environment cold level set based on this volume's ColdIntensity.
 *
 * Overlapping volumes use the highest cold intensity value.
 */
UCLASS(Blueprintable)
class SNOWYENGINE_API AColdZoneVolume : public AActor
{
	GENERATED_BODY()

public:
	AColdZoneVolume();

	/** Cold intensity of this zone (0=mild chill, 1=lethal freezing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold Zone", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ColdIntensity = 0.5f;

	/** If true, this zone is active. Can be toggled by gameplay events (hull breach repair). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cold Zone")
	bool bIsActive = true;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Cold Zone")
	UBoxComponent* ZoneVolume = nullptr;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
