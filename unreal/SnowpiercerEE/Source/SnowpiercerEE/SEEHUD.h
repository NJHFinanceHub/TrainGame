#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SEEHUD.generated.h"

class USEEHealthComponent;
class USEEHungerComponent;
class USEEColdComponent;
class USEECombatComponent;
class ASEECharacter;

UCLASS()
class SNOWPIERCEREE_API ASEEHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASEEHUD();

	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	void DrawHealthBar();
	void DrawStaminaBar();
	void DrawHungerIndicator();
	void DrawColdIndicator();
	void DrawWeaponIndicator();
	void DrawInteractionPrompt();
	void DrawDetectionIndicator();
	void DrawCrosshair();
	void DrawDamageDirection();

	void DrawBar(float X, float Y, float Width, float Height, float Percent,
				 FLinearColor FillColor, FLinearColor BackColor);

	// Cached references
	UPROPERTY()
	TWeakObjectPtr<ASEECharacter> PlayerCharacter;

	UPROPERTY()
	TWeakObjectPtr<USEEHealthComponent> HealthComp;

	UPROPERTY()
	TWeakObjectPtr<USEEHungerComponent> HungerComp;

	UPROPERTY()
	TWeakObjectPtr<USEEColdComponent> ColdComp;

	UPROPERTY()
	TWeakObjectPtr<USEECombatComponent> CombatComp;

	// Damage direction tracking
	float LastDamageTime = 0.0f;
	FVector LastDamageDirection = FVector::ZeroVector;
};
