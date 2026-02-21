#pragma once

#include "CoreMinimal.h"
#include "TrainGame/AI/SEENPCCharacter.h"
#include "SEECivilianCharacter.generated.h"

UENUM(BlueprintType)
enum class ESEECivilianVariant : uint8
{
	TailieWorker		UMETA(DisplayName = "Tailie Worker"),
	TailieElder			UMETA(DisplayName = "Tailie Elder"),
	ThirdClassLaborer	UMETA(DisplayName = "Third Class Laborer"),
	ThirdClassVendor	UMETA(DisplayName = "Third Class Vendor"),
	SecondClassResident	UMETA(DisplayName = "Second Class Resident"),
	FirstClassElite		UMETA(DisplayName = "First Class Elite")
};

/**
 * Civilian NPC. Non-combatant characters that populate the train.
 * Variants cover different social classes and roles.
 * Civilians flee from combat rather than fight.
 */
UCLASS()
class SNOWPIERCEREE_API ASEECivilianCharacter : public ASEENPCCharacter
{
	GENERATED_BODY()

public:
	ASEECivilianCharacter();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
	ESEECivilianVariant CivilianVariant = ESEECivilianVariant::TailieWorker;

	/** Whether this civilian can give the player information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
	bool bIsInformant = false;

	/** Whether this civilian is essential (cannot be killed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Civilian")
	bool bIsEssential = false;

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ActivateDeathRagdoll();

protected:
	void FleeFromThreat(float DeltaTime);
};
