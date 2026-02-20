#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SkillTreeTypes.generated.h"

// The 5 skill trees
UENUM(BlueprintType)
enum class ESEESkillTree : uint8
{
	Combat		UMETA(DisplayName = "Combat"),
	Stealth		UMETA(DisplayName = "Stealth"),
	Survival	UMETA(DisplayName = "Survival"),
	Leadership	UMETA(DisplayName = "Leadership"),
	Engineer	UMETA(DisplayName = "Engineer")
};

// Node tier within a tree (determines cost and prerequisites)
UENUM(BlueprintType)
enum class ESkillNodeTier : uint8
{
	Tier1	UMETA(DisplayName = "Tier 1 - Basic"),
	Tier2	UMETA(DisplayName = "Tier 2 - Advanced"),
	Tier3	UMETA(DisplayName = "Tier 3 - Expert"),
	Tier4	UMETA(DisplayName = "Tier 4 - Master")
};

// What a skill node does when unlocked
UENUM(BlueprintType)
enum class ESkillNodeEffect : uint8
{
	StatBonus			UMETA(DisplayName = "Stat Bonus"),
	DamageMultiplier	UMETA(DisplayName = "Damage Multiplier"),
	StaminaReduction	UMETA(DisplayName = "Stamina Cost Reduction"),
	NewAbility			UMETA(DisplayName = "Unlock New Ability"),
	PassiveEffect		UMETA(DisplayName = "Passive Effect"),
	CraftingUnlock		UMETA(DisplayName = "Crafting Recipe Unlock"),
	MovementBonus		UMETA(DisplayName = "Movement Bonus"),
	SocialBonus			UMETA(DisplayName = "Social/Dialogue Bonus")
};

// Perk rarity/impact
UENUM(BlueprintType)
enum class EPerkTier : uint8
{
	Minor	UMETA(DisplayName = "Minor Perk"),
	Major	UMETA(DisplayName = "Major Perk"),
	Elite	UMETA(DisplayName = "Elite Perk")
};

// A single node in a skill tree
USTRUCT(BlueprintType)
struct FSEESkillNode : public FTableRowBase
{
	GENERATED_BODY()

	// Unique identifier for this node
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName NodeID;

	// Which tree this belongs to
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEESkillTree Tree = ESEESkillTree::Combat;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	// Description of what this node does
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Tier determines position in tree and base cost
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillNodeTier Tier = ESkillNodeTier::Tier1;

	// Skill points required to unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointCost = 1;

	// Minimum player level required
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	// Prerequisite node IDs (must be unlocked first)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Prerequisites;

	// What kind of effect this provides
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillNodeEffect EffectType = ESkillNodeEffect::StatBonus;

	// Magnitude of the effect (interpretation depends on EffectType)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue = 0.1f;

	// Optional: which stat is affected (for StatBonus type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AffectedStat;

	// Optional: tag for ability/recipe unlock
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnlockTag;

	// UI position in the tree (0-1 normalized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D TreePosition = FVector2D(0.5f, 0.0f);

	// Icon texture path
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IconPath;
};

// A perk — passive bonus unlocked at level thresholds
USTRUCT(BlueprintType)
struct FSEEPerk : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PerkID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	// Level at which this perk becomes available
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 5;

	// Optional: require nodes from a specific tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESEESkillTree RequiredTree = ESEESkillTree::Combat;

	// How many nodes in that tree must be unlocked
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredTreeNodes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPerkTier PerkTier = EPerkTier::Minor;

	// Effect type and value (same system as skill nodes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ESkillNodeEffect EffectType = ESkillNodeEffect::PassiveEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AffectedStat;

	// Mutually exclusive perk group (only one from a group can be active)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ExclusionGroup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IconPath;
};

// Runtime state for an unlocked skill node
USTRUCT(BlueprintType)
struct FSkillNodeState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName NodeID;

	UPROPERTY(BlueprintReadOnly)
	bool bUnlocked = false;
};

// Runtime state for a perk
USTRUCT(BlueprintType)
struct FPerkState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName PerkID;

	UPROPERTY(BlueprintReadOnly)
	bool bActive = false;
};
