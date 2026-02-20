# Difficulty Settings

## Overview

Four difficulty tiers themed around the train's social hierarchy. Difficulty affects enemy stats, resource availability, detection sensitivity, and environmental lethality. Players choose difficulty at game start and can change it mid-game with restrictions.

## Difficulty Tiers

### Passenger

**Theme:** You're here for the story. The train is dangerous, but forgiving.

| Parameter | Modifier |
|-----------|----------|
| Enemy health | -30% |
| Enemy damage | -40% |
| Enemy detection range | -25% |
| Enemy detection speed | -30% |
| Resource drop rate | +50% |
| Resource degradation rate | -50% |
| Cold exposure time | +50% |
| Stamina drain | -25% |
| XP gain | +25% |
| Companion down timer | 60 seconds |
| Fall damage | -50% |
| Injury severity | Reduced by 1 tier |

**Additional rules:**
- Quest markers always visible
- Lockpick/hack mini-games have an auto-complete option after 2 failures
- Death respawns at nearest safe point with no resource loss
- Contested stat checks get +3 bonus
- Companions cannot permanently die

### Survivor

**Theme:** The intended experience. The train tests you, but fair play is rewarded.

| Parameter | Modifier |
|-----------|----------|
| Enemy health | Baseline (1.0x) |
| Enemy damage | Baseline (1.0x) |
| Enemy detection range | Baseline (1.0x) |
| Enemy detection speed | Baseline (1.0x) |
| Resource drop rate | Baseline (1.0x) |
| Resource degradation rate | Baseline (1.0x) |
| Cold exposure time | Baseline (1.0x) |
| Stamina drain | Baseline (1.0x) |
| XP gain | Baseline (1.0x) |
| Companion down timer | 30 seconds |
| Fall damage | Baseline (1.0x) |
| Injury severity | Normal |

**Additional rules:**
- Quest markers visible for active quest only
- Standard mini-game difficulty
- Death respawns at last checkpoint with 10% resource loss
- Standard stat checks
- Companions can die if not revived within down timer

### Eternal Engine

**Theme:** The train is a meat grinder. Every resource matters. Every fight could be your last.

| Parameter | Modifier |
|-----------|----------|
| Enemy health | +25% |
| Enemy damage | +30% |
| Enemy detection range | +20% |
| Enemy detection speed | +25% |
| Resource drop rate | -30% |
| Resource degradation rate | +25% |
| Cold exposure time | -25% |
| Stamina drain | +20% |
| XP gain | -15% |
| Companion down timer | 15 seconds |
| Fall damage | +30% |
| Injury severity | Increased by 1 tier |

**Additional rules:**
- No quest markers (journal descriptions only)
- Mini-games have no retry hints
- Death respawns at last checkpoint with 25% resource loss
- Contested stat checks get -2 penalty
- Companions die permanently if not revived
- Friendly fire enabled at reduced damage (25%)
- Save limited to rest points and zone transitions

### Mr. Wilford

**Theme:** Wilford designed this train to break people. Prove him wrong.

| Parameter | Modifier |
|-----------|----------|
| Enemy health | +50% |
| Enemy damage | +60% |
| Enemy detection range | +35% |
| Enemy detection speed | +40% |
| Resource drop rate | -50% |
| Resource degradation rate | +50% |
| Cold exposure time | -40% |
| Stamina drain | +35% |
| XP gain | -25% |
| Companion down timer | 8 seconds |
| Fall damage | +60% |
| Injury severity | Increased by 2 tiers |

**Additional rules:**
- No quest markers, no HUD compass
- Mini-games have no visual aids
- Death is permanent (permadeath) — single save file, deleted on death
- Contested stat checks get -4 penalty
- Companions die permanently if not revived
- Friendly fire enabled at full damage
- Save only at rest points (no zone transition saves)
- No auto-healing out of combat (requires medicine or rest)
- Cold kills faster — hull breach sections become lethal puzzles
- Enemy patrols are randomized (no memorizable patterns)
- Kronole withdrawal onset is 50% faster

## Adaptive Difficulty

An optional system that can be layered on top of any base difficulty tier (except Mr. Wilford). Toggled independently from the base tier selection.

### How It Works

The system tracks a hidden **performance score** based on rolling metrics from the last 30 minutes of play:

| Metric | Score Impact |
|--------|-------------|
| Player deaths | -10 per death |
| Combat time (avg per encounter) | >60s: -2, <15s: +3 |
| Health at combat end (avg) | <25%: -3, >75%: +2 |
| Resources remaining (% of carry cap) | <20%: -2, >60%: +1 |
| Stealth detections (rate) | >50% detected: -2, <10%: +3 |
| Time since last heal | >10 min: -1 |

**Score thresholds:**
- **Below -15:** Difficulty shifts down one sub-tier
- **-15 to +15:** No adjustment (dead zone)
- **Above +15:** Difficulty shifts up one sub-tier

### Sub-Tier Adjustments

Each base tier has three sub-tiers: Easy, Normal, Hard. Adaptive difficulty shifts between these by applying ±10% to all numerical modifiers.

- Adjustments are **gradual** (applied over 2-3 minutes to avoid noticeable jumps)
- Adjustments are **capped** (never shifts more than one sub-tier from the base)
- Players are **never told** the system is adjusting — the experience should feel organic
- The system **resets** on zone transitions to avoid carrying frustration between areas

### Opt-Out

Players can disable adaptive difficulty at any time. Disabling it resets all sub-tier adjustments to Normal.

## Mid-Game Difficulty Changes

Players can change difficulty mid-game from the pause menu with the following rules:

### Lowering Difficulty

- Available at any time, no restrictions
- Takes effect immediately
- No penalty or narrative acknowledgment
- Progress, achievements, and unlocks are unaffected

### Raising Difficulty

- Available at any time
- Takes effect immediately
- **Resource adjustment:** Current resources are reduced by the difference in resource drop modifiers (e.g., switching Passenger → Survivor removes the +50% bonus from current inventory, reducing held resources by 33%)
- **Enemy state:** All currently spawned enemies are re-scaled to new difficulty values
- **No retroactive XP adjustment** — XP already earned is kept

### Mr. Wilford Restrictions

- Cannot switch **to** Mr. Wilford mid-game (must be selected at game start)
- Can switch **from** Mr. Wilford to a lower difficulty, but the save file converts from permadeath to checkpoint-based. A one-time confirmation dialog warns: *"Leaving Mr. Wilford difficulty is permanent. You cannot return. Continue?"*
- Switching from Mr. Wilford disables Mr. Wilford-exclusive achievements for that save

### Achievement Tracking

- Achievements track the **lowest difficulty played** on that save file
- Difficulty-gated achievements (e.g., "Complete Zone 3 on Eternal Engine or higher") require the entire qualifying section to be played at that difficulty or above
- Mr. Wilford has exclusive achievements that require a full playthrough without ever lowering difficulty
