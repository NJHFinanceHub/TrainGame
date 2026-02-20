// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEVFXComponent.cpp - Character-attached VFX: cold breath, damage feedback, Kronole distortion

#include "SEEVFXComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PostProcessComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "SEEHealthComponent.h"
#include "SEEColdComponent.h"
#include "SEECombatComponent.h"
#include "Survival/KronoleComponent.h"

USEEVFXComponent::USEEVFXComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;
}

void USEEVFXComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Cache sibling component references
	HealthComp = Owner->FindComponentByClass<USEEHealthComponent>();
	ColdComp = Owner->FindComponentByClass<USEEColdComponent>();
	CombatComp = Owner->FindComponentByClass<USEECombatComponent>();
	KronoleComp = Owner->FindComponentByClass<UKronoleComponent>();

	BindDelegates();

	// Create Kronole post-process component (player only)
	ACharacter* AsCharacter = Cast<ACharacter>(Owner);
	if (AsCharacter && AsCharacter->IsPlayerControlled() && KronolePostProcessMaterial.IsValid())
	{
		KronolePostProcess = NewObject<UPostProcessComponent>(Owner);
		KronolePostProcess->bUnbound = false;
		KronolePostProcess->BlendWeight = 0.0f;
		KronolePostProcess->Priority = 10.0f;
		KronolePostProcess->RegisterComponent();
		KronolePostProcess->AttachToComponent(Owner->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		UMaterialInterface* BaseMat = KronolePostProcessMaterial.LoadSynchronous();
		if (BaseMat)
		{
			KronoleMID = UMaterialInstanceDynamic::Create(BaseMat, this);
			KronolePostProcess->AddOrUpdateBlendable(KronoleMID, 0.0f);
		}
	}
}

void USEEVFXComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ColdBreathNiagara)
	{
		ColdBreathNiagara->DestroyComponent();
		ColdBreathNiagara = nullptr;
	}

	if (KronolePostProcess)
	{
		KronolePostProcess->DestroyComponent();
		KronolePostProcess = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void USEEVFXComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateColdBreath(DeltaTime);
	UpdateKronoleVFX(DeltaTime);
	UpdateLowHealthOverlay(DeltaTime);
}

void USEEVFXComponent::BindDelegates()
{
	if (HealthComp)
	{
		HealthComp->OnDamageTaken.AddDynamic(this, &USEEVFXComponent::OnDamageTaken);
		HealthComp->OnHealthChanged.AddDynamic(this, &USEEVFXComponent::OnHealthChanged);
	}

	if (ColdComp)
	{
		ColdComp->OnFrostbiteStageChanged.AddDynamic(this, &USEEVFXComponent::OnFrostbiteStageChanged);
	}

	if (CombatComp)
	{
		CombatComp->OnCombatStateChanged.AddDynamic(this, &USEEVFXComponent::OnCombatStateChanged);
		CombatComp->OnParrySuccess.AddDynamic(this, &USEEVFXComponent::OnParrySuccess);
		CombatComp->OnBlockBroken.AddDynamic(this, &USEEVFXComponent::OnBlockBroken);
	}
}

// --- Delegate Handlers ---

void USEEVFXComponent::OnDamageTaken(float Damage, ESEEDamageType DamageType, AActor* Instigator)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector HitLocation = Owner->GetActorLocation();
	FVector HitNormal = FVector::UpVector;

	// Derive hit direction from instigator if available
	if (Instigator)
	{
		FVector ToInstigator = (Instigator->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();
		HitNormal = -ToInstigator;
		HitLocation = Owner->GetActorLocation() + ToInstigator * 50.0f;
	}

	switch (DamageType)
	{
	case ESEEDamageType::Blunt:
	case ESEEDamageType::Bladed:
	case ESEEDamageType::Piercing:
		SpawnHitEffect(ESEEHitVFXType::BloodSplatter, HitLocation, HitNormal);
		if (Damage > 15.0f)
		{
			SpawnBloodDecal(HitLocation, FVector::DownVector, FMath::Clamp(Damage / 30.0f, 0.5f, 2.0f));
		}
		break;
	case ESEEDamageType::Electric:
		SpawnHitEffect(ESEEHitVFXType::MetalSparks, HitLocation, HitNormal);
		break;
	case ESEEDamageType::Fire:
		// Fire damage uses embers/smoke — spawned by environment, not character VFX
		break;
	default:
		break;
	}
}

void USEEVFXComponent::OnFrostbiteStageChanged(ESEEFrostbiteStage NewStage)
{
	// Cold breath intensifies with frostbite progression
	if (ColdBreathNiagara)
	{
		float Density = 1.0f;
		switch (NewStage)
		{
		case ESEEFrostbiteStage::None:     Density = 0.5f; break;
		case ESEEFrostbiteStage::Shivers:  Density = 1.0f; break;
		case ESEEFrostbiteStage::Numbness: Density = 1.5f; break;
		case ESEEFrostbiteStage::Blackout: Density = 2.0f; break;
		}
		ColdBreathNiagara->SetFloatParameter(FName("BreathDensity"), Density);
	}
}

void USEEVFXComponent::OnCombatStateChanged(ESEECombatState NewState)
{
	// Combat state changes affect breath rate
	// Actual breath rate update happens in UpdateColdBreath
}

void USEEVFXComponent::OnParrySuccess()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UNiagaraSystem* ParryFX = ParryFlashSystem.LoadSynchronous();
	if (ParryFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), ParryFX,
			Owner->GetActorLocation() + Owner->GetActorForwardVector() * 80.0f,
			Owner->GetActorRotation(),
			FVector::OneVector, true, true, ENCPoolMethod::AutoRelease);
	}
}

void USEEVFXComponent::OnBlockBroken()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	UNiagaraSystem* BlockFX = BlockBreakSystem.LoadSynchronous();
	if (BlockFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), BlockFX,
			Owner->GetActorLocation() + Owner->GetActorForwardVector() * 60.0f,
			Owner->GetActorRotation(),
			FVector(1.5f), true, true, ENCPoolMethod::AutoRelease);
	}
}

void USEEVFXComponent::OnHealthChanged(float NewHealthPercent)
{
	// Low-health overlay target set here, interpolated in tick
}

// --- Manual Triggers ---

void USEEVFXComponent::SpawnHitEffect(ESEEHitVFXType HitType, FVector Location, FVector Normal)
{
	UNiagaraSystem* SystemToSpawn = nullptr;
	FVector EffectScale = FVector::OneVector;

	switch (HitType)
	{
	case ESEEHitVFXType::BloodSplatter:
		SystemToSpawn = BloodSplatterSystem.LoadSynchronous();
		break;
	case ESEEHitVFXType::MetalSparks:
		SystemToSpawn = MetalSparkHitSystem.LoadSynchronous();
		break;
	case ESEEHitVFXType::Stagger:
		EffectScale = FVector(2.0f);
		SystemToSpawn = ParryFlashSystem.LoadSynchronous(); // Reuse flash
		break;
	default:
		return;
	}

	if (SystemToSpawn)
	{
		FRotator Rotation = Normal.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), SystemToSpawn,
			Location, Rotation, EffectScale,
			true, true, ENCPoolMethod::AutoRelease);
	}
}

void USEEVFXComponent::SpawnBloodDecal(FVector Location, FVector Normal, float Scale)
{
	UMaterialInterface* DecalMat = BloodDecalConfig.DecalMaterial.LoadSynchronous();
	if (!DecalMat || !GetWorld()) return;

	FRotator DecalRotation = Normal.Rotation();
	// Add random yaw rotation for variety
	DecalRotation.Yaw += FMath::RandRange(0.0f, 360.0f);

	FVector DecalSize = BloodDecalConfig.DecalSize * Scale;

	UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
		GetWorld(), DecalMat, DecalSize, Location, DecalRotation,
		BloodDecalConfig.FadeDelay + BloodDecalConfig.FadeDuration);

	if (Decal && BloodDecalConfig.FadeDelay > 0.0f)
	{
		Decal->SetFadeScreenSize(0.0f);
		Decal->SetFadeOut(BloodDecalConfig.FadeDelay, BloodDecalConfig.FadeDuration);
	}
}

void USEEVFXComponent::SetKronoleVFXStage(ESEEKronoleVFXStage NewStage)
{
	if (CurrentKronoleStage == NewStage) return;

	ESEEKronoleVFXStage OldStage = CurrentKronoleStage;
	CurrentKronoleStage = NewStage;

	switch (NewStage)
	{
	case ESEEKronoleVFXStage::None:       KronoleBlendTarget = 0.0f; break;
	case ESEEKronoleVFXStage::Onset:      KronoleBlendTarget = 0.3f; break;
	case ESEEKronoleVFXStage::Peak:       KronoleBlendTarget = 1.0f; break;
	case ESEEKronoleVFXStage::Comedown:   KronoleBlendTarget = 0.15f; break;
	case ESEEKronoleVFXStage::Withdrawal: KronoleBlendTarget = 0.6f; break;
	}

	OnKronoleVFXStageChanged.Broadcast(OldStage, NewStage);
}

// --- Tick Updates ---

void USEEVFXComponent::UpdateColdBreath(float DeltaTime)
{
	if (!ColdComp) return;

	float Temperature = ColdComp->GetTemperature();
	bool bShouldShow = Temperature < ColdBreathThreshold;

	if (bShouldShow && !ColdBreathNiagara)
	{
		UNiagaraSystem* BreathFX = ColdBreathSystem.LoadSynchronous();
		if (BreathFX)
		{
			AActor* Owner = GetOwner();
			ColdBreathNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
				BreathFX, Owner->GetRootComponent(), FName("head"),
				FVector(30.0f, 0.0f, 0.0f), FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget, false,
				true, ENCPoolMethod::None);
		}
	}
	else if (!bShouldShow && ColdBreathNiagara)
	{
		ColdBreathNiagara->Deactivate();
		ColdBreathNiagara->DestroyComponent();
		ColdBreathNiagara = nullptr;
	}

	if (ColdBreathNiagara)
	{
		// Scale breath rate based on exertion
		bool bExerted = false;
		if (CombatComp)
		{
			bExerted = CombatComp->IsInCombat();
		}
		float Rate = bExerted ? ExertedBreathRate : BaseBreathRate;
		ColdBreathNiagara->SetFloatParameter(FName("BreathRate"), Rate);

		// Scale opacity with cold intensity
		float ColdIntensity = FMath::GetMappedRangeValueClamped(
			FVector2D(ColdBreathThreshold, 10.0f),
			FVector2D(0.3f, 1.0f), Temperature);
		ColdBreathNiagara->SetFloatParameter(FName("BreathOpacity"), ColdIntensity);
	}
}

void USEEVFXComponent::UpdateKronoleVFX(float DeltaTime)
{
	// Auto-detect Kronole state from KronoleComponent
	if (KronoleComp)
	{
		if (KronoleComp->IsInWithdrawal())
		{
			SetKronoleVFXStage(ESEEKronoleVFXStage::Withdrawal);
		}
		else if (KronoleComp->IsUnderEffect())
		{
			float Remaining = KronoleComp->GetRemainingBuffDuration();
			if (Remaining > 5.0f)
			{
				float TimeDilation = KronoleComp->GetTimeDilationFactor();
				SetKronoleVFXStage(TimeDilation < 0.7f ?
					ESEEKronoleVFXStage::Peak : ESEEKronoleVFXStage::Onset);
			}
			else
			{
				SetKronoleVFXStage(ESEEKronoleVFXStage::Comedown);
			}
		}
		else
		{
			SetKronoleVFXStage(ESEEKronoleVFXStage::None);
		}
	}

	// Smooth blend
	KronoleBlendCurrent = FMath::FInterpTo(KronoleBlendCurrent, KronoleBlendTarget, DeltaTime, 3.0f);

	if (KronolePostProcess && KronoleMID)
	{
		KronolePostProcess->BlendWeight = KronoleBlendCurrent;

		KronoleMID->SetScalarParameterValue(FName("DistortionIntensity"),
			KronoleBlendCurrent * KronolePeakDistortion);
		KronoleMID->SetScalarParameterValue(FName("ChromaticAberration"),
			KronoleBlendCurrent * KronoleChromaticAberration);

		// Withdrawal gets desaturation and jitter
		bool bWithdrawal = (CurrentKronoleStage == ESEEKronoleVFXStage::Withdrawal);
		KronoleMID->SetScalarParameterValue(FName("Desaturation"),
			bWithdrawal ? KronoleBlendCurrent * 0.4f : 0.0f);
		KronoleMID->SetScalarParameterValue(FName("ScreenJitter"),
			bWithdrawal ? WithdrawalShakeIntensity * KronoleBlendCurrent : 0.0f);
	}
}

void USEEVFXComponent::UpdateLowHealthOverlay(float DeltaTime)
{
	if (!HealthComp) return;

	float HealthPercent = HealthComp->GetHealthPercent();
	float TargetIntensity = 0.0f;

	if (HealthPercent < LowHealthOverlayThreshold)
	{
		TargetIntensity = FMath::GetMappedRangeValueClamped(
			FVector2D(0.0f, LowHealthOverlayThreshold),
			FVector2D(1.0f, 0.0f), HealthPercent);
	}

	LowHealthOverlayIntensity = FMath::FInterpTo(
		LowHealthOverlayIntensity, TargetIntensity, DeltaTime, 5.0f);

	// Feed to Kronole post-process material (shared PP for screen effects)
	if (KronoleMID)
	{
		KronoleMID->SetScalarParameterValue(FName("DamageVignetteIntensity"), LowHealthOverlayIntensity);
	}
}
