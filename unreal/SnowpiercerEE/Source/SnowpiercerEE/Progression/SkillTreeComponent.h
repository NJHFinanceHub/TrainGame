#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SkillTreeTypes.h"
#include "SkillTreeComponent.generated.h"

class USEEStatsComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillNodeUnlocked, ESEESkillTree, Tree, FName, NodeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerkActivated, FName, PerkID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerkDeactivated, FName, PerkID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTreePointsChanged, int32, AvailablePoints);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SNOWPIERCEREE_API USEESkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USEESkillTreeComponent();

	virtual void BeginPlay() override;

	// --- Skill Tree API ---

	// Attempt to unlock a skill node (spends tree points)
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	bool UnlockNode(FName NodeID);

	// Check if a node is unlocked
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	bool IsNodeUnlocked(FName NodeID) const;

	// Check if a node can be unlocked (prerequisites met, enough points, level requirement)
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	bool CanUnlockNode(FName NodeID) const;

	// Get all nodes for a specific tree
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	TArray<FSEESkillNode> GetNodesForTree(ESEESkillTree Tree) const;

	// Get count of unlocked nodes in a tree
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	int32 GetUnlockedNodeCount(ESEESkillTree Tree) const;

	// Get available tree points
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	int32 GetAvailableTreePoints() const { return AvailableTreePoints; }

	// Award tree points (called on level up or quest reward)
	UFUNCTION(BlueprintCallable, Category = "Progression|SkillTree")
	void AddTreePoints(int32 Amount);

	// --- Perk API ---

	// Activate a perk (if requirements met)
	UFUNCTION(BlueprintCallable, Category = "Progression|Perks")
	bool ActivatePerk(FName PerkID);

	// Deactivate a perk
	UFUNCTION(BlueprintCallable, Category = "Progression|Perks")
	void DeactivatePerk(FName PerkID);

	// Check if a perk is active
	UFUNCTION(BlueprintPure, Category = "Progression|Perks")
	bool IsPerkActive(FName PerkID) const;

	// Check if a perk can be activated
	UFUNCTION(BlueprintPure, Category = "Progression|Perks")
	bool CanActivatePerk(FName PerkID) const;

	// Get all available perks (requirements met)
	UFUNCTION(BlueprintCallable, Category = "Progression|Perks")
	TArray<FSEEPerk> GetAvailablePerks() const;

	// Get all active perks
	UFUNCTION(BlueprintCallable, Category = "Progression|Perks")
	TArray<FSEEPerk> GetActivePerks() const;

	// Maximum number of perks that can be active simultaneously
	UFUNCTION(BlueprintPure, Category = "Progression|Perks")
	int32 GetMaxActivePerks() const;

	// --- Effect Queries ---

	// Check if a specific ability/tag has been unlocked via skill tree
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	bool HasUnlockedAbility(FName AbilityTag) const;

	// Get total bonus for a specific effect type across all unlocked nodes and active perks
	UFUNCTION(BlueprintPure, Category = "Progression|SkillTree")
	float GetTotalEffectBonus(ESkillNodeEffect EffectType, FName OptionalStat = NAME_None) const;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnSkillNodeUnlocked OnSkillNodeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPerkActivated OnPerkActivated;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnPerkDeactivated OnPerkDeactivated;

	UPROPERTY(BlueprintAssignable, Category = "Progression")
	FOnTreePointsChanged OnTreePointsChanged;

protected:
	// DataTable containing all skill nodes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Data")
	TObjectPtr<UDataTable> SkillNodeTable;

	// DataTable containing all perks
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Data")
	TObjectPtr<UDataTable> PerkTable;

	// Points available for spending on skill tree nodes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 AvailableTreePoints = 0;

	// Tree points awarded per level up
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression")
	int32 TreePointsPerLevel = 1;

	// Max simultaneous perks scales with level: base + 1 per PerkSlotInterval levels
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Perks")
	int32 BasePerkSlots = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progression|Perks")
	int32 PerkSlotInterval = 10;

	// Reference to stats component for level checks
	UPROPERTY()
	TWeakObjectPtr<USEEStatsComponent> StatsComponent;

private:
	// Runtime state
	UPROPERTY()
	TSet<FName> UnlockedNodes;

	UPROPERTY()
	TSet<FName> ActivePerks;

	// Cache loaded data
	TArray<FSEESkillNode> CachedNodes;
	TArray<FSEEPerk> CachedPerks;

	void LoadDataTables();
	void InitializeDefaultData();
	const FSEESkillNode* FindNode(FName NodeID) const;
	const FSEEPerk* FindPerk(FName PerkID) const;
	void ApplyNodeEffect(const FSEESkillNode& Node);
	void ApplyPerkEffect(const FSEEPerk& Perk, bool bApply);

	void OnLevelUp(int32 NewLevel, int32 SkillPointsAvailable);
};
