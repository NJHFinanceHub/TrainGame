# Perk System

## Overview

Perks are powerful passive abilities that define playstyle identity. Unlike skill tree nodes (incremental bonuses), perks are transformative — they change how you interact with core systems.

## Perk Slots

- **6 perk slots** total, unlocked at levels 5, 10, 15, 20, 25, 30
- Perks can be swapped at **rest points** (braziers, safe rooms, beds) — not in the field
- Some perks have prerequisites (stat thresholds, quest completions, faction standing)
- Equipped perks are always active. No activation, no cooldowns.

## Perk List

### Combat Perks

| Perk | Prerequisite | Effect |
|---|---|---|
| **Glass Cannon** | STR 10 | +40% melee damage, -20% max HP |
| **Iron Wall** | END 12 | Block consumes no stamina. Cannot sprint while blocking. |
| **First Blood** | Combat tree: Adrenaline | First hit in each combat encounter is a guaranteed critical |
| **Cornered Animal** | Complete Zone 1 | Below 25% HP: attack speed doubled |
| **Duelist** | STR 8, CUN 8 | +30% damage when fighting a single enemy. -15% when outnumbered. |
| **Breaker** | STR 14 | Melee attacks penetrate enemy blocks (50% damage through block) |

### Stealth Perks

| Perk | Prerequisite | Effect |
|---|---|---|
| **Shadowwalker** | Stealth tree: Shadow Blend | Moving while crouched in darkness produces zero noise |
| **Opportunist** | PER 10 | Backstab damage +100% |
| **Face in the Crowd** | CHA 8, CUN 8 | In civilian areas, guards cannot detect you unless you act hostile |
| **Night Vision** | Complete Car 10 (Dark Car) | See in darkness as if dim lighting. Perpetual. |
| **Patience** | SRV 10 | Crouching for 5 seconds without moving grants temporary invisibility (10 sec) |
| **Clean Hands** | No kills through Zone 1 | Non-lethal takedowns restore 5% HP |

### Survival Perks

| Perk | Prerequisite | Effect |
|---|---|---|
| **Iron Stomach** | END 8 | No negative effects from spoiled food. Protein blocks restore +50% |
| **Frostblood** | Survive Car 11 without cold damage | Cold damage immunity. Visual distortion still applies. |
| **Addiction Immune** | SRV 12, never used Kronole | Immune to Kronole addiction. Still get buffs. (Locked if you've ever used Kronole) |
| **Pack Rat** | SRV 8 | Carry capacity +50% |
| **Scrap King** | CUN 10, SRV 8 | Crafting costs reduced by 25% for all recipes |
| **Camel** | END 10 | Hunger meter drains 40% slower |

### Social Perks

| Perk | Prerequisite | Effect |
|---|---|---|
| **Natural Leader** | CHA 12 | Companion max morale increased by 20. Morale decay rate halved. |
| **Merchant Prince** | CHA 10, Trade 3+ barter transactions | Buy/sell prices improved by 15% |
| **Terrifying Presence** | STR 12 OR CHA 12 | New dialogue option: Threaten. Some enemies flee combat. |
| **Double Life** | CUN 12 | Faction reputation is hidden from NPCs. They react to your last action, not your history. |
| **Oathkeeper** | Complete 5 companion quests | Companions never abandon you, regardless of moral choices |
| **Recruiter** | CHA 14, Leadership tree: Born Leader | Some enemies can be persuaded to surrender and join as temporary allies |

### Engineer Perks

| Perk | Prerequisite | Effect |
|---|---|---|
| **Walking Workshop** | CUN 12, Engineer tree: Upgrade Station | Can craft Tier 2 items without a workbench |
| **Saboteur** | CUN 10 | Environmental traps you create deal +100% damage and last until triggered |
| **System Administrator** | Hack 3+ electronic panels | Hacked cameras remain permanently under your control |
| **Overcharge** | Engineer tree: Circuit Breaker | Electrical interactions can be boosted for double effect (risky — 20% chance of backfire) |
| **Recyclotron** | CUN 8, SRV 8 | Breaking down items yields 100% materials (up from 75% with Salvage skill) |
| **Architecture Reader** | PER 10, CUN 10 | Structural weaknesses in ALL objects visible. See hidden rooms through walls. |

### Unique Perks (Quest-Locked)

| Perk | How to Unlock | Effect |
|---|---|---|
| **Gilliam's Heir** | Complete Gilliam's final quest (Zone 1) | Tailie NPCs fight alongside you in any zone when combat begins nearby |
| **Kronole Visionary** | Use Kronole 10+ times, complete Night Car quest | Kronole grants precognition: see enemy movements 2 seconds in advance |
| **Engine's Whisper** | Reach Zone 6 with Order of the Engine rep: Revered | Sense the Engine's "heartbeat" — reveals hidden passages and machinery interactions |
| **Breachman** | Complete exterior walk without cold damage (requires Frostblood) | Can exit and re-enter the train through hull breaches. Access exterior route between adjacent cars. |
| **Wilford's Mirror** | Reach Zone 5 with First Class Elite rep: Allied | First Class NPCs treat you as one of them. Full disguise without Tailor outfits. |

## Design Notes

**Perk balance philosophy:**
- Every perk should feel impactful enough to be worth the slot
- Trade-off perks (Glass Cannon, Duelist) should tempt aggressive players with real risk
- Quest-locked perks are the most powerful but require specific playthrough decisions
- No perk should feel mandatory — there's no "must-have" that invalidates other choices
- Perks that gate on "never did X" (Addiction Immune, Clean Hands) reward deliberate restraint and create interesting tension when the temptation arises
