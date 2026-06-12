// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SEEPickupActor.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;
class USphereComponent;
class USEEInventoryComponent;

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

	/** Configure the pickup after spawn (used by the level fixup subsystem). */
	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void InitPickup(FName InItemID, int32 InQuantity);

	/**
	 * Map a loosely named pickup ID to the DT_Items row it was meant to grant.
	 * The BP_Pickup_* content carries short IDs ("Bandage", "Pipe", "Cloth")
	 * while DT_Items rows are canonical ("Item_Bandage", "Item_RustyPipe",
	 * "Item_ClothStrips"). Resolution order:
	 *   1. the raw ID if the inventory's table already knows it
	 *   2. an explicit alias (Pipe -> Item_RustyPipe, Medicine -> Item_Painkillers, ...)
	 *   3. the "Item_" prefixed form
	 *   4. the raw ID unchanged (graceful fallback for unknown items)
	 */
	static FName ResolveItemRowName(FName RawItemID, const USEEInventoryComponent* Inventory);

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

/**
 * USEEPickupFixupSubsystem
 *
 * Level repair for the placed pickup content. The BP_Pickup_* assets created
 * by Scripts/setup_game_content.py are parented to plain AActor with no
 * components, no ItemID and no pickup logic (the script's component/CDO setup
 * silently no-ops), so the ~58 instances placed by populate_zone1.py are
 * invisible and grant nothing.
 *
 * At world begin-play this subsystem finds every actor whose generated class
 * is named "BP_Pickup_<Item>..." but is NOT an ASEEPickupActor, replaces it
 * in-place with a real ASEEPickupActor and derives the ItemID from the class
 * name (resolved against DT_Items row names via ResolveItemRowName).
 */
UCLASS()
class SNOWPIERCEREE_API USEEPickupFixupSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override
	{
		return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
