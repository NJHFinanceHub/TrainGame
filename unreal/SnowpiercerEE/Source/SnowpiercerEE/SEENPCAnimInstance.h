#pragma once

#include "CoreMinimal.h"
#include "SEECharacterAnimInstance.h"
#include "SEETypes.h"
#include "AI/SEENPCCharacter.h"
#include "SEENPCAnimInstance.generated.h"

/**
 * Animation instance for NPC characters.
 * Extends base anim instance with NPC-specific state (patrol, detection, alert).
 */
UCLASS()
class SNOWPIERCEREE_API USEENPCAnimInstance : public USEECharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// --- NPC State ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	ESEENPCState NPCState = ESEENPCState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	ESEENPCClass NPCClass = ESEENPCClass::Tailie;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	bool bIsPatrolling = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	bool bIsInCombat = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	bool bIsAlerted = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	float DetectionLevel = 0.0f;
};
