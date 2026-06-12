#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SEEHUD.generated.h"

class ASEECharacter;
class USEEHealthComponent;
class USEEColdComponent;
class USEECombatComponent;
class UArmorComponent;

/**
 * ASEEHUD
 *
 * Canvas-drawn gameplay HUD: health/stamina bars, armor readout (damage
 * reduction + per-slot durability), cold/weapon indicators and crosshair.
 *
 * Full-screen UI (inventory, menus, death screen, ...) is owned entirely by
 * USEEUISubsystem; this class only kicks off the once-per-session main menu
 * and reports the player's death to the subsystem.
 */
UCLASS()
class SNOWPIERCEREE_API ASEEHUD : public AHUD
{
	GENERATED_BODY()

public:
	ASEEHUD();

	virtual void DrawHUD() override;

	/** Legacy dialogue hooks (UMG dialogue was removed; Slate dialogue is driven
	 *  by the dialogue manager directly). Kept as no-ops for API compatibility. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowDialogue();

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideDialogue();

	/** Routes to USEEUISubsystem::NotifyPlayerDeath. */
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowDeathScreen(const FText& CauseOfDeath);

protected:
	virtual void BeginPlay() override;

private:
	/** Dynamic-delegate target for the pawn's USEEHealthComponent::OnDeath. */
	UFUNCTION()
	void HandleOwnerDeath();

	/** (Re)cache pawn component pointers and bind the death delegate. */
	void CachePawnComponents();

	// Canvas drawing
	void DrawHealthBar();
	void DrawStaminaBar();
	void DrawArmorIndicator();
	void DrawColdIndicator();
	void DrawWeaponIndicator();
	void DrawCrosshair();
	void DrawDamageDirection();
	void DrawNPCLabels();

	void DrawBar(float X, float Y, float Width, float Height, float Percent,
				 FLinearColor FillColor, FLinearColor BackColor);

	// Cached references
	TWeakObjectPtr<ASEECharacter> PlayerCharacter;
	TWeakObjectPtr<USEEHealthComponent> HealthComp;
	TWeakObjectPtr<USEEColdComponent> ColdComp;
	TWeakObjectPtr<USEECombatComponent> CombatComp;
	TWeakObjectPtr<UArmorComponent> ArmorComp;

	// Damage direction tracking
	float LastDamageTime = 0.0f;
	FVector LastDamageDirection = FVector::ZeroVector;
};
