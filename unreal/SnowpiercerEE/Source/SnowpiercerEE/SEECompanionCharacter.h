#pragma once

#include "CoreMinimal.h"
#include "SEENPCCharacter.h"
#include "SEECharacterAnimInstance.h"
#include "SEECompanionCharacter.generated.h"

UENUM(BlueprintType)
enum class ESEECompanionBehavior : uint8
{
	Follow			UMETA(DisplayName = "Follow"),
	Hold			UMETA(DisplayName = "Hold Position"),
	Aggressive		UMETA(DisplayName = "Aggressive"),
	Defensive		UMETA(DisplayName = "Defensive"),
	Passive			UMETA(DisplayName = "Passive")
};

/**
 * Companion character that follows and assists the player.
 * Create a Blueprint extending this class with a companion skeletal mesh
 * and NPC Animation Blueprint.
 */
UCLASS()
class SNOWPIERCEREE_API ASEECompanionCharacter : public ASEENPCCharacter
{
	GENERATED_BODY()

public:
	ASEECompanionCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// --- Companion Commands ---

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void SetBehavior(ESEECompanionBehavior NewBehavior);

	UFUNCTION(BlueprintPure, Category = "Companion")
	ESEECompanionBehavior GetBehavior() const { return CurrentBehavior; }

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void CommandFollow();

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void CommandHold();

	UFUNCTION(BlueprintCallable, Category = "Companion")
	void CommandAttack(AActor* Target);

	// --- Hit Reactions ---

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ApplyHitReaction(ESEEHitReactionType ReactionType, FVector HitDirection = FVector::ZeroVector);

	UFUNCTION(BlueprintCallable, Category = "HitReaction")
	void ActivateDeathRagdoll();

	// --- Weapon Socket ---

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponSocketName = TEXT("weapon_r");

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AttachToWeaponSocket(AActor* ActorToAttach);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	ESEECompanionBehavior CurrentBehavior = ESEECompanionBehavior::Follow;

	/** Distance at which companion tries to stay from player when following */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	float FollowDistance = 250.0f;

	/** Distance at which companion starts running to catch up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	float CatchUpDistance = 600.0f;

	/** Distance at which companion teleports to player (too far) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	float TeleportDistance = 3000.0f;

	/** Combat engagement range when in Aggressive mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Companion")
	float AggressiveEngageRange = 800.0f;

	void UpdateFollow(float DeltaTime);
	void UpdateCompanionCombat(float DeltaTime);

private:
	UPROPERTY()
	TObjectPtr<AActor> CommandedTarget;
};
