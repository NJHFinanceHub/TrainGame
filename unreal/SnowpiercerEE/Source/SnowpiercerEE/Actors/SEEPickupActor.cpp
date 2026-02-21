// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "Actors/SEEPickupActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SEEInventoryComponent.h"

ASEEPickupActor::ASEEPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	SetRootComponent(VisualMesh);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(VisualMesh);
	OverlapSphere->InitSphereRadius(100.0f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ASEEPickupActor::HandleOverlapBegin);
}

void ASEEPickupActor::BeginPlay()
{
	Super::BeginPlay();
}

bool ASEEPickupActor::TryPickup(AActor* Picker)
{
	if (bConsumed || !Picker || ItemID.IsNone() || Quantity <= 0)
	{
		return false;
	}

	USEEInventoryComponent* Inventory = Picker->FindComponentByClass<USEEInventoryComponent>();
	if (!Inventory)
	{
		return false;
	}

	if (!Inventory->AddItem(ItemID, Quantity))
	{
		return false;
	}

	bConsumed = true;
	OnPickedUp(Picker);

	if (bDestroyOnPickup)
	{
		Destroy();
	}
	else
	{
		SetActorEnableCollision(false);
		SetActorHiddenInGame(true);
	}

	return true;
}

void ASEEPickupActor::HandleOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bAutoPickupOnOverlap || !OtherActor || OtherActor == this)
	{
		return;
	}

	TryPickup(OtherActor);
}
