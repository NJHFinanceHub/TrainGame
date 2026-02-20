# Procedural Elements

## Overview

Procedural systems add controlled randomization to prevent memorization and create emergent situations across playthroughs. These systems operate within authored constraints — the narrative and level design remain fixed, but resource placement, NPC behaviors, merchant stock, and optional challenge modes introduce meaningful variation.

All procedural elements are optional and can be disabled independently in the settings menu.

---

## 1. Loot Randomization

### Base System (Active in All Modes)

Every car has a fixed set of loot containers (crates, lockers, drawers, bodies). The contents of these containers are determined by a **per-car seed** combined with zone-specific loot tables.

#### Loot Table Structure

Each container has:
- **Category**: What type of items can spawn (resource, weapon, armor, consumable, crafting material, collectible)
- **Tier range**: Min–max tier of items (e.g., Zone 1 containers drop T1–T2; Zone 5 drops T3–T4)
- **Guaranteed items**: Some containers always contain a specific item (quest items, blueprints, legendaries) — these are NEVER randomized
- **Variable items**: 1–3 additional slots filled from the zone's loot table using the per-car seed

#### Seed Behavior

| Playthrough Type | Seed Behavior |
|------------------|---------------|
| **First playthrough** | Fixed seed — identical loot for all players on first run (ensures consistent difficulty curve) |
| **NG+** | New seed per cycle — loot placement shuffles within each car's loot table constraints |
| **Chapter Select** | Random seed — different each time |
| **Roguelike Mode** | Fully random seed with expanded loot tables (see Section 5) |

#### Loot Quality Scaling

| Factor | Effect |
|--------|--------|
| **PER stat** | +2% chance of higher-tier loot per PER point above 5 |
| **Scavenger background** | +10% chance of finding bonus resource in any container |
| **Rat's Eye perk** | Lootable containers glow — also reveals one hidden container per car that non-Scavengers miss |
| **SRV stat** | +5% bonus yield on resource-type loot per SRV point above 5 |
| **NG+ cycle** | Loot quality baseline increases by one tier every 2 NG+ cycles (e.g., Zone 1 drops T2–T3 in NG+ Cycle 2) |

#### Resource Respawn

- Respawnable containers (marked internally) regenerate contents on a timer: 2–10 real-time days depending on zone (Tail: 3 days, Spine: 5 days, First Class: 10 days)
- Respawned loot uses a new roll from the loot table — different items each time
- Non-respawnable containers (quest-related, unique) never regenerate

### Loot Randomization Constraints

To prevent game-breaking outcomes:
- **No T4+ items in Zones 1–2** regardless of seed or NG+ cycle
- **No T5 items from random loot** — legendaries are always quest/boss rewards
- **Blueprint drops are fixed locations** — they shuffle in NG+ but always appear in the same zone
- **Medicine availability scales with difficulty** — higher difficulties reduce medicine spawns by up to 30%
- **At least 1 weapon-tier-appropriate item** spawns per car to prevent progression softlocks

---

## 2. NPC Placement Variation

### Patrol Randomization

Enemy patrols (Jackboots, Order Inquisitors, faction guards) follow authored routes on first playthrough. In NG+ and procedural modes, patrol behavior varies:

#### Patrol Variation Tiers

| Tier | Active In | Effect |
|------|-----------|--------|
| **Fixed** | First playthrough | All patrols follow authored routes. Predictable. |
| **Shifted** | NG+ Cycle 1 | Patrol start positions shift by ±1 car. Routes unchanged but timing offsets vary ±10 seconds. |
| **Randomized** | NG+ Cycle 2+ | Patrol routes selected from 2–3 pre-authored variants per car. Start positions and timing randomized within bounds. |
| **Adaptive** | NG+ Cycle 3+ / Roguelike | Patrols respond to player behavior: if the player uses vents frequently, guards occasionally check vent openings; if the player uses disguises, NPC spot-check frequency increases. |

#### Civilian NPC Variation

Non-hostile NPCs (workers, merchants, civilians) have slight placement variation across playthroughs:
- Each civilian NPC has 2–3 authored positions within their assigned car
- Position selected per playthrough seed
- Dialogue and quests are unaffected — only physical placement changes
- Purpose: prevents rote memorization of "talk to the person at the left table" patterns

### Merchant Inventory Variation

Merchants refresh stock on authored timers (1–10 days per merchant type). Stock is drawn from tier-appropriate item pools with per-refresh seed variation:

| Merchant Type | Refresh Timer | Stock Pool Size | Items per Refresh |
|---------------|---------------|-----------------|-------------------|
| Scavenger | 2 days | 15 items | 4–6 |
| Workshop Vendor | 3 days | 20 items | 5–7 |
| Medic Trader | 4 days | 12 items | 3–5 |
| Kronole Dealer | 1 day | 8 items | 3–4 |
| Black Marketeer | 3 days (location moves) | 25 items | 6–8 |
| Arms Dealer | 5 days | 18 items | 4–6 |
| First Class Purveyor | 7 days | 15 items | 3–5 |
| Engine Specialist | 10 days | 10 items | 2–4 |

**Black Market special rules:**
- Location changes every 3 days, cycling through 5 pre-authored locations
- In NG+, 2 additional locations are added to the rotation
- Stock always includes at least 1 item not available from any other merchant in the current zone

---

## 3. Daily Challenge Mode

### Overview

A standalone mode accessible from the main menu after first completion. Players tackle a single zone (randomly selected each day) with a pre-built character and specific challenge modifiers. All players worldwide receive the same seed and modifiers on the same day.

### Structure

| Element | Details |
|---------|---------|
| **Duration** | 1 zone (15 cars), estimated 45–90 minutes |
| **Character** | Pre-built, fixed stats/skills/perks appropriate for the zone's level range |
| **Starting Gear** | Randomized loadout from the zone's tier range (same for all players) |
| **Objective** | Reach the zone boss and resolve the encounter (combat or non-combat) |
| **Modifiers** | 2–3 daily modifiers that change gameplay (see modifier list below) |
| **Scoring** | Points based on speed, stealth, combat efficiency, resources remaining, and optional objectives |

### Daily Modifiers

Each day, 2–3 modifiers are drawn from this pool:

| Modifier | Effect |
|----------|--------|
| **Blackout** | No lighting in any car. Player relies on torches, PER stat, and memory. |
| **Lockdown** | All doors require lockpicking or forced entry. No doors are pre-opened. |
| **Kronole Fog** | Visibility reduced to 5 meters. Enemy detection range also reduced. |
| **Rationed** | No loot respawns. Starting supplies are all you get. |
| **Hunted** | A Jackboot Elite squad spawns every 10 minutes and searches for the player. |
| **Fragile** | Player takes double damage. Encourages stealth and avoidance. |
| **Diplomatic Immunity** | No combat allowed. All encounters must be resolved socially or by avoidance. |
| **Overclocked** | Player moves 30% faster, but stamina drains 50% faster. |
| **Scavenger's Dream** | Loot quality increased by 2 tiers. But inventory is halved (3×4 grid). |
| **The Clock** | 30-minute real-time timer. Reach the boss before time expires. |
| **Companion Roulette** | A random companion joins at the start. Their loyalty is Neutral. |
| **Fog of War** | No map. Player must navigate by memory and environmental cues. |

### Scoring System

| Category | Max Points | Criteria |
|----------|------------|---------|
| **Completion** | 1000 | Reached and resolved zone boss |
| **Speed** | 500 | Time-based (faster = more points) |
| **Stealth** | 500 | Cars cleared undetected |
| **Combat Efficiency** | 300 | Kill/takedown ratio, damage taken, resources used |
| **Exploration** | 300 | Hidden items found, optional rooms accessed |
| **Optional Objectives** | 400 | 2 daily bonus objectives (e.g., "Find the hidden note in Car 22", "Complete the zone without using medicine") |
| **Total** | 3000 | |

### Leaderboard

- Global leaderboard (if online features supported)
- Personal best tracking per zone
- Weekly summary: total points across 7 daily challenges = weekly score
- Monthly top scores earn cosmetic titles

---

## 4. Weekly Challenge Mode

### Overview

A longer-form challenge mode that spans 2–3 zones with curated narrative constraints. Refreshes every Monday. More story-driven than Daily Challenges.

### Structure

| Element | Details |
|---------|---------|
| **Duration** | 2–3 zones (30–45 cars), estimated 3–6 hours |
| **Character** | Player-created (simplified creation: background + 3 stat allocations) |
| **Narrative Hook** | A unique scenario with authored context (e.g., "You are a Jackboot defector starting in Zone 3 with the Order hunting you") |
| **Companions** | 1 assigned companion with fixed loyalty and pre-set personal quest progress |
| **Objective** | Complete the scenario's specific goal (not always the zone boss) |
| **Scoring** | Points + a pass/fail assessment based on scenario-specific criteria |

### Example Weekly Scenarios

| Week | Scenario | Zones | Goal |
|------|----------|-------|------|
| 1 | *The Defector* | 3–4 | Escape from Second Class to the Spine with stolen surveillance data. Order Inquisitors hunt you. No disguises available. |
| 2 | *The Mercy Run* | 1–3 | Reach Zone 3 with zero kills and all companions alive. Start with Caretaker background. |
| 3 | *Kronole Runner* | 2–5 | Deliver 10 Kronole from Third Class to First Class without being caught. Black Market is your only ally. |
| 4 | *The Engineer's Gambit* | 4–6 | Kazakov has betrayed you. Reach the Engine with a skeleton crew and sabotaged equipment. Only Engineer tree skills available. |

### Weekly Rewards

- Completing a weekly challenge unlocks a cosmetic for the main game (character appearance variation, weapon skin, rest point decoration)
- Cosmetics are permanently unlocked — missing a week means missing that specific cosmetic until it rotates back (quarterly rotation)

---

## 5. Roguelike Mode: Eternal Engine

### Overview

A standalone permadeath mode that reimagines the full game with heavy procedural elements. Unlocks after completing the main story once. Designed for experienced players who want maximum replayability with high stakes.

### Core Differences from Standard Mode

| Feature | Standard Mode | Eternal Engine |
|---------|---------------|----------------|
| **Death** | Reload from checkpoint | Permanent. Run ends. |
| **Save System** | Checkpoint + 10 manual saves | Checkpoint only (deleted on load — true roguelike save) |
| **Car Order** | Fixed (Cars 1–103) | Fixed zone structure, but cars within each zone are shuffled |
| **Loot** | Seeded per car | Fully randomized from expanded loot tables |
| **Companions** | All 12 available | 6 randomly selected per run (2 per zone, drawn from pool of 12) |
| **Moral Choices** | Fixed 24 | 18 per run (3 per zone, drawn from pool of 24 + 6 Eternal Engine exclusives) |
| **Enemy Scaling** | Level-appropriate | +20% baseline + scales with car count cleared |
| **Crafting** | All stations available | Only 4 of 7 station types available per run (random) |
| **Run Length** | 30–40 hours | 8–15 hours (compressed zone length: 8–10 cars per zone instead of 15) |

### Car Shuffling Rules

Within each zone, cars are drawn from a pool of authored car layouts and shuffled:

| Zone | Standard Cars | Eternal Engine Cars | Pool Size |
|------|---------------|---------------------|-----------|
| Zone 1 | 15 | 8 | 12 (drawn from 15 base + 3 EE-exclusive) |
| Zone 2 | 15 | 8 | 12 (drawn from 15 base + 3 EE-exclusive) |
| Zone 3 | 18 | 10 | 14 (drawn from 18 base + 4 EE-exclusive) |
| Zone 4 | 17 | 8 | 12 (drawn from 17 base + 3 EE-exclusive) |
| Zone 5 | 20 | 10 | 14 (drawn from 20 base + 4 EE-exclusive) |
| Zone 6 | 21 | 10 | 14 (drawn from 21 base + 4 EE-exclusive) |

- Zone boss cars are always the final car of each zone (not shuffled)
- Transition cars between zones are fixed (not shuffled)
- EE-exclusive cars are new layouts designed specifically for the mode, with procedural loot and enemy placement

### Companion Roulette

- At the start of each run, 6 companions are randomly selected from the pool of 12
- 2 companions are assigned to each of the first 3 zones (Zones 1–3)
- Zones 4–6 have no new companions — the player works with whoever they've recruited
- If a companion dies, they're gone for the run
- Companion personal quests are compressed to 2 steps (from 4–5) to fit the shorter run length

### Moral Choice Drafting

- Each zone presents 3 moral choices (drawn from the full pool of 24 + 6 EE-exclusive choices)
- Choices are selected to avoid narrative contradictions (e.g., two choices requiring the same NPC won't both appear)
- EE-exclusive moral choices involve resource-vs-risk trade-offs suited to the permadeath format:

| EE Choice | Options |
|-----------|---------|
| **The Shortcut** | Take a dangerous exterior route (skip 2 cars, risk cold death) or push through normally |
| **The Stranger's Offer** | Accept mysterious medicine (+50% HP, unknown side effect that triggers later) or refuse |
| **The Locked Car** | Spend 3 lockpicks to access a sealed car (guaranteed high-tier loot) or save resources |
| **Companion's Gamble** | Let a companion attempt a solo mission (50% chance of great reward, 50% chance they die) |
| **The Kronole Vision** | Use Kronole to see enemy placements in the next 3 cars (costs 2 Kronole + withdrawal risk) |
| **The Trader's Dilemma** | A merchant offers your best weapon in exchange for all your medicine. Take the deal? |

### Run-Based Unlocks

Completing Eternal Engine runs (or reaching specific milestones) permanently unlocks content for future runs:

| Unlock | Requirement | Effect |
|--------|-------------|--------|
| **Starter Blueprints** | Complete 1 EE run | Start future runs knowing 5 basic crafting recipes |
| **Companion Memories** | Reach Zone 4 with a specific companion 3 times | That companion starts at Friendly loyalty instead of Neutral |
| **The Veteran's Kit** | Complete 3 EE runs | Start with a T2 weapon and 1 medicine |
| **Zone Mastery: Tail** | Complete Zone 1 in under 45 minutes | Zone 1 enemy density reduced by 10% in future runs |
| **Zone Mastery: Third Class** | Complete Zone 2 without any companion injuries | Zone 2 loot quality +1 tier in future runs |
| **Zone Mastery: Second Class** | Resolve Zone 3 boss non-violently | Start Zone 3 with a disguise in future runs |
| **Zone Mastery: Spine** | Complete Zone 4 without using any crafting station | Zone 4 enemy patrols have a gap in future runs |
| **Zone Mastery: First Class** | Complete Zone 5 with all companions alive | Zone 5 merchant prices -15% in future runs |
| **Zone Mastery: Sanctum** | Complete Zone 6 in under 60 minutes | Zone 6 cold accumulation -20% in future runs |
| **True Eternal** | Complete EE 5 times with 5 different endings | Unlock "Conductor's Insight" — see enemy types before entering each car |
| **The Perfect Run** | Complete EE with zero deaths, all companions alive, zone boss non-violence | Cosmetic: Golden Exo-Frame skin for standard mode |

### Scoring and Meta-Progression

| Metric | Points | Details |
|--------|--------|---------|
| **Cars Cleared** | 10/car | Incentivizes pushing deeper |
| **Zone Boss Defeated** | 100/boss | Major milestone reward |
| **Companions Alive at End** | 50/companion | Rewards careful play |
| **Resources Remaining** | 1/unit | Minor efficiency bonus |
| **Time Bonus** | Up to 200 | Sub-10-hour completion |
| **No-Death Bonus** | 500 | Never hit 0 HP |
| **Ending Achieved** | 200 | Any ending in EE counts |

**Lifetime EE Stats:**
- Total runs attempted
- Total runs completed
- Longest run (cars cleared before death)
- Fastest completion
- Most companions saved
- Most common cause of death
- Favorite companion (most recruited across runs)
- Favorite ending (most chosen across runs)

---

## 6. Procedural Element Settings

All procedural systems can be individually configured:

| Setting | Options | Default |
|---------|---------|---------|
| **Loot Randomization** | Off (fixed) / On (seeded) / Full Random | On (seeded) |
| **NPC Placement Variation** | Off / Shifted / Randomized / Adaptive | Shifted (NG+) |
| **Patrol Variation** | Off / Shifted / Randomized / Adaptive | Per NG+ cycle |
| **Daily Challenges** | Enabled / Disabled | Enabled |
| **Weekly Challenges** | Enabled / Disabled | Enabled |
| **Roguelike Mode** | Locked until first completion | — |

### Difficulty Interaction

Procedural elements interact with the game's difficulty settings:

| Difficulty | Procedural Modification |
|------------|------------------------|
| **Passenger** (Easy) | Loot quality +1 tier, patrol variation capped at Shifted, EE enemy scaling reduced 50% |
| **Snowpiercer** (Normal) | Standard procedural behavior |
| **Eternal Winter** (Hard) | Loot quality -1 tier in Zones 1–2, patrol variation starts at Randomized, EE enemy scaling +10% |
| **Wilford's Law** (Very Hard) | Loot quality -1 tier everywhere, patrol variation always Adaptive, EE run-based unlocks require 2× completions |

---

## Design Principles

1. **Procedural within authored bounds.** Every random element draws from authored content pools. No purely generated rooms, NPCs, or dialogue. Randomization is selection and arrangement, not creation.
2. **First playthrough is sacred.** No procedural variation affects the first playthrough. Fixed seeds ensure every player has the same initial experience. Variation kicks in during NG+ and optional modes.
3. **Permadeath earns its stakes.** Eternal Engine is clearly labeled and opt-in. It never affects main game progress. Its unlocks are run-based bonuses, not required content.
4. **Daily/Weekly modes respect time.** Daily challenges are 45–90 minutes. Weekly challenges are 3–6 hours. Neither requires the 30+ hour investment of a full playthrough.
5. **Unlocks reward mastery, not grind.** Run-based unlocks in Eternal Engine make future runs smoother, not trivial. A veteran player starts slightly better equipped, but the challenge remains.
