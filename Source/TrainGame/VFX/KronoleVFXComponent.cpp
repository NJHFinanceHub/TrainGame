// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#include "KronoleVFXComponent.h"
#include "VFXSubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"

UKronoleVFXComponent::UKronoleVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKronoleVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	CreatePostProcessComponent();

	UWorld* World = GetWorld();
	if (World)
	{
		VFXSubsystemRef = World->GetSubsystem<UVFXSubsystem>();
	}
}

void UKronoleVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PostProcessComp)
	{
		PostProcessComp->DestroyComponent();
		PostProcessComp = nullptr;
	}
	if (MoteComp)
	{
		MoteComp->DestroyComponent();
		MoteComp = nullptr;
	}
	if (SlowMoComp)
	{
		SlowMoComp->DestroyComponent();
		SlowMoComp = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void UKronoleVFXComponent::CreatePostProcessComponent()
{
	if (!GetOwner()) return;

	PostProcessComp = NewObject<UPostProcessComponent>(GetOwner());
	if (!PostProcessComp) return;

	PostProcessComp->bUnbound = true; // Affects entire view
	PostProcessComp->Priority = 10.f; // High priority to overlay on zone PP
	PostProcessComp->BlendWeight = 0.f;
	PostProcessComp->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform);
	PostProcessComp->RegisterComponent();
}

void UKronoleVFXComponent::CreateMoteComponent()
{
	if (!AmbientMoteEffect || !GetOwner() || MoteComp) return;

	MoteComp = NewObject<UNiagaraComponent>(GetOwner());
	if (!MoteComp) return;

	MoteComp->SetAsset(AmbientMoteEffect);
	MoteComp->SetAutoActivate(false);
	MoteComp->AttachToComponent(GetOwner()->GetRootComponent(),
		FAttachmentTransformRules::KeepRelativeTransform);
	MoteComp->RegisterComponent();
}

void UKronoleVFXComponent::SetIntensity(EKronoleVFXIntensity NewIntensity)
{
	if (NewIntensity == TargetIntensity) return;

	TargetIntensity = NewIntensity;
	SourceParams = CurrentParams;
	TargetParams = GetParamsForIntensity(NewIntensity);
	TransitionAlpha = 0.f;

	// Notify VFX subsystem
	if (UVFXSubsystem* VFX = VFXSubsystemRef.Get())
	{
		VFX->SetKronoleIntensity(NewIntensity);
	}

	// Manage mote particles
	if (NewIntensity == EKronoleVFXIntensity::Peak || NewIntensity == EKronoleVFXIntensity::Onset)
	{
		CreateMoteComponent();
		if (MoteComp) MoteComp->Activate(true);
	}
	else if (MoteComp)
	{
		MoteComp->Deactivate();
	}

	// Manage slow-mo lines
	if (NewIntensity == EKronoleVFXIntensity::Peak && SlowMoLinesEffect)
	{
		if (!SlowMoComp)
		{
			SlowMoComp = NewObject<UNiagaraComponent>(GetOwner());
			if (SlowMoComp)
			{
				SlowMoComp->SetAsset(SlowMoLinesEffect);
				SlowMoComp->AttachToComponent(GetOwner()->GetRootComponent(),
					FAttachmentTransformRules::KeepRelativeTransform);
				SlowMoComp->RegisterComponent();
			}
		}
		if (SlowMoComp) SlowMoComp->Activate(true);
	}
	else if (SlowMoComp)
	{
		SlowMoComp->Deactivate();
	}
}

void UKronoleVFXComponent::SetTimeDilationVisual(float InTimeDilation)
{
	TimeDilation = InTimeDilation;

	if (SlowMoComp)
	{
		SlowMoComp->SetFloatParameter(FName("TimeDilation"), InTimeDilation);
	}
}

void UKronoleVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentIntensity == TargetIntensity && TransitionAlpha >= 1.f) return;

	// Advance transition
	if (TransitionDuration > 0.f)
	{
		TransitionAlpha = FMath::Clamp(TransitionAlpha + DeltaTime / TransitionDuration, 0.f, 1.f);
	}
	else
	{
		TransitionAlpha = 1.f;
	}

	CurrentParams = LerpParams(SourceParams, TargetParams, TransitionAlpha);
	ApplyPostProcessParams(CurrentParams);

	if (TransitionAlpha >= 1.f)
	{
		CurrentIntensity = TargetIntensity;
	}
}

void UKronoleVFXComponent::ApplyPostProcessParams(const FKronolePostProcessParams& Params)
{
	if (!PostProcessComp) return;

	const bool bActive = (Params.ChromaticAberration > 0.01f ||
		Params.RadialBlur > 0.01f ||
		FMath::Abs(Params.Saturation - 1.f) > 0.01f ||
		Params.VignetteIntensity > 0.01f);

	PostProcessComp->BlendWeight = bActive ? 1.f : 0.f;

	FPostProcessSettings& Settings = PostProcessComp->Settings;

	Settings.bOverride_SceneFringeIntensity = true;
	Settings.SceneFringeIntensity = Params.ChromaticAberration;

	Settings.bOverride_ColorSaturation = true;
	Settings.ColorSaturation = FVector4(Params.Saturation, Params.Saturation, Params.Saturation, 1.f);

	Settings.bOverride_VignetteIntensity = true;
	Settings.VignetteIntensity = Params.VignetteIntensity;

	Settings.bOverride_FilmGrainIntensity = true;
	Settings.FilmGrainIntensity = Params.FilmGrain;
}

FKronolePostProcessParams UKronoleVFXComponent::GetParamsForIntensity(EKronoleVFXIntensity Intensity) const
{
	switch (Intensity)
	{
	case EKronoleVFXIntensity::None:
		return FKronolePostProcessParams();

	case EKronoleVFXIntensity::Onset:
	{
		FKronolePostProcessParams Result;
		Result.ChromaticAberration = PeakParams.ChromaticAberration * 0.3f;
		Result.RadialBlur = PeakParams.RadialBlur * 0.2f;
		Result.Saturation = FMath::Lerp(1.f, PeakParams.Saturation, 0.3f);
		Result.ColorTint = FLinearColor::LerpUsingHSV(FLinearColor::White, PeakParams.ColorTint, 0.3f);
		Result.VignetteIntensity = PeakParams.VignetteIntensity * 0.2f;
		Result.FilmGrain = PeakParams.FilmGrain * 0.1f;
		return Result;
	}

	case EKronoleVFXIntensity::Peak:
		return PeakParams;

	case EKronoleVFXIntensity::Comedown:
	{
		FKronolePostProcessParams Result;
		Result.ChromaticAberration = PeakParams.ChromaticAberration * 0.5f;
		Result.RadialBlur = PeakParams.RadialBlur * 0.1f;
		Result.Saturation = FMath::Lerp(1.f, PeakParams.Saturation, 0.5f);
		Result.VignetteIntensity = PeakParams.VignetteIntensity * 0.3f;
		Result.FilmGrain = PeakParams.FilmGrain * 0.3f;
		return Result;
	}

	case EKronoleVFXIntensity::Withdrawal:
		return WithdrawalParams;

	default:
		return FKronolePostProcessParams();
	}
}

FKronolePostProcessParams UKronoleVFXComponent::LerpParams(const FKronolePostProcessParams& A, const FKronolePostProcessParams& B, float Alpha) const
{
	FKronolePostProcessParams Result;
	Result.ChromaticAberration = FMath::Lerp(A.ChromaticAberration, B.ChromaticAberration, Alpha);
	Result.RadialBlur = FMath::Lerp(A.RadialBlur, B.RadialBlur, Alpha);
	Result.Saturation = FMath::Lerp(A.Saturation, B.Saturation, Alpha);
	Result.ColorTint = FLinearColor::LerpUsingHSV(A.ColorTint, B.ColorTint, Alpha);
	Result.VignetteIntensity = FMath::Lerp(A.VignetteIntensity, B.VignetteIntensity, Alpha);
	Result.FilmGrain = FMath::Lerp(A.FilmGrain, B.FilmGrain, Alpha);
	return Result;
}
