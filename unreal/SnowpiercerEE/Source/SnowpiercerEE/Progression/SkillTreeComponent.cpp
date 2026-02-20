#include "SkillTreeComponent.h"
#include "SEEStatsComponent.h"

USEESkillTreeComponent::USEESkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USEESkillTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	StatsComponent = GetOwner()->FindComponentByClass<USEEStatsComponent>();

	if (StatsComponent.IsValid())
	{
		StatsComponent->OnLevelUp.AddDynamic(this, &USEESkillTreeComponent::OnLevelUp);
	}

	LoadDataTables();
}

void USEESkillTreeComponent::LoadDataTables()
{
	CachedNodes.Empty();
	CachedPerks.Empty();

	if (SkillNodeTable)
	{
		TArray<FSEESkillNode*> Rows;
		SkillNodeTable->GetAllRows<FSEESkillNode>(TEXT("SkillTreeComponent"), Rows);
		for (const FSEESkillNode* Row : Rows)
		{
			CachedNodes.Add(*Row);
		}
	}

	if (PerkTable)
	{
		TArray<FSEEPerk*> Rows;
		PerkTable->GetAllRows<FSEEPerk>(TEXT("SkillTreeComponent"), Rows);
		for (const FSEEPerk* Row : Rows)
		{
			CachedPerks.Add(*Row);
		}
	}

	// If no DataTable assigned, use hardcoded defaults
	if (CachedNodes.Num() == 0)
	{
		InitializeDefaultData();
	}
}

void USEESkillTreeComponent::InitializeDefaultData()
{
	// ===== COMBAT TREE (10 nodes) =====
	auto AddNode = [this](FName ID, ESEESkillTree Tree, const FString& Name, const FString& Desc,
		ESkillNodeTier Tier, int32 Cost, int32 Level, ESkillNodeEffect Effect, float Value,
		FName Stat, FName Unlock, FVector2D Pos, TArray<FName> Prereqs = {})
	{
		FSEESkillNode Node;
		Node.NodeID = ID;
		Node.Tree = Tree;
		Node.DisplayName = FText::FromString(Name);
		Node.Description = FText::FromString(Desc);
		Node.Tier = Tier;
		Node.PointCost = Cost;
		Node.RequiredLevel = Level;
		Node.EffectType = Effect;
		Node.EffectValue = Value;
		Node.AffectedStat = Stat;
		Node.UnlockTag = Unlock;
		Node.TreePosition = Pos;
		Node.Prerequisites = Prereqs;
		CachedNodes.Add(Node);
	};

	// Combat Tree
	AddNode("CMB_HardHitter", ESEESkillTree::Combat, "Hard Hitter",
		"Melee damage increased by 15%",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::DamageMultiplier, 0.15f,
		"MeleeDamage", NAME_None, FVector2D(0.5f, 0.1f));

	AddNode("CMB_QuickStrike", ESEESkillTree::Combat, "Quick Strike",
		"Light attack speed increased by 20%",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.2f,
		"AttackSpeed", NAME_None, FVector2D(0.3f, 0.1f));

	AddNode("CMB_IronGuard", ESEESkillTree::Combat, "Iron Guard",
		"Block stamina cost reduced by 25%",
		ESkillNodeTier::Tier1, 1, 2, ESkillNodeEffect::StaminaReduction, 0.25f,
		"BlockCost", NAME_None, FVector2D(0.7f, 0.1f));

	AddNode("CMB_CounterStrike", ESEESkillTree::Combat, "Counter Strike",
		"Perfect parries deal 50% damage back",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::NewAbility, 0.5f,
		NAME_None, "Ability_CounterStrike", FVector2D(0.5f, 0.3f),
		{"CMB_HardHitter"});

	AddNode("CMB_WeaponMaster", ESEESkillTree::Combat, "Weapon Master",
		"+1 Strength permanently",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::StatBonus, 1.0f,
		"Strength", NAME_None, FVector2D(0.3f, 0.3f),
		{"CMB_QuickStrike"});

	AddNode("CMB_ShieldBreaker", ESEESkillTree::Combat, "Shield Breaker",
		"Heavy attacks break through blocks",
		ESkillNodeTier::Tier2, 2, 8, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_ShieldBreaker", FVector2D(0.7f, 0.3f),
		{"CMB_IronGuard"});

	AddNode("CMB_BerserkerRage", ESEESkillTree::Combat, "Berserker Rage",
		"Below 30% health: +40% damage, -20% defense",
		ESkillNodeTier::Tier3, 3, 12, ESkillNodeEffect::PassiveEffect, 0.4f,
		"BerserkerDamage", NAME_None, FVector2D(0.4f, 0.55f),
		{"CMB_CounterStrike", "CMB_WeaponMaster"});

	AddNode("CMB_DualWield", ESEESkillTree::Combat, "Dual Wield",
		"Equip weapons in both hands",
		ESkillNodeTier::Tier3, 3, 15, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_DualWield", FVector2D(0.6f, 0.55f),
		{"CMB_CounterStrike", "CMB_ShieldBreaker"});

	AddNode("CMB_LethalPrecision", ESEESkillTree::Combat, "Lethal Precision",
		"Critical hit chance +15%, critical damage +50%",
		ESkillNodeTier::Tier4, 4, 20, ESkillNodeEffect::DamageMultiplier, 0.15f,
		"CritChance", NAME_None, FVector2D(0.3f, 0.8f),
		{"CMB_BerserkerRage"});

	AddNode("CMB_Unstoppable", ESEESkillTree::Combat, "Unstoppable",
		"Cannot be staggered while attacking. +2 Strength.",
		ESkillNodeTier::Tier4, 4, 25, ESkillNodeEffect::StatBonus, 2.0f,
		"Strength", "Ability_Unstoppable", FVector2D(0.7f, 0.8f),
		{"CMB_DualWield"});

	// ===== STEALTH TREE (10 nodes) =====
	AddNode("STL_SoftStep", ESEESkillTree::Stealth, "Soft Step",
		"Footstep noise reduced by 30%",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.3f,
		"NoiseReduction", NAME_None, FVector2D(0.5f, 0.1f));

	AddNode("STL_ShadowMeld", ESEESkillTree::Stealth, "Shadow Meld",
		"Crouch movement speed +25% in dark areas",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::MovementBonus, 0.25f,
		"CrouchSpeed", NAME_None, FVector2D(0.3f, 0.1f));

	AddNode("STL_Pickpocket", ESEESkillTree::Stealth, "Pickpocket",
		"Steal items from unaware NPCs",
		ESkillNodeTier::Tier1, 1, 2, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_Pickpocket", FVector2D(0.7f, 0.1f));

	AddNode("STL_DisguiseMaster", ESEESkillTree::Stealth, "Disguise Master",
		"Disguise detection time doubled",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::PassiveEffect, 2.0f,
		"DisguiseDuration", NAME_None, FVector2D(0.5f, 0.3f),
		{"STL_SoftStep"});

	AddNode("STL_Locksmith", ESEESkillTree::Stealth, "Locksmith",
		"Pick locks silently, +1 Cunning",
		ESkillNodeTier::Tier2, 2, 6, ESkillNodeEffect::StatBonus, 1.0f,
		"Cunning", "Ability_SilentLockpick", FVector2D(0.7f, 0.3f),
		{"STL_Pickpocket"});

	AddNode("STL_VentRunner", ESEESkillTree::Stealth, "Vent Runner",
		"Move through vents 50% faster, no noise",
		ESkillNodeTier::Tier2, 2, 8, ESkillNodeEffect::MovementBonus, 0.5f,
		"VentSpeed", NAME_None, FVector2D(0.3f, 0.3f),
		{"STL_ShadowMeld"});

	AddNode("STL_SilentTakedown", ESEESkillTree::Stealth, "Silent Takedown",
		"Non-lethal takedowns produce no noise",
		ESkillNodeTier::Tier3, 3, 12, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_SilentTakedown", FVector2D(0.5f, 0.55f),
		{"STL_DisguiseMaster"});

	AddNode("STL_GhostWalk", ESEESkillTree::Stealth, "Ghost Walk",
		"Invisible for 5 seconds after entering stealth from hidden spot",
		ESkillNodeTier::Tier3, 3, 15, ESkillNodeEffect::NewAbility, 5.0f,
		NAME_None, "Ability_GhostWalk", FVector2D(0.3f, 0.55f),
		{"STL_VentRunner"});

	AddNode("STL_MasterOfShadows", ESEESkillTree::Stealth, "Master of Shadows",
		"+2 Cunning, +2 Agility. Detection radius halved.",
		ESkillNodeTier::Tier4, 4, 20, ESkillNodeEffect::StatBonus, 2.0f,
		"Cunning", NAME_None, FVector2D(0.4f, 0.8f),
		{"STL_SilentTakedown", "STL_GhostWalk"});

	AddNode("STL_Phantom", ESEESkillTree::Stealth, "Phantom",
		"Bodies you hide are never found. Perfect disguise in owned faction gear.",
		ESkillNodeTier::Tier4, 4, 25, ESkillNodeEffect::PassiveEffect, 1.0f,
		"PerfectStealth", NAME_None, FVector2D(0.6f, 0.8f),
		{"STL_SilentTakedown", "STL_Locksmith"});

	// ===== SURVIVAL TREE (9 nodes) =====
	AddNode("SRV_ThickSkin", ESEESkillTree::Survival, "Thick Skin",
		"Cold resistance increased by 20%",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.2f,
		"ColdResistance", NAME_None, FVector2D(0.5f, 0.1f));

	AddNode("SRV_IronStomach", ESEESkillTree::Survival, "Iron Stomach",
		"Hunger drains 25% slower",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.25f,
		"HungerReduction", NAME_None, FVector2D(0.3f, 0.1f));

	AddNode("SRV_Scavenger", ESEESkillTree::Survival, "Scavenger",
		"Find 30% more loot from containers",
		ESkillNodeTier::Tier1, 1, 2, ESkillNodeEffect::PassiveEffect, 0.3f,
		"LootBonus", NAME_None, FVector2D(0.7f, 0.1f));

	AddNode("SRV_FieldMedic", ESEESkillTree::Survival, "Field Medic",
		"Healing items 40% more effective",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::PassiveEffect, 0.4f,
		"HealingBonus", NAME_None, FVector2D(0.3f, 0.3f),
		{"SRV_IronStomach"});

	AddNode("SRV_Endurance", ESEESkillTree::Survival, "Endurance Training",
		"+15 max stamina, +1 Endurance",
		ESkillNodeTier::Tier2, 2, 6, ESkillNodeEffect::StatBonus, 1.0f,
		"Endurance", NAME_None, FVector2D(0.5f, 0.3f),
		{"SRV_ThickSkin"});

	AddNode("SRV_Climber", ESEESkillTree::Survival, "Trained Climber",
		"Climb speed +40%, unlock pipe climbing",
		ESkillNodeTier::Tier2, 2, 8, ESkillNodeEffect::MovementBonus, 0.4f,
		"ClimbSpeed", "Ability_PipeClimb", FVector2D(0.7f, 0.3f),
		{"SRV_Scavenger"});

	AddNode("SRV_Swimmer", ESEESkillTree::Survival, "Strong Swimmer",
		"Swim speed +50%, underwater breath duration doubled",
		ESkillNodeTier::Tier3, 3, 12, ESkillNodeEffect::MovementBonus, 0.5f,
		"SwimSpeed", "Ability_ExtendedSwim", FVector2D(0.7f, 0.55f),
		{"SRV_Climber"});

	AddNode("SRV_ArcticExplorer", ESEESkillTree::Survival, "Arctic Explorer",
		"Halve cold damage. Exterior traversal speed +30%.",
		ESkillNodeTier::Tier3, 3, 15, ESkillNodeEffect::PassiveEffect, 0.5f,
		"ColdDamageReduction", NAME_None, FVector2D(0.4f, 0.55f),
		{"SRV_Endurance", "SRV_FieldMedic"});

	AddNode("SRV_Unkillable", ESEESkillTree::Survival, "Unkillable",
		"Once per encounter: survive lethal damage with 1 HP. +2 Endurance.",
		ESkillNodeTier::Tier4, 4, 22, ESkillNodeEffect::StatBonus, 2.0f,
		"Endurance", "Ability_LastStand", FVector2D(0.5f, 0.8f),
		{"SRV_ArcticExplorer"});

	// ===== LEADERSHIP TREE (9 nodes) =====
	AddNode("LDR_Diplomat", ESEESkillTree::Leadership, "Diplomat",
		"New dialogue options with faction leaders",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::SocialBonus, 1.0f,
		"DiplomacyOptions", NAME_None, FVector2D(0.5f, 0.1f));

	AddNode("LDR_Inspiring", ESEESkillTree::Leadership, "Inspiring Presence",
		"Companion morale +15% in combat",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.15f,
		"CompanionMorale", NAME_None, FVector2D(0.3f, 0.1f));

	AddNode("LDR_Barter", ESEESkillTree::Leadership, "Silver Tongue",
		"Trade prices improved by 15%",
		ESkillNodeTier::Tier1, 1, 2, ESkillNodeEffect::SocialBonus, 0.15f,
		"TradeBonus", NAME_None, FVector2D(0.7f, 0.1f));

	AddNode("LDR_Recruiter", ESEESkillTree::Leadership, "Recruiter",
		"Unlock additional companion recruitment dialogue",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::SocialBonus, 1.0f,
		NAME_None, "Ability_ExtraRecruit", FVector2D(0.3f, 0.3f),
		{"LDR_Inspiring"});

	AddNode("LDR_Negotiator", ESEESkillTree::Leadership, "Master Negotiator",
		"+1 Charisma, quest reward bonuses +25%",
		ESkillNodeTier::Tier2, 2, 6, ESkillNodeEffect::StatBonus, 1.0f,
		"Charisma", NAME_None, FVector2D(0.5f, 0.3f),
		{"LDR_Diplomat"});

	AddNode("LDR_BlackMarket", ESEESkillTree::Leadership, "Black Market Contact",
		"Access rare items at faction merchants",
		ESkillNodeTier::Tier2, 2, 8, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_BlackMarket", FVector2D(0.7f, 0.3f),
		{"LDR_Barter"});

	AddNode("LDR_Commander", ESEESkillTree::Leadership, "Commander",
		"Companions gain +20% damage and +10% defense",
		ESkillNodeTier::Tier3, 3, 14, ESkillNodeEffect::PassiveEffect, 0.2f,
		"CompanionDamage", NAME_None, FVector2D(0.4f, 0.55f),
		{"LDR_Recruiter", "LDR_Negotiator"});

	AddNode("LDR_FactionLeader", ESEESkillTree::Leadership, "Faction Leader",
		"Reputation gains doubled with all factions",
		ESkillNodeTier::Tier3, 3, 16, ESkillNodeEffect::SocialBonus, 2.0f,
		"ReputationMultiplier", NAME_None, FVector2D(0.6f, 0.55f),
		{"LDR_Negotiator", "LDR_BlackMarket"});

	AddNode("LDR_Vanguard", ESEESkillTree::Leadership, "Vanguard of the Engine",
		"+2 Charisma. All companions gain unique elite ability.",
		ESkillNodeTier::Tier4, 4, 22, ESkillNodeEffect::StatBonus, 2.0f,
		"Charisma", "Ability_EliteCompanions", FVector2D(0.5f, 0.8f),
		{"LDR_Commander", "LDR_FactionLeader"});

	// ===== ENGINEER TREE (10 nodes) =====
	AddNode("ENG_Tinkerer", ESEESkillTree::Engineer, "Tinkerer",
		"Crafting speed +25%",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.25f,
		"CraftSpeed", NAME_None, FVector2D(0.5f, 0.1f));

	AddNode("ENG_Scrapper", ESEESkillTree::Engineer, "Scrapper",
		"Salvage 50% more materials from disassembly",
		ESkillNodeTier::Tier1, 1, 1, ESkillNodeEffect::PassiveEffect, 0.5f,
		"SalvageBonus", NAME_None, FVector2D(0.3f, 0.1f));

	AddNode("ENG_Electrician", ESEESkillTree::Engineer, "Electrician",
		"Hack electrical panels and junction boxes",
		ESkillNodeTier::Tier1, 1, 2, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_HackPanels", FVector2D(0.7f, 0.1f));

	AddNode("ENG_Armorer", ESEESkillTree::Engineer, "Armorer",
		"Craft improved armor at any station",
		ESkillNodeTier::Tier2, 2, 5, ESkillNodeEffect::CraftingUnlock, 1.0f,
		NAME_None, "Recipe_ImprovedArmor", FVector2D(0.3f, 0.3f),
		{"ENG_Scrapper"});

	AddNode("ENG_Weaponsmith", ESEESkillTree::Engineer, "Weaponsmith",
		"Weapon degradation 30% slower, craft Military tier weapons",
		ESkillNodeTier::Tier2, 2, 6, ESkillNodeEffect::CraftingUnlock, 0.3f,
		"WeaponDurability", "Recipe_MilitaryWeapons", FVector2D(0.5f, 0.3f),
		{"ENG_Tinkerer"});

	AddNode("ENG_Demolitions", ESEESkillTree::Engineer, "Demolitions Expert",
		"Craft and use explosives, +1 Perception",
		ESkillNodeTier::Tier2, 2, 8, ESkillNodeEffect::StatBonus, 1.0f,
		"Perception", "Recipe_Explosives", FVector2D(0.7f, 0.3f),
		{"ENG_Electrician"});

	AddNode("ENG_TrainMechanic", ESEESkillTree::Engineer, "Train Mechanic",
		"Repair train systems, open maintenance hatches",
		ESkillNodeTier::Tier3, 3, 12, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_TrainRepair", FVector2D(0.4f, 0.55f),
		{"ENG_Armorer", "ENG_Weaponsmith"});

	AddNode("ENG_ChemistrySet", ESEESkillTree::Engineer, "Chemistry Set",
		"Craft medicines, poisons, and Kronole derivatives",
		ESkillNodeTier::Tier3, 3, 15, ESkillNodeEffect::CraftingUnlock, 1.0f,
		NAME_None, "Recipe_Chemistry", FVector2D(0.6f, 0.55f),
		{"ENG_Weaponsmith", "ENG_Demolitions"});

	AddNode("ENG_MasterEngineer", ESEESkillTree::Engineer, "Master Engineer",
		"+2 Perception. Craft Specialized tier equipment.",
		ESkillNodeTier::Tier4, 4, 20, ESkillNodeEffect::StatBonus, 2.0f,
		"Perception", "Recipe_Specialized", FVector2D(0.3f, 0.8f),
		{"ENG_TrainMechanic"});

	AddNode("ENG_EngineKnowledge", ESEESkillTree::Engineer, "Engine Knowledge",
		"Understand the Eternal Engine. Unlock Engineer ending path.",
		ESkillNodeTier::Tier4, 4, 25, ESkillNodeEffect::NewAbility, 1.0f,
		NAME_None, "Ability_EngineKnowledge", FVector2D(0.7f, 0.8f),
		{"ENG_ChemistrySet"});

	// ===== PERKS =====
	auto AddPerk = [this](FName ID, const FString& Name, const FString& Desc,
		int32 Level, ESEESkillTree Tree, int32 TreeNodes, EPerkTier Tier,
		ESkillNodeEffect Effect, float Value, FName Stat, FName Group = NAME_None)
	{
		FSEEPerk Perk;
		Perk.PerkID = ID;
		Perk.DisplayName = FText::FromString(Name);
		Perk.Description = FText::FromString(Desc);
		Perk.RequiredLevel = Level;
		Perk.RequiredTree = Tree;
		Perk.RequiredTreeNodes = TreeNodes;
		Perk.PerkTier = Tier;
		Perk.EffectType = Effect;
		Perk.EffectValue = Value;
		Perk.AffectedStat = Stat;
		Perk.ExclusionGroup = Group;
		CachedPerks.Add(Perk);
	};

	// Level 5 perks (Minor)
	AddPerk("PRK_Resilient", "Resilient", "Take 10% less physical damage",
		5, ESEESkillTree::Combat, 1, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.1f, "PhysicalReduction");

	AddPerk("PRK_NightOwl", "Night Owl", "See better in dark areas, +10% stealth in darkness",
		5, ESEESkillTree::Stealth, 1, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.1f, "DarknessBonus");

	AddPerk("PRK_Resourceful", "Resourceful", "Crafting uses 15% fewer materials",
		5, ESEESkillTree::Engineer, 1, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.15f, "MaterialReduction");

	AddPerk("PRK_WellFed", "Well Fed", "Food provides 20% more hunger restoration",
		5, ESEESkillTree::Survival, 1, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.2f, "FoodBonus");

	// Level 10 perks (Minor)
	AddPerk("PRK_QuickRecovery", "Quick Recovery", "Stamina regenerates 25% faster",
		10, ESEESkillTree::Combat, 3, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.25f, "StaminaRegen");

	AddPerk("PRK_LightFeet", "Light Feet", "Sprint noise reduced by 50%",
		10, ESEESkillTree::Stealth, 3, EPerkTier::Minor,
		ESkillNodeEffect::PassiveEffect, 0.5f, "SprintNoiseReduction");

	// Level 15 perks (Major)
	AddPerk("PRK_Juggernaut", "Juggernaut", "Cannot be knocked down. +20% health.",
		15, ESEESkillTree::Combat, 5, EPerkTier::Major,
		ESkillNodeEffect::PassiveEffect, 0.2f, "HealthBonus", "CombatStyle");

	AddPerk("PRK_Assassin", "Assassin", "Stealth kills deal 3x damage. Silent movement.",
		15, ESEESkillTree::Stealth, 5, EPerkTier::Major,
		ESkillNodeEffect::DamageMultiplier, 3.0f, "StealthKillDamage", "CombatStyle");

	AddPerk("PRK_PackRat", "Pack Rat", "Carry weight increased by 50%",
		15, ESEESkillTree::Survival, 4, EPerkTier::Major,
		ESkillNodeEffect::PassiveEffect, 0.5f, "CarryWeight");

	AddPerk("PRK_Charismatic", "Born Leader", "All companion abilities cooldown 30% faster",
		15, ESEESkillTree::Leadership, 4, EPerkTier::Major,
		ESkillNodeEffect::PassiveEffect, 0.3f, "CompanionCooldown");

	// Level 25 perks (Elite)
	AddPerk("PRK_Warlord", "Warlord", "+30% damage, companions deal +20% damage",
		25, ESEESkillTree::Combat, 7, EPerkTier::Elite,
		ESkillNodeEffect::DamageMultiplier, 0.3f, "AllDamage", "EndgameRole");

	AddPerk("PRK_Phantom", "Living Shadow", "Nearly undetectable while crouching. Takedowns never fail.",
		25, ESEESkillTree::Stealth, 7, EPerkTier::Elite,
		ESkillNodeEffect::PassiveEffect, 0.9f, "StealthMastery", "EndgameRole");

	AddPerk("PRK_Immortal", "Immortal", "+50% max health, regenerate health slowly over time",
		25, ESEESkillTree::Survival, 6, EPerkTier::Elite,
		ESkillNodeEffect::PassiveEffect, 0.5f, "HealthRegen", "EndgameRole");

	AddPerk("PRK_Emperor", "Emperor of the Train", "All faction reputations max. All companions bonded.",
		25, ESEESkillTree::Leadership, 6, EPerkTier::Elite,
		ESkillNodeEffect::SocialBonus, 1.0f, "FactionMaxRep", "EndgameRole");

	AddPerk("PRK_Inventor", "Grand Inventor", "All crafting recipes unlocked. Craft Legendary items.",
		25, ESEESkillTree::Engineer, 7, EPerkTier::Elite,
		ESkillNodeEffect::CraftingUnlock, 1.0f, "AllRecipes", "EndgameRole");
}

// --- Core Skill Tree Logic ---

bool USEESkillTreeComponent::UnlockNode(FName NodeID)
{
	if (!CanUnlockNode(NodeID))
	{
		return false;
	}

	const FSEESkillNode* Node = FindNode(NodeID);
	if (!Node)
	{
		return false;
	}

	AvailableTreePoints -= Node->PointCost;
	UnlockedNodes.Add(NodeID);

	ApplyNodeEffect(*Node);

	OnSkillNodeUnlocked.Broadcast(Node->Tree, NodeID);
	OnTreePointsChanged.Broadcast(AvailableTreePoints);

	return true;
}

bool USEESkillTreeComponent::IsNodeUnlocked(FName NodeID) const
{
	return UnlockedNodes.Contains(NodeID);
}

bool USEESkillTreeComponent::CanUnlockNode(FName NodeID) const
{
	if (IsNodeUnlocked(NodeID))
	{
		return false;
	}

	const FSEESkillNode* Node = FindNode(NodeID);
	if (!Node)
	{
		return false;
	}

	if (AvailableTreePoints < Node->PointCost)
	{
		return false;
	}

	if (StatsComponent.IsValid() && StatsComponent->GetLevel() < Node->RequiredLevel)
	{
		return false;
	}

	for (const FName& Prereq : Node->Prerequisites)
	{
		if (!IsNodeUnlocked(Prereq))
		{
			return false;
		}
	}

	return true;
}

TArray<FSEESkillNode> USEESkillTreeComponent::GetNodesForTree(ESEESkillTree Tree) const
{
	TArray<FSEESkillNode> Result;
	for (const FSEESkillNode& Node : CachedNodes)
	{
		if (Node.Tree == Tree)
		{
			Result.Add(Node);
		}
	}
	return Result;
}

int32 USEESkillTreeComponent::GetUnlockedNodeCount(ESEESkillTree Tree) const
{
	int32 Count = 0;
	for (const FSEESkillNode& Node : CachedNodes)
	{
		if (Node.Tree == Tree && UnlockedNodes.Contains(Node.NodeID))
		{
			Count++;
		}
	}
	return Count;
}

void USEESkillTreeComponent::AddTreePoints(int32 Amount)
{
	AvailableTreePoints += Amount;
	OnTreePointsChanged.Broadcast(AvailableTreePoints);
}

// --- Perk Logic ---

bool USEESkillTreeComponent::ActivatePerk(FName PerkID)
{
	if (!CanActivatePerk(PerkID))
	{
		return false;
	}

	const FSEEPerk* Perk = FindPerk(PerkID);
	if (!Perk)
	{
		return false;
	}

	// Deactivate any perk in the same exclusion group
	if (Perk->ExclusionGroup != NAME_None)
	{
		for (const FSEEPerk& Other : CachedPerks)
		{
			if (Other.ExclusionGroup == Perk->ExclusionGroup && ActivePerks.Contains(Other.PerkID))
			{
				DeactivatePerk(Other.PerkID);
			}
		}
	}

	ActivePerks.Add(PerkID);
	ApplyPerkEffect(*Perk, true);
	OnPerkActivated.Broadcast(PerkID);

	return true;
}

void USEESkillTreeComponent::DeactivatePerk(FName PerkID)
{
	if (!ActivePerks.Contains(PerkID))
	{
		return;
	}

	const FSEEPerk* Perk = FindPerk(PerkID);
	if (Perk)
	{
		ApplyPerkEffect(*Perk, false);
	}

	ActivePerks.Remove(PerkID);
	OnPerkDeactivated.Broadcast(PerkID);
}

bool USEESkillTreeComponent::IsPerkActive(FName PerkID) const
{
	return ActivePerks.Contains(PerkID);
}

bool USEESkillTreeComponent::CanActivatePerk(FName PerkID) const
{
	if (IsPerkActive(PerkID))
	{
		return false;
	}

	const FSEEPerk* Perk = FindPerk(PerkID);
	if (!Perk)
	{
		return false;
	}

	// Level check
	if (StatsComponent.IsValid() && StatsComponent->GetLevel() < Perk->RequiredLevel)
	{
		return false;
	}

	// Tree node count check
	if (Perk->RequiredTreeNodes > 0 && GetUnlockedNodeCount(Perk->RequiredTree) < Perk->RequiredTreeNodes)
	{
		return false;
	}

	// Perk slot check (exclusion group swaps don't count as needing a new slot)
	int32 CurrentActive = ActivePerks.Num();
	if (Perk->ExclusionGroup != NAME_None)
	{
		for (const FSEEPerk& Other : CachedPerks)
		{
			if (Other.ExclusionGroup == Perk->ExclusionGroup && ActivePerks.Contains(Other.PerkID))
			{
				CurrentActive--; // Will be swapped, not added
				break;
			}
		}
	}

	if (CurrentActive >= GetMaxActivePerks())
	{
		return false;
	}

	return true;
}

TArray<FSEEPerk> USEESkillTreeComponent::GetAvailablePerks() const
{
	TArray<FSEEPerk> Result;
	for (const FSEEPerk& Perk : CachedPerks)
	{
		if (CanActivatePerk(Perk.PerkID) || IsPerkActive(Perk.PerkID))
		{
			Result.Add(Perk);
		}
	}
	return Result;
}

TArray<FSEEPerk> USEESkillTreeComponent::GetActivePerks() const
{
	TArray<FSEEPerk> Result;
	for (const FSEEPerk& Perk : CachedPerks)
	{
		if (ActivePerks.Contains(Perk.PerkID))
		{
			Result.Add(Perk);
		}
	}
	return Result;
}

int32 USEESkillTreeComponent::GetMaxActivePerks() const
{
	int32 Level = StatsComponent.IsValid() ? StatsComponent->GetLevel() : 1;
	return BasePerkSlots + (Level / PerkSlotInterval);
}

// --- Effect Queries ---

bool USEESkillTreeComponent::HasUnlockedAbility(FName AbilityTag) const
{
	for (const FSEESkillNode& Node : CachedNodes)
	{
		if (Node.UnlockTag == AbilityTag && UnlockedNodes.Contains(Node.NodeID))
		{
			return true;
		}
	}

	for (const FSEEPerk& Perk : CachedPerks)
	{
		if (Perk.AffectedStat == AbilityTag && ActivePerks.Contains(Perk.PerkID))
		{
			return true;
		}
	}

	return false;
}

float USEESkillTreeComponent::GetTotalEffectBonus(ESkillNodeEffect EffectType, FName OptionalStat) const
{
	float Total = 0.0f;

	for (const FSEESkillNode& Node : CachedNodes)
	{
		if (Node.EffectType == EffectType && UnlockedNodes.Contains(Node.NodeID))
		{
			if (OptionalStat == NAME_None || Node.AffectedStat == OptionalStat)
			{
				Total += Node.EffectValue;
			}
		}
	}

	for (const FSEEPerk& Perk : CachedPerks)
	{
		if (Perk.EffectType == EffectType && ActivePerks.Contains(Perk.PerkID))
		{
			if (OptionalStat == NAME_None || Perk.AffectedStat == OptionalStat)
			{
				Total += Perk.EffectValue;
			}
		}
	}

	return Total;
}

// --- Internal ---

const FSEESkillNode* USEESkillTreeComponent::FindNode(FName NodeID) const
{
	for (const FSEESkillNode& Node : CachedNodes)
	{
		if (Node.NodeID == NodeID)
		{
			return &Node;
		}
	}
	return nullptr;
}

const FSEEPerk* USEESkillTreeComponent::FindPerk(FName PerkID) const
{
	for (const FSEEPerk& Perk : CachedPerks)
	{
		if (Perk.PerkID == PerkID)
		{
			return &Perk;
		}
	}
	return nullptr;
}

void USEESkillTreeComponent::ApplyNodeEffect(const FSEESkillNode& Node)
{
	if (!StatsComponent.IsValid())
	{
		return;
	}

	// Apply stat bonuses directly
	if (Node.EffectType == ESkillNodeEffect::StatBonus)
	{
		if (Node.AffectedStat == "Strength")
			StatsComponent->ModifyStat(ESEEStat::Strength, FMath::RoundToInt32(Node.EffectValue));
		else if (Node.AffectedStat == "Agility")
			StatsComponent->ModifyStat(ESEEStat::Agility, FMath::RoundToInt32(Node.EffectValue));
		else if (Node.AffectedStat == "Endurance")
			StatsComponent->ModifyStat(ESEEStat::Endurance, FMath::RoundToInt32(Node.EffectValue));
		else if (Node.AffectedStat == "Cunning")
			StatsComponent->ModifyStat(ESEEStat::Cunning, FMath::RoundToInt32(Node.EffectValue));
		else if (Node.AffectedStat == "Perception")
			StatsComponent->ModifyStat(ESEEStat::Perception, FMath::RoundToInt32(Node.EffectValue));
		else if (Node.AffectedStat == "Charisma")
			StatsComponent->ModifyStat(ESEEStat::Charisma, FMath::RoundToInt32(Node.EffectValue));
	}
}

void USEESkillTreeComponent::ApplyPerkEffect(const FSEEPerk& Perk, bool bApply)
{
	if (!StatsComponent.IsValid())
	{
		return;
	}

	int32 Direction = bApply ? 1 : -1;

	if (Perk.EffectType == ESkillNodeEffect::StatBonus)
	{
		int32 Delta = FMath::RoundToInt32(Perk.EffectValue) * Direction;
		if (Perk.AffectedStat == "Strength")
			StatsComponent->ModifyStat(ESEEStat::Strength, Delta);
		else if (Perk.AffectedStat == "Agility")
			StatsComponent->ModifyStat(ESEEStat::Agility, Delta);
		else if (Perk.AffectedStat == "Endurance")
			StatsComponent->ModifyStat(ESEEStat::Endurance, Delta);
		else if (Perk.AffectedStat == "Cunning")
			StatsComponent->ModifyStat(ESEEStat::Cunning, Delta);
		else if (Perk.AffectedStat == "Perception")
			StatsComponent->ModifyStat(ESEEStat::Perception, Delta);
		else if (Perk.AffectedStat == "Charisma")
			StatsComponent->ModifyStat(ESEEStat::Charisma, Delta);
	}
}

void USEESkillTreeComponent::OnLevelUp(int32 NewLevel, int32 SkillPointsAvailable)
{
	AddTreePoints(TreePointsPerLevel);
}
