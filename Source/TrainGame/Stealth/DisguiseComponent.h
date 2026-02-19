// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TrainGame/Stealth/StealthTypes.h"
#include "DisguiseComponent.generated.h"

// ============================================================================
// UDisguiseComponent
//
// Attached to the player character. Manages the current disguise, its
// degradation over time, and interaction with NPC challenge checks.
// Disguises let the player pass through zone checkpoints and blend into
// crowds by wearing zone-appropriate attire.
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDisguiseChanged, const FDisguiseData&, OldDisguise, const FDisguiseData&, NewDisguise);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisguiseDegraded, float, NewDegradation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDisguiseBlown);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChallengeInitiated, AActor*, Challenger);

UCLASS(ClassGroup=(Stealth), meta=(BlueprintSpawnableComponent))
class TRAINGAME_API UDisguiseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDisguiseComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Disguise Management ---

	/** Equip a new disguise (takes time, can be interrupted) */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Disguise")
	bool EquipDisguise(const FDisguiseData& NewDisguise);

	/** Remove current disguise */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Disguise")
	void RemoveDisguise();

	/** Apply degradation from an event (combat, running, etc.) */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Disguise")
	void DegradeDisguise(float Amount);

	/** Mark the current disguise as blown in the current car */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Disguise")
	void BlowDisguise();

	// --- Challenge System ---

	/** Resolve an NPC challenge. Returns true if the player passes. */
	UFUNCTION(BlueprintCallable, Category = "Stealth|Disguise")
	bool ResolveChallenge(AActor* Challenger);

	// --- Queries ---

	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	bool HasDisguise() const { return CurrentDisguise.Quality != EDisguiseQuality::None; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	const FDisguiseData& GetCurrentDisguise() const { return CurrentDisguise; }

	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	EDisguiseQuality GetEffectiveQuality() const { return CurrentDisguise.GetEffectiveQuality(); }

	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	bool IsDisguiseBlown() const { return bDisguiseBlown; }

	/** Get the detection rate modifier based on current disguise vs zone */
	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	float GetDetectionModifier(ETrainZone CurrentZone) const;

	/** Check if the current disguise matches a zone */
	UFUNCTION(BlueprintPure, Category = "Stealth|Disguise")
	bool MatchesZone(ETrainZone Zone) const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Disguise")
	FOnDisguiseChanged OnDisguiseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Disguise")
	FOnDisguiseDegraded OnDisguiseDegraded;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Disguise")
	FOnDisguiseBlown OnDisguiseBlown;

	UPROPERTY(BlueprintAssignable, Category = "Stealth|Disguise")
	FOnChallengeInitiated OnChallengeInitiated;

protected:
	virtual void BeginPlay() override;

	/** Time to equip a disguise from a looted NPC */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	float EquipTime = 5.f;

	/** Time before first challenge in a new zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	float FirstChallengeDelay = 60.f;

	/** Time between subsequent challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	float ChallengeInterval = 120.f;

	/** Number of passed challenges before an NPC stops challenging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	int32 MaxChallengesPerNPC = 3;

	/** Passive degradation rate (per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Disguise")
	float PassiveDegradationRate = 0.0033f; // ~1% per 5 minutes

private:
	void UpdatePassiveDegradation(float DeltaTime);
	void UpdateChallengeTimer(float DeltaTime);

	FDisguiseData CurrentDisguise;
	bool bDisguiseBlown = false;

	float ChallengeTimer = 0.f;
	bool bFirstChallengeIssued = false;
	float TimeInCurrentZone = 0.f;

	/** NPCs who have seen the player's real face (persistent per zone visit) */
	UPROPERTY()
	TArray<AActor*> NPCsWhoKnowFace;

	/** NPCs who have been passed in challenges (tracks count per NPC) */
	TMap<AActor*, int32> ChallengePassCount;
};
