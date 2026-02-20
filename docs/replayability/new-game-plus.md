# New Game+ Design

## Overview

New Game+ (NG+) unlocks after completing any of the six endings. Players restart from Car 1 with carryover benefits while facing harder challenges and exclusive new content. NG+ is designed to reward mastery and encourage exploration of missed paths without trivializing the early game.

---

## Carryover Systems

### What Carries Over

| Category | Details | Balance Notes |
|----------|---------|---------------|
| **Character Level** | Retained (soft cap remains 30, hard cap 35) | XP requirements increase 25% per NG+ cycle |
| **Stat Points** | All allocated stat points retained | Permanent stat items do NOT carry over |
| **Skill Trees** | All unlocked skill nodes retained | New NG+-exclusive Tier 4 nodes available (see below) |
| **Perk Slots** | All 6 slots retained and unlocked from start | Quest-locked perks must be re-earned |
| **Crafting Recipes** | All learned standard recipes (62) retained | Blueprint-required recipes reset (must find blueprints again) |
| **Blueprints Found** | Tracked as "known" — blueprint locations change in NG+ | New blueprint locations randomized across zones |
| **Collectible Progress** | Journal pages, Memory Fragments tracked in Codex | Physical items not in inventory; discovery status preserved |
| **Completion Percentage** | Cumulative across playthroughs | See completion-tracking.md |
| **Hidden Ledger Knowledge** | Previous Ledger scores visible as "echoes" in UI | Does not affect current playthrough's Ledger |

### What Resets

| Category | Reason |
|----------|--------|
| **Inventory** | All items, weapons, armor, resources cleared — prevents trivializing Zone 1 economy |
| **Companion Roster** | All companions must be re-recruited — their stories reset with variations |
| **Faction Reputation** | All factions reset to Neutral — NG+ adds new reputation events |
| **Quest Progress** | All quests reset — moral choices are independent per playthrough |
| **Permanent Stat Items** | Must be found again (3–5 per playthrough) — locations shuffle in NG+ |
| **Disguises** | All disguise items reset — Tailor's Station recipes retained |
| **Legendary Weapons** | Must be re-acquired — one new legendary per NG+ cycle (see below) |
| **Map/Exploration** | Fog of war returns — car layouts may have minor variations |

### Partial Carryover

| Category | Details |
|----------|---------|
| **Scrap Stipend** | Start with 200 scrap (enough for early crafting, not enough to bypass Zone 1 economy) |
| **Starter Kit** | One T2 weapon of player's choice from a curated list of 4 (one per weapon category: blunt, blade, piercing, ranged) |
| **Codex Entries** | All lore entries, NPC biographies, and zone histories unlocked — viewable from start |
| **Background Perk** | Retained from original character creation; cannot be changed |

---

## NG+ Changes: Harder Enemies

### Combat Scaling

| Modifier | NG+ Cycle 1 | NG+ Cycle 2 | NG+ Cycle 3+ |
|----------|-------------|-------------|---------------|
| **Enemy HP** | +25% | +50% | +75% |
| **Enemy Damage** | +15% | +30% | +40% |
| **Enemy Detection Range** | +10% | +20% | +25% |
| **Enemy Aggression** | More frequent patrols | Patrol routes randomized | Patrols respond to noise across adjacent cars |
| **Boss Phases** | +1 additional phase | +1 phase, new attack patterns | +1 phase, environmental hazards active during boss |

### New Enemy Types (NG+ Exclusive)

- **Jackboot Elites**: Appear in Zone 1 (normally only Zone 2+). Armored, carry T3 weapons, immune to basic takedowns. Require combat skill investment or creative stealth.
- **Order Inquisitors**: Roam Zones 2–4 (normally Zone 6 only). CHA-contest interrogations when disguised. Carry Kronole detectors.
- **Wilford's Watchers**: Surveillance drones in Zones 3–5. Must be disabled via Electronics skill or EMP. Alert all enemies in the car if they spot the player.
- **The Thawed** (NG+ Cycle 2+): Former Drawer prisoners released by Wilford as expendable shock troops. Kronole-enhanced, erratic movement patterns, high damage but low HP.

### Stealth Difficulty

- Vent access points reduced by 1 per car (minimum 1 remaining)
- Disguise detection rate increased by one tier (Perfect → Good, Good → Partial)
- NPC memory persistence increased: blown covers carry across adjacent cars (normally per-car)
- Under-car traversal gains cold exposure penalty (+20% cold accumulation rate)

---

## NG+ Changes: New Dialogue

### Wilford's Awareness

Wilford's dialogue in Zone 7 references the player's previous playthrough choices:
- "You've been here before, haven't you? I can see it in your eyes — the weight of knowing what comes next."
- References to specific previous moral choices: "Last time you chose mercy for Beaumont. Will you be so generous again?"
- If the player makes the same ending choice: "Interesting. You'd choose the same path, knowing where it leads."
- If the player chooses differently: "So you've reconsidered. Growth, or cowardice?"

### Companion Variations

Each companion gains 2–3 new dialogue lines acknowledging a sense of déjà vu:
- Edgar: "I feel like I've fought alongside someone like you before. Can't shake it."
- ATLAS: "My memory banks contain fragmented data from a previous iteration. Corrupted, but... familiar."
- Companions who died in the previous playthrough have a single cryptic line foreshadowing their potential death: "Promise me something — if it comes down to it, don't let me die in the dark."

### NPC Echoes

- 6 NPCs across the train (one per zone) deliver "echo" dialogue — brief, unsettling lines that reference previous playthrough events without breaking the fourth wall.
- Example (Blind Yuri, Zone 1): "I hear two sets of footsteps when you walk. One now, one from before."
- Echo dialogue only triggers once per NPC per NG+ cycle.

---

## NG+ Exclusive Content

### Tier 4 Skill Nodes (NG+ Only)

Each skill tree gains one Tier 4 capstone node, requiring all 5 Tier 3 nodes in that tree:

| Tree | T4 Node | Effect |
|------|---------|--------|
| **Combat** | *Juggernaut* | After killing 3 enemies in 10 seconds, gain 5 seconds of damage immunity. Cooldown: 60 seconds. |
| **Stealth** | *Specter* | Become fully invisible for 3 seconds after a stealth takedown. Cooldown: 45 seconds. |
| **Survival** | *Phoenix Protocol* | Upon receiving lethal damage, restore to 25% HP and gain 10 seconds of +50% damage resistance. Once per zone. |
| **Leadership** | *Revolution's Voice* | All companion abilities recharge 50% faster. Party size increased to 3 companions. |
| **Engineer** | *Architect* | Unlock the ability to permanently modify one environmental feature per car (seal doors, reroute steam, disable systems). Changes persist for the rest of the playthrough. |

### New Legendary Weapon (NG+ Cycle 1)

- **The Conductor's Baton** (T5 Blunt/Electric hybrid): 60 damage, electric stun on every 3rd hit, 15% chance to disable mechanical enemies permanently. Found in a new hidden room in Car 52 (Working Spine), accessible only in NG+.

### New Legendary Weapon (NG+ Cycle 2)

- **Frostbite** (T5 Ranged): Fires condensed cold-air projectiles. 50 damage, applies Chilled status (stamina regen -20% for 15s), headshots apply Freezing (movement -50% for 5s). Ammo crafted at Chemistry Station from Chemicals + Engine Parts. Found in a sealed exterior compartment on Car 88 (requires Breachman perk + NG+ Cycle 2).

### The Seventh Revolution (NG+ Questline)

A 5-quest chain available only in NG+, woven into the main story:

1. **Echoes of the Fallen** (Zone 1): Discover evidence of a fifth revolution that was erased from all records. Starter: a scratched symbol under a bunk in Car 3 that wasn't there in the first playthrough.
2. **The Erased Names** (Zone 2): Track down survivors who remember the Fifth Revolution. One is a Third Class worker hiding in plain sight; another is in the Drawers.
3. **Wilford's Countermeasure** (Zone 4): Find the weapon Wilford built specifically to prevent future revolutions — a nerve agent dispersal system hidden in the ventilation network.
4. **The Architect's Journal** (Zone 5): Discover the identity of the Fifth Revolution's leader — someone still alive and in a position of power. The identity changes based on which ending the player chose in their first playthrough.
5. **The Seventh Spark** (Zone 6): Confront or ally with the Fifth Revolution's leader. Outcome feeds into a new variant of the Zone 7 Wilford conversation, unlocking a unique dialogue branch about cycles of revolution and whether breaking them is possible.

Completing this questline unlocks the **Eternal Vigilance** perk: +10% XP from all sources, and all NPC echo dialogue becomes permanent (persists through future NG+ cycles).

---

## NG+ Cycle Progression

| Cycle | Unlock |
|-------|--------|
| **NG+ 1** | T4 skill nodes, The Conductor's Baton, Seventh Revolution questline, enemy scaling tier 1, new dialogue |
| **NG+ 2** | Frostbite legendary, The Thawed enemy type, enemy scaling tier 2, additional echo NPCs (12 total) |
| **NG+ 3+** | Enemy scaling tier 3 (caps here), cosmetic variations (snow accumulation on exterior cars increases each cycle, visible interior wear), bragging-rights title system |

### Title System (NG+ 3+)

Earned titles displayed on the pause menu and post-game statistics screen:

| Title | Requirement |
|-------|-------------|
| *Eternal Passenger* | Complete NG+ Cycle 3 |
| *The Cycle Breaker* | Achieve all 6 endings across multiple playthroughs |
| *Ghost of the Engine* | Complete any NG+ cycle with zero kills |
| *The Last Revolutionary* | Complete NG+ with all 12 companions surviving |
| *Wilford's Equal* | Trigger all 24 unique Wilford dialogue responses across playthroughs |
| *True Conductor* | 100% completion percentage |

---

## Design Principles

1. **Reward, don't trivialize.** Carryover makes the player stronger but not invincible. Enemy scaling and new threats maintain tension.
2. **New reasons to replay.** Exclusive content (questline, weapons, dialogue) gives NG+ its own identity rather than being "the same game but harder."
3. **Respect previous choices.** Echo dialogue and Wilford's awareness honor what the player did before without invalidating it.
4. **Cap the grind.** Enemy scaling caps at NG+ Cycle 3. Beyond that, the game is about completion and mastery, not infinite difficulty.
5. **Preserve moral weight.** The Hidden Ledger resets each cycle. Previous Ledger scores are visible as context but don't influence outcomes. Each playthrough's choices stand on their own.
