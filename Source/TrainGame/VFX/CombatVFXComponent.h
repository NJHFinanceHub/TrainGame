// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VFXTypes.h"
#include "CombatVFXComponent.generated.h"

class UVFXSubsystem;

// ============================================================================
// UCombatVFXComponent
//
// Handles hit feedback VFX: blood splatter, impact sparks on metal,
// weapon trails, and damage decals. Attaches to combat characters and
// responds to combat events (OnHitLanded, OnBlocked, etc.).
// ============================================================================

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UCombatVFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatVFXComponent();

	/** Spawn hit impact VFX at the hit location */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void PlayHitImpact(FVector HitLocation, FVector HitNormal, EImpactSurface Surface, float DamageAmount);

	/** Spawn blood splatter from a wound */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void PlayBloodSplatter(FVector HitLocation, FVector Direction, float Intensity);

	/** Spawn block impact (shield/weapon parry sparks) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void PlayBlockImpact(FVector ContactPoint, FVector Normal);

	/** Spawn environmental kill VFX (steam scalding, electrocution, etc.) */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void PlayEnvironmentalKillVFX(FVector Location, EImpactSurface Surface);

	/** Flash red screen overlay on taking heavy damage */
	UFUNCTION(BlueprintCallable, Category = "VFX|Combat")
	void PlayDamageFlash(float DamageRatio);

protected:
	virtual void BeginPlay() override;

	/** Blood splatter Niagara system override (uses zone profile if null) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
	UNiagaraSystem* BloodSplatterOverride = nullptr;

	/** Block spark Niagara system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
	UNiagaraSystem* BlockSparkEffect = nullptr;

	/** Minimum damage ratio (damage/maxHP) to trigger screen flash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DamageFlashThreshold = 0.15f;

	/** Maximum number of active blood decals before recycling oldest */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
	int32 MaxBloodDecals = 20;

private:
	UPROPERTY()
	TWeakObjectPtr<UVFXSubsystem> VFXSubsystemRef;

	int32 ActiveBloodDecalCount = 0;
};
