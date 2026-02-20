// FactionReputationComponent.h - Actor component tracking faction reputation for a character
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactionTypes.h"
#include "FactionReputationComponent.generated.h"

/**
 * UFactionReputationComponent
 *
 * Tracks the player's reputation with each of the 8 train factions.
 * Reputation values range from -100 to +100 per faction and determine:
 * - Available dialogue options and NPC willingness to interact
 * - Ally/hostile faction disposition
 * - Access to faction-controlled train cars
 * - Companion availability and loyalty
 * - Unlockable faction perks at reputation thresholds
 *
 * Reputation changes use event tags with cooldowns to prevent spam,
 * mirroring the pattern from USurvivalComponent::ApplyMoraleEvent.
 *
 * Zone 1 focus: Tailies faction is fully configured with 6 reputation
 * tiers and 4 perks. Other factions have default configurations.
 */
UCLASS(ClassGroup=(Faction), meta=(BlueprintSpawnableComponent))
class SNOWYENGINE_API UFactionReputationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFactionReputationComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// --- Reputation Access ---

	/** Get raw reputation value for a faction (-100 to 100). */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	float GetReputation(EFaction Faction) const;

	/** Get normalized reputation (0.0 to 1.0 across the faction's range). */
	UFUNCTION(BlueprintPure, Category = "Faction|Reputation")
	float GetReputationNormalized(EFaction Faction) const;

	/** Get the current reputation level/tier for a faction. */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	EReputationLevel GetReputationLevel(EFaction Faction) const;

	/** Get the faction's disposition toward the player based on reputation. */
	UFUNCTION(BlueprintPure, Category = "Faction|Reputation")
	EFactionDisposition GetDisposition(EFaction Faction) const;

	// --- Reputation Modification ---

	/** Modify reputation by delta. EventTag prevents duplicate events via cooldown. */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void ModifyReputation(EFaction Faction, float Delta, FName EventTag = NAME_None);

	/** Set reputation to an absolute value (bypasses cooldown). */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void SetReputation(EFaction Faction, float NewValue);

	/** Modify reputation with multiple factions at once (e.g., a choice helps one, hurts another). */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	void ModifyReputationMultiple(const TMap<EFaction, float>& Deltas, FName EventTag = NAME_None);

	// --- Perk System ---

	/** Check if a specific perk is currently active. */
	UFUNCTION(BlueprintPure, Category = "Faction|Perks")
	bool IsPerkActive(FName PerkTag) const;

	/** Get all currently active perks across all factions. */
	UFUNCTION(BlueprintCallable, Category = "Faction|Perks")
	TArray<FFactionPerk> GetActivePerks() const;

	/** Get all perks for a specific faction (active and inactive). */
	UFUNCTION(BlueprintCallable, Category = "Faction|Perks")
	TArray<FFactionPerk> GetFactionPerks(EFaction Faction) const;

	// --- Query ---

	/** Get the faction config (thresholds, perks, etc.). */
	UFUNCTION(BlueprintPure, Category = "Faction|Config")
	FFactionConfig GetFactionConfig(EFaction Faction) const;

	/** Check if the player meets a minimum reputation level with a faction. */
	UFUNCTION(BlueprintPure, Category = "Faction|Reputation")
	bool MeetsReputationRequirement(EFaction Faction, EReputationLevel RequiredLevel) const;

	/** Get all factions where the player has at least the given level. */
	UFUNCTION(BlueprintCallable, Category = "Faction|Reputation")
	TArray<EFaction> GetFactionsAtLevel(EReputationLevel MinLevel) const;

	// --- Serialization ---

	UFUNCTION(BlueprintCallable, Category = "Faction|Save")
	FFactionReputationSnapshot CreateSnapshot() const;

	UFUNCTION(BlueprintCallable, Category = "Faction|Save")
	void RestoreFromSnapshot(const FFactionReputationSnapshot& Snapshot);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
	FOnReputationChanged OnReputationChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
	FOnReputationLevelChanged OnReputationLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
	FOnFactionPerkChanged OnPerkUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Faction|Events")
	FOnFactionPerkChanged OnPerkLost;

protected:
	virtual void BeginPlay() override;

	// Per-faction configuration (editable in editor)
	UPROPERTY(EditAnywhere, Category = "Faction|Config")
	TMap<EFaction, FFactionConfig> FactionConfigs;

	// Current reputation values per faction
	UPROPERTY(VisibleAnywhere, Category = "Faction|Runtime")
	TMap<EFaction, float> ReputationValues;

	// Cached reputation levels for detecting threshold crossings
	TMap<EFaction, EReputationLevel> CachedLevels;

	// Currently active perk tags
	UPROPERTY(VisibleAnywhere, Category = "Faction|Runtime")
	TSet<FName> ActivePerks;

	// Event cooldown tracking (EventTag -> remaining seconds)
	TMap<FName, float> EventCooldowns;

	UPROPERTY(EditAnywhere, Category = "Faction|Config")
	float EventCooldownSeconds = 30.0f;

private:
	void InitializeDefaultConfigs();
	void InitializeTailiesConfig(FFactionConfig& Config);

	EReputationLevel CalculateLevel(EFaction Faction, float RepValue) const;
	void CheckLevelCrossings(EFaction Faction, float OldValue);
	void RefreshPerks(EFaction Faction);
	float ClampReputation(EFaction Faction, float Value) const;

	EFactionDisposition DispositionFromLevel(EReputationLevel Level) const;
};
