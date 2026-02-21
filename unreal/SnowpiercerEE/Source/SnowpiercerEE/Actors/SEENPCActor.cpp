// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "SEENPCActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

ASEENPCActor::ASEENPCActor()
{
	PrimaryActorTick.bCanEverTick = false;

	DialogueTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("DialogueTrigger"));
	DialogueTrigger->SetupAttachment(GetRootComponent());
	DialogueTrigger->InitSphereRadius(DialogueTriggerRadius);
	DialogueTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DialogueTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	DialogueTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DialogueTrigger->OnComponentBeginOverlap.AddDynamic(this, &ASEENPCActor::HandleDialogueTriggerBegin);
}

void ASEENPCActor::BeginPlay()
{
	Super::BeginPlay();

	if (DialogueTrigger)
	{
		DialogueTrigger->SetSphereRadius(DialogueTriggerRadius);
	}
}

void ASEENPCActor::Interact(AActor* Interactor)
{
	if (!Interactor || Interactor == this)
	{
		return;
	}

	OnInteract(Interactor);
}

void ASEENPCActor::HandleDialogueTriggerBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!bAutoInteractOnDialogueTrigger || !OtherActor || OtherActor == this)
	{
		return;
	}

	Interact(OtherActor);
}
