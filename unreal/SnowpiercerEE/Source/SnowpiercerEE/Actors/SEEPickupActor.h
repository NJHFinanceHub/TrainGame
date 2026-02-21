// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SEEPickupActor.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class USphereComponent;

UCLASS(Blueprintable)
class SNOWPIERCEREE_API ASEEPickupActor : public AActor
{
	GENERATED_BODY()

public:
	ASEEPickupActor();

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	bool TryPickup(AActor* Picker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickedUp(AActor* Picker);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> OverlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	FName ItemID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bAutoPickupOnOverlap = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	bool bDestroyOnPickup = true;

private:
	bool bConsumed = false;
};
