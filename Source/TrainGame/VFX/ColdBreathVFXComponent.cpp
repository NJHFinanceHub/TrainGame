// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "ColdBreathVFXComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UColdBreathVFXComponent::UColdBreathVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UColdBreathVFXComponent::BeginPlay()
{
	Super::BeginPlay();
	BreathTimer = FMath::RandRange(0.f, BreathCycleInterval);
}

void UColdBreathVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BreathComp)
	{
		BreathComp->DestroyComponent();
		BreathComp = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UColdBreathVFXComponent::CreateBreathComponent()
{
	if (!BreathEffect || !GetOwner()) return;
	if (BreathComp) return;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USceneComponent* AttachParent = nullptr;

	if (Character && Character->GetMesh())
	{
		AttachParent = Character->GetMesh();
	}
	else
	{
		AttachParent = GetOwner()->GetRootComponent();
	}

	if (!AttachParent) return;

	BreathComp = NewObject<UNiagaraComponent>(GetOwner());
	if (!BreathComp) return;

	BreathComp->SetAsset(BreathEffect);
	BreathComp->SetAutoActivate(false);
	BreathComp->SetRelativeLocation(AttachOffset);

	if (Character && Character->GetMesh())
	{
		BreathComp->AttachToComponent(Character->GetMesh(),
			FAttachmentTransformRules::KeepRelativeTransform, AttachBoneName);
	}
	else
	{
		BreathComp->AttachToComponent(AttachParent,
			FAttachmentTransformRules::KeepRelativeTransform);
	}

	BreathComp->RegisterComponent();
}

void UColdBreathVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bBreathActive) return;

	BreathTimer += DeltaTime;
	if (BreathTimer >= GetCurrentBreathInterval())
	{
		BreathTimer = 0.f;
		PulseBreath();
	}
}

void UColdBreathVFXComponent::EnableBreathEffect()
{
	if (bBreathActive) return;
	bBreathActive = true;

	CreateBreathComponent();
}

void UColdBreathVFXComponent::DisableBreathEffect()
{
	if (!bBreathActive) return;
	bBreathActive = false;

	if (BreathComp)
	{
		BreathComp->Deactivate();
	}
}

void UColdBreathVFXComponent::SetBreathIntensity(float Intensity)
{
	BreathIntensity = FMath::Clamp(Intensity, 0.f, 1.f);

	if (BreathComp)
	{
		BreathComp->SetFloatParameter(FName("Intensity"), BreathIntensity);
	}

	if (BreathIntensity <= 0.f && bBreathActive)
	{
		DisableBreathEffect();
	}
}

void UColdBreathVFXComponent::SetExertion(bool bExertingNow)
{
	bExerting = bExertingNow;
}

void UColdBreathVFXComponent::PulseBreath()
{
	if (!BreathComp) return;

	// Each "breath" is a short burst activation
	BreathComp->SetFloatParameter(FName("Intensity"), BreathIntensity);
	BreathComp->SetFloatParameter(FName("PuffScale"), bExerting ? 1.5f : 1.f);
	BreathComp->Activate(true);
}

float UColdBreathVFXComponent::GetCurrentBreathInterval() const
{
	return bExerting ? ExertionBreathInterval : BreathCycleInterval;
}
