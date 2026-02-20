# Leveling & Skill Trees

## XP System

### Level Range
- **Level 1** at game start
- **Level 30** soft cap (reachable by endgame on a thorough playthrough)
- **Level 35** hard cap (requires near-100% completion)

### XP Curve
Exponential with diminishing returns. Each level requires 15% more XP than the previous.

| Level Range | XP Per Level | Typical Zone |
|---|---|---|
| 1-5 | 100-175 | Zone 1 (Tail) |
| 6-10 | 200-350 | Zone 2 (Third Class) |
| 11-15 | 400-700 | Zone 3 (Second Class) |
| 16-20 | 800-1400 | Zone 4-5 (Spine/First Class) |
| 21-25 | 1600-2800 | Zone 5-6 (First Class/Sanctum) |
| 26-30 | 3200-5600 | Zone 6-7 (Sanctum/Engine) |

### XP Sources

| Source | XP Range | Notes |
|---|---|---|
| **Quest completion** | 50-500 | Primary XP source. Major quests at top end. |
| **Combat encounters** | 10-80 | Per encounter, not per kill. Encourages finishing fights, not grinding. |
| **Discovery** | 5-30 | Finding hidden areas, lore items, secret passages. |
| **Skill checks passed** | 10-25 | Bonus for using stats to solve problems. |
| **Moral choices** | 0 | Choices grant NO XP — prevents gaming the morality system. |
| **Companion quests** | 30-150 | Per companion loyalty milestone. |
| **Car completion** | 20-50 | Bonus for clearing all content in a car (not required). |

**No grinding.** XP from repeated combat in already-cleared areas drops to 0. The train is finite — you can't farm it.

### Level-Up Rewards
Each level grants:
- **2 stat points** (freely allocated, max +1 per stat per level)
- **1 skill point** (spent in skill trees below)
- At levels **5, 10, 15, 20, 25, 30**: **1 perk slot** (see perks.md)

## Skill Trees

Five branches. Each has 15 nodes arranged in 3 tiers (5 nodes per tier). Higher tiers unlock at skill point thresholds in that tree.

### Combat

**Tier 1** (0 points in tree required):
| Skill | Cost | Effect |
|---|---|---|
| **Heavy Hitter** | 1 | Charged melee attacks deal +25% damage |
| **Quick Recovery** | 1 | Stamina regeneration +20% during combat |
| **Weapon Maintenance** | 1 | Weapon durability loss reduced by 30% |
| **Counter-Strike** | 2 | Perfect block triggers automatic riposte for 150% damage |
| **Adrenaline** | 1 | First hit in combat deals +40% damage |

**Tier 2** (4 points in tree required):
| Skill | Cost | Effect |
|---|---|---|
| **Combo Master** | 2 | Unlock 4-hit combo chains (base is 3) |
| **Iron Skin** | 2 | Damage reduction +10% from all sources |
| **Environmental Fighter** | 2 | Environmental kill/interaction damage +50%, activation range increased |
| **Disarm** | 3 | Chance to knock weapon from enemy hands on heavy attacks (15% base, scales with STR) |
| **Berserker** | 2 | Below 30% HP: +25% damage, +15% attack speed |

**Tier 3** (8 points in tree required):
| Skill | Cost | Effect |
|---|---|---|
| **Executioner** | 3 | Instant kill finisher available on staggered enemies below 20% HP |
| **Whirlwind** | 3 | AOE melee attack hitting all adjacent enemies (high stamina cost) |
| **Unbreakable** | 3 | Once per combat: survive a lethal hit with 1 HP, gain 3-second invulnerability |
| **Weapon Mastery** | 3 | Choose a weapon type — permanent +20% damage and -50% durability loss with that type |
| **War Cry** | 2 | Shout that staggers nearby enemies and boosts companion damage by 20% for 10 seconds |

### Stealth

**Tier 1** (0 points):
| Skill | Cost | Effect |
|---|---|---|
| **Soft Steps** | 1 | Movement noise reduced 30% while crouching |
| **Shadow Blend** | 1 | Stationary in shadows: detection range against you halved |
| **Pickpocket** | 2 | Steal items from NPC inventories (success based on CUN vs PER) |
| **Distraction** | 1 | Throw objects to create noise lures |
| **Quick Hide** | 1 | Enter hiding spots 50% faster |

**Tier 2** (4 points):
| Skill | Cost | Effect |
|---|---|---|
| **Silent Takedown** | 2 | Non-lethal takedown from behind. No noise. Target unconscious for 60 seconds |
| **Vent Crawler** | 2 | Move through ventilation shafts at normal crouch speed (default: crawl speed) |
| **Locksmith** | 2 | Lockpicking difficulty reduced by one tier. Lock minigame slowed. |
| **Sixth Sense** | 3 | Brief warning flash before being detected (0.5 second grace period) |
| **Corpse Cleanup** | 2 | Drag unconscious/dead bodies. Bodies hidden in containers won't be discovered. |

**Tier 3** (8 points):
| Skill | Cost | Effect |
|---|---|---|
| **Ghost** | 3 | If you enter and leave an area without being detected, bonus XP (+50% for that encounter) |
| **Master Thief** | 3 | Pickpocket chance +30%. Can steal equipped weapons. |
| **Assassin** | 3 | Silent lethal takedowns from behind (requires weapon) |
| **Vanish** | 3 | If detected, crouch in shadow for 5 seconds to reset enemy alert to searching |
| **Phantom** | 2 | Sprint while crouched at 80% of normal crouch speed |

### Survival

**Tier 1** (0 points):
| Skill | Cost | Effect |
|---|---|---|
| **Scavenger** | 1 | +1 bonus item from all loot containers |
| **Field Medic** | 1 | Bandages and basic medicine +30% effectiveness |
| **Hardy** | 1 | Cold damage reduced by 20% |
| **Rationing** | 1 | Food items provide +25% hunger restoration |
| **Improvise** | 2 | Craft basic items (bandages, lockpicks, torches) without a workbench |

**Tier 2** (4 points):
| Skill | Cost | Effect |
|---|---|---|
| **Toxin Resistance** | 2 | Kronole withdrawal effects halved. Poison damage -40% |
| **Scrap Surgeon** | 2 | Craft medical supplies from scrap metal and cloth |
| **Thermal Mastery** | 2 | Craft insulated clothing. Cold resistance doubled. |
| **Second Wind** | 3 | Once per rest cycle: fully restore stamina on command |
| **Tracker** | 2 | See footprints and interaction traces of NPCs (past 5 minutes) |

**Tier 3** (8 points):
| Skill | Cost | Effect |
|---|---|---|
| **Cockroach** | 3 | Survive lethal environmental damage (cold, gas, fall) once per zone with 10% HP |
| **Master Crafter** | 3 | All crafted items gain a random bonus property |
| **Pharmacist** | 3 | Craft advanced medicine. Cure diseases. Synthesize Kronole antidotes. |
| **Self-Sufficient** | 2 | Passive HP regeneration active even in combat (0.5%/sec) |
| **Sixth Sense (Traps)** | 2 | All traps and environmental hazards visible through walls within 10m |

### Leadership

**Tier 1** (0 points):
| Skill | Cost | Effect |
|---|---|---|
| **Inspiring Presence** | 1 | Companion damage +10% when within 5m of you |
| **Quick Learner** | 1 | +10% XP from all sources |
| **Smooth Talker** | 2 | Unlock additional dialogue options in trade and negotiation |
| **Faction Diplomat** | 1 | Reputation losses from negative actions reduced by 25% |
| **Share Rations** | 1 | Spend food to boost companion morale by 15 points |

**Tier 2** (4 points):
| Skill | Cost | Effect |
|---|---|---|
| **Tactical Command** | 2 | Issue combat orders to companions (hold position, focus target, retreat) |
| **Double Agent** | 3 | Maintain positive reputation with opposing factions simultaneously (normally exclusive) |
| **Rabble Rouser** | 2 | Incite civilian NPCs to fight during combat encounters (CHA check) |
| **Field Promotion** | 2 | Companions gain +1 to their primary stat permanently after loyalty quest completion |
| **Interrogator** | 2 | Extract additional information from captured/surrendered enemies |

**Tier 3** (8 points):
| Skill | Cost | Effect |
|---|---|---|
| **Born Leader** | 3 | Maximum companion roster +1 (default 2 → 3) |
| **Revolution** | 3 | At maximum Tailie reputation: summon Tailie reinforcements once per zone |
| **Silver Tongue** | 3 | Persuasion/deception checks: treat CHA as 4 points higher |
| **Martyr's Inspiration** | 2 | When you drop below 25% HP, all companions gain +30% damage and damage resistance |
| **United Front** | 3 | Companions no longer suffer morale loss from moral choices they disagree with |

### Engineer

**Tier 1** (0 points):
| Skill | Cost | Effect |
|---|---|---|
| **Tinker** | 1 | Repair weapons and tools at 50% scrap cost |
| **Circuit Breaker** | 1 | Interact with electrical panels to disable lights, open electronic locks |
| **Structural Analysis** | 2 | Highlight destructible walls, weak floors, and climbable surfaces |
| **Salvage** | 1 | Break down items into component materials at 75% efficiency (default 50%) |
| **Quick Fix** | 1 | Repair broken equipment mid-combat (3 second channel) |

**Tier 2** (4 points):
| Skill | Cost | Effect |
|---|---|---|
| **Trap Maker** | 2 | Craft and deploy traps: tripwires, shock plates, caltrops |
| **Override** | 2 | Hack electronic systems: cameras, automated defenses, PA system |
| **Fortify** | 2 | Barricade doors to prevent enemy flanking (holds for 30 seconds) |
| **Weapon Mod** | 3 | Add modifications to weapons: electrified, weighted, extended reach |
| **Jury-Rig Armor** | 2 | Craft improvised armor from scrap. Quality scales with CUN. |

**Tier 3** (8 points):
| Skill | Cost | Effect |
|---|---|---|
| **Master Engineer** | 3 | Access restricted engineering areas throughout the train. Unlock unique environmental solutions. |
| **EMP Device** | 3 | Craft single-use EMP grenades. Disable all electronics in a car for 30 seconds. |
| **Hull Breach** | 3 | Create controlled breaches in exterior walls. Opens new routes. Causes cold damage to area. |
| **Upgrade Station** | 2 | Build portable workbench (3 uses). Craft anywhere. |
| **Mechanical Savant** | 3 | All engineering interactions (locks, panels, traps) auto-succeed if CUN ≥ threshold |

## Respec Mechanic

**Full respec is not available.** The train doesn't offer do-overs. However:

### Partial Respec — "The Kronole Reset"
- Available from **Zone 3 onward** (the Night Car)
- Costs: 3 Kronole doses + 500 scrap + a unique quest completion
- Refunds: Up to **10 skill points** (not stat points)
- Restriction: Can only be done **twice per playthrough**
- Side effect: Causes 2-hour Kronole withdrawal afterward

### Stat Reallocation — "The Surgeon"
- Available in **Zone 6** (The Experimental Car)
- Costs: Completing a morally questionable quest
- Effect: Redistribute **up to 4 stat points** between stats
- One-time only
- Narrative consequence: Other characters react to your "change"

**Design intent:** Builds are commitments. The limited respec exists as a safety valve for players who hit walls, not as a routine optimization tool. The cost (Kronole, moral compromise) reinforces the game's themes — nothing on this train is free.
