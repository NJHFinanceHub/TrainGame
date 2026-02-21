// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.
// SEEVFXSubsystem.h - World subsystem managing global VFX state

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VFXTypes.h"
#include "SEEVFXSubsystem.generated.h"

class USEEWeatherVFXComponent;

/**
 * USEEVFXSubsystem
 *
 * World subsystem coordinating global VFX state across all environment
 * VFX components. Manages weather transitions, zone-based lighting
 * profiles, and VFX quality scaling.
 *
 * Acts as the single source of truth for current weather so all window
 * components stay in sync.
 */
UCLASS()
class SNOWPIERCEREE_API USEEVFXSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Global Weather ---

	UFUNCTION(BlueprintCallable, Category = "VFX|Global")
	void SetGlobalWeather(ESEEWeatherType Weather);

	UFUNCTION(BlueprintPure, Category = "VFX|Global")
	ESEEWeatherType GetGlobalWeather() const { return GlobalWeather; }

	UFUNCTION(BlueprintCallable, Category = "VFX|Global")
	void SetGlobalWindParameters(FVector Direction, float Speed);

	// --- VFX Quality Scaling ---

	// Scale factor for particle counts (0.5 = half, 1.0 = normal, 1.5 = ultra)
	UFUNCTION(BlueprintCallable, Category = "VFX|Quality")
	void SetParticleDensityScale(float Scale);

	UFUNCTION(BlueprintPure, Category = "VFX|Quality")
	float GetParticleDensityScale() const { return ParticleDensityScale; }

	// Maximum concurrent VFX emitters (budget from performance-budgets.md: 10-20)
	UFUNCTION(BlueprintCallable, Category = "VFX|Quality")
	void SetMaxConcurrentEmitters(int32 Max);

	UFUNCTION(BlueprintPure, Category = "VFX|Quality")
	int32 GetMaxConcurrentEmitters() const { return MaxConcurrentEmitters; }

	// --- Component Registration ---

	void RegisterWeatherComponent(USEEWeatherVFXComponent* Component);
	void UnregisterWeatherComponent(USEEWeatherVFXComponent* Component);

private:
	ESEEWeatherType GlobalWeather = ESEEWeatherType::Blizzard;
	FVector GlobalWindDirection = FVector(-1.0f, 0.3f, -0.1f);
	float GlobalWindSpeed = 1.0f;
	float ParticleDensityScale = 1.0f;
	int32 MaxConcurrentEmitters = 15;

	/** Registered weather components (not UPROPERTY - TArray<TWeakObjectPtr> unsupported by UHT) */
	TArray<TWeakObjectPtr<USEEWeatherVFXComponent>> RegisteredWeatherComponents;

	void BroadcastWeatherToComponents();
};
