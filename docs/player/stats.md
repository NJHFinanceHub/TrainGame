# Stat System

## Overview

Six core stats on a **1-20 scale**. All start at 5. Stats grow through use, level-up allocation, and rare permanent items. The stat system is designed so that no single stat is dump-able — the train tests everything.

## Core Stats

### Strength (STR)

**What it represents:** Raw physical power. The ability to hit hard, break things, and endure brute-force solutions.

**Gameplay effects:**
- **Melee damage** — Each point above 5 adds +4% melee damage; below 5 subtracts 4%
- **Forced entry** — Pry open sealed doors, bend bars, break barricades (threshold checks: 8/12/16)
- **Carry capacity** — Base 20 kg + 2 kg per STR point
- **Grapple** — Determines grab/throw outcomes in CQC
- **Intimidation (physical)** — Some dialogue options require STR threshold to be credible

**Zone relevance:** Critical in Zones 1-2 (Tail/Third Class) where brute force is the primary currency. Diminishing returns in Zones 4-6 where problems become political.

### Endurance (END)

**What it represents:** Stamina, pain tolerance, resistance to the train's environmental hazards.

**Gameplay effects:**
- **Max stamina** — Base 100 + 5 per END point. Stamina governs sprinting, melee combos, and climbing
- **Cold resistance** — Each point extends survival time in hull breach/exterior sections by 8 seconds
- **Injury resistance** — Reduces severity of wounds (see injury system). At END 12+, minor injuries downgrade to scratches
- **Recovery rate** — HP regeneration out of combat: 0.5% max HP/sec base, +0.1%/sec per END above 5
- **Addiction resistance** — Higher END delays Kronole withdrawal onset by 1 hour per point above 8
- **Sprint duration** — Base 4 seconds + 0.3 seconds per END point

**Zone relevance:** Essential in Zone 1 (Car 11 Freezer Breach), Zone 4 (exterior maintenance sections), and any hull breach encounters. The train is trying to kill you with cold — END keeps you alive.

### Cunning (CUN)

**What it represents:** Technical intelligence, mechanical aptitude, problem-solving, and the ability to exploit systems.

**Gameplay effects:**
- **Lockpicking** — Determines which locks can be picked (threshold: Easy 4, Medium 8, Hard 12, Master 16)
- **Crafting quality** — Items crafted with higher CUN have better stats and durability (+3% per point)
- **Hacking** — Access electronic systems, override doors, disable cameras (Zone 3+)
- **Trap detection/disarming** — Spot and disarm environmental traps and tripwires
- **Weakness analysis** — At CUN 10+, enemy health bars show weak points. At 15+, show resistances
- **Sabotage** — Rig equipment to fail, poison supplies, set traps for enemies

**Zone relevance:** Increasingly critical from Zone 3 onward. The Working Spine (Zone 4) is essentially a CUN-check gauntlet. Also determines quality of all crafted gear throughout.

### Charisma (CHA)

**What it represents:** Force of personality, leadership ability, the power to persuade, deceive, or inspire.

**Gameplay effects:**
- **Dialogue options** — Unlocks persuasion, deception, and inspiration dialogue branches (thresholds: 6/10/14/18)
- **Companion morale** — Base companion morale modified by +3% per CHA point. Higher morale = better combat performance
- **Faction reputation gain** — +5% reputation gain per CHA point above 5
- **Barter prices** — Buy prices reduced and sell prices increased by 2% per CHA point
- **Crowd influence** — Ability to sway groups (rallying speeches, inciting mobs, calming panics). Major quest-altering moments require CHA checks
- **Recruitment** — Some companions require minimum CHA to recruit (Kez: 6, more demanding companions later)

**Zone relevance:** Rises in importance with each zone. In the Tail, fists talk. In First Class, words are the weapon. CHA 14+ unlocks the "peaceful revolution" quest branches in Zone 5-6.

### Perception (PER)

**What it represents:** Awareness, attention to detail, sensory acuity in the train's dark and noisy environment.

**Gameplay effects:**
- **Detection radius** — Spot enemies, traps, and hidden items at greater range (+1m per PER point)
- **Loot discovery** — Hidden containers and environmental loot become visible (threshold: 6/10/14 for tiers)
- **Eavesdropping** — Overhear NPC conversations at greater range, unlocking optional intel and quest hooks
- **Combat initiative** — Higher PER = faster reaction to ambushes. At PER 10+, ambushes trigger a brief slow-motion window
- **Stealth awareness** — Detect enemies in stealth/dark environments. Counter to the Dark Car (Car 10) and similar
- **Sniper proficiency** — Ranged weapon accuracy bonus: +3% per PER point

**Zone relevance:** Essential in Car 10 (Dark Car), critical for stealth builds throughout, and unlocks massive amounts of optional lore/intel content. PER is the "see more game" stat.

### Survival (SRV)

**What it represents:** Practical knowledge of staying alive. Rationing, first aid, reading danger, adapting to hostile environments.

**Gameplay effects:**
- **Food efficiency** — Each food item restores +5% more hunger per SRV point above 5
- **Medicine efficiency** — Healing items restore +5% more HP per SRV point above 5
- **Environmental awareness** — HUD warnings for temperature drops, structural instability, gas leaks appear earlier (+2 seconds per point)
- **Scavenging yield** — Loot containers give +1 bonus item at SRV 8, +2 at 14
- **Campfire crafting** — Rest spots provide better buffs with higher SRV (better sleep quality, meal preparation bonuses)
- **Wound treatment** — Can self-treat minor injuries at SRV 8, moderate at 12. Reduces need for medicine
- **Tracking** — Follow NPC trails, detect recent passage, find hidden paths through maintenance areas

**Zone relevance:** The generalist survival stat. Never critical for any single encounter but makes every zone easier. High SRV players consume fewer resources, find more loot, and have more margin for error. The "play smart" stat.

## Stat Checks

Stats are checked in two ways:

### Threshold Checks
Binary pass/fail. Your stat must meet or exceed the threshold. Used for:
- Dialogue options (visible threshold shown as [STR 12] tag)
- Environmental interactions (force doors, hack panels)
- Skill unlocks in crafting

### Contested Checks
Stat vs. stat rolls. Your stat + d20 vs. NPC stat + d20. Used for:
- Grappling (STR vs STR)
- Persuasion (CHA vs target's CUN)
- Stealth detection (PER vs target's CUN)
- Intimidation (STR or CHA vs target's END)

**The d20 roll is hidden** — the player sees the outcome but not the numbers. This maintains tension without exposing the math.

## Stat Growth

Stats increase through three mechanisms:

1. **Level-up allocation** — 2 stat points per level, freely allocated. Cap of +1 per stat per level.
2. **Use-based micro-gains** — Hidden XP per stat. Using STR in combat slowly builds toward a passive +1 bonus (every ~50 significant uses). Capped at +2 total from use.
3. **Permanent items** — Rare one-time consumables or quest rewards that grant +1 to a stat (3-5 exist per playthrough). These are permanent and stack beyond the level cap.

**Soft cap:** 15 (achievable through normal play)
**Hard cap:** 20 (requires permanent items and optimal use-based gains)

## Build Archetypes

The stat system supports distinct playstyles without enforcing classes:

| Archetype | Primary Stats | Playstyle |
|---|---|---|
| **The Enforcer** | STR/END | Front-line brawler. Breaks through obstacles. Intimidates. |
| **The Operator** | CUN/PER | Stealth/tech specialist. Bypasses encounters. Sees everything. |
| **The Leader** | CHA/SRV | Social manipulation. Strong companions. Resource efficiency. |
| **The Survivor** | END/SRV | Outlasts everything. Shrugs off cold, injury, addiction. |
| **The Ghost** | PER/CUN | Pure stealth. Avoids combat entirely where possible. |
| **The Revolutionary** | STR/CHA | Fighter who inspires others. Combat leader. |
