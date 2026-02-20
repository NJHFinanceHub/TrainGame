# Post-Game Statistics Screen

## Overview

After the ending cinematic and credits, the player is presented with a multi-page statistics screen summarizing their entire playthrough. This screen is accessible at any time from the main menu under "Journey Log" after completing the game at least once. Each playthrough's statistics are saved permanently and can be compared side-by-side.

---

## Page 1: The Journey — Summary

A stylized map of the train (side elevation view) with key events marked along its length. Each zone is color-coded and labeled.

### Header Stats

| Stat | Format | Example |
|------|--------|---------|
| **Ending Achieved** | Name + short description | "The Exodus — You opened the doors." |
| **Total Playtime** | Hours:Minutes | 34:17 |
| **Completion Percentage** | X% (see completion-tracking.md) | 67% |
| **NG+ Cycle** | Number (0 for first playthrough) | 0 |
| **Difficulty** | Setting name | Snowpiercer |

### Character Summary

| Field | Details |
|-------|---------|
| **Background** | Name + starting perk (e.g., "Scavenger — Rat's Eye") |
| **Final Level** | Level at endgame (e.g., "Level 27") |
| **Final Stats** | STR/END/CUN/CHA/PER/SRV values displayed as a hexagonal radar chart |
| **Build Archetype** | Auto-classified based on top 2 stats (e.g., "Ghost — PER/CUN") |
| **Highest Skill Tree** | Tree with most nodes unlocked (e.g., "Stealth — 12/15") |
| **Perks Equipped at End** | List of 6 equipped perks |

---

## Page 2: Combat & Stealth

### Combat Statistics

| Stat | Tracked Value |
|------|---------------|
| **Total Kills** | Number |
| **Non-Lethal Takedowns** | Number |
| **Kill Ratio** | Kills / (Kills + Takedowns) as percentage |
| **Favorite Weapon** | Most kills/takedowns with a single weapon |
| **Weapon Tier Used Most** | T1–T5, weighted by time equipped |
| **Highest Single Hit Damage** | Damage value + weapon + target |
| **Times Downed** | Number of times HP reached 0 |
| **Deaths** | Number of reloads from death |
| **Injuries Sustained** | Count by tier (Tier 1/2/3/4) |
| **Worst Injury** | Highest tier injury + body region + cause |
| **Bosses Defeated in Combat** | X/7 |
| **Bosses Resolved Non-Violently** | X/7 |

### Stealth Statistics

| Stat | Tracked Value |
|------|---------------|
| **Cars Cleared Undetected** | X/103 |
| **Disguises Worn** | List of types used |
| **Disguise Challenges Passed** | X passed / Y total encountered |
| **Times Detected** | Count |
| **Longest Undetected Streak** | Number of consecutive cars without detection |
| **Vent Traversals** | Count |
| **Under-Car Traversals** | Count |
| **Rooftop Traversals** | Count |

### Playstyle Classification

Based on kill ratio, detection rate, and dialogue check usage, the game assigns a playstyle label displayed prominently:

| Label | Criteria |
|-------|----------|
| **The Butcher** | Kill ratio > 80%, few stealth takedowns |
| **The Soldier** | Kill ratio 50–80%, balanced combat approach |
| **The Shadow** | Kill ratio < 20%, high undetected car count |
| **The Ghost** | Zero kills, zero detections |
| **The Diplomat** | Most encounters resolved via dialogue |
| **The Survivor** | High injury count, many near-death recoveries |
| **The Engineer** | Most encounters resolved via environmental/trap solutions |
| **The Revolutionary** | High companion involvement, leadership skills dominant |

---

## Page 3: Companions

### Companion Roster Grid

All 12 companions displayed in a 2×6 grid (zone order). Each entry shows:

| Field | Details |
|-------|---------|
| **Portrait** | Character art (greyed out if never recruited) |
| **Recruited** | Yes/No |
| **Final Loyalty State** | Hostile / Resentful / Cold / Neutral / Friendly / Devoted / Bonded |
| **Survived** | Yes / Died (with cause of death if applicable) |
| **Personal Quest Progress** | X/4 steps completed (or X/5 for extended chains) |
| **Time in Active Party** | Hours:Minutes |
| **Combat Contribution** | Kills + Takedowns attributed to companion |

### Companion Highlights

| Stat | Value |
|------|-------|
| **Most Loyal Companion** | Name + loyalty state |
| **Least Loyal Companion** | Name + loyalty state (if any reached Resentful or Hostile) |
| **Companions Lost** | Count + names |
| **Cause of Death Summary** | For each dead companion: "Died in combat at Car X" / "Sacrificed at Car X" / "Abandoned after loyalty collapse" / "Killed by player choice" |
| **Romance** | Name of romanced companion (if any) + relationship stage reached |
| **Companion Conflicts Resolved** | X/8 documented conflicts encountered + which side taken |
| **Best Synergy Pair** | Most-used companion pairing + synergy bonus name |

### The Memorial Wall

If any companions died, a solemn panel lists them with:
- Name and title
- Zone and car of death
- Cause (combat, sacrifice, choice, loyalty collapse)
- Their final words (last dialogue line before death)
- How many NPCs referenced their death in subsequent zones

---

## Page 4: Factions & Reputation

### Faction Standings Bar Chart

Horizontal bar chart showing final reputation for all factions:

| Faction | Final Rep | Standing | Key Moment |
|---------|-----------|----------|------------|
| **Tail Council** | +750 | Allied | "Led the revolution from Car 1" |
| **Jackboots** | -400 | Hostile | "Defeated Commander Grey in combat" |
| **Third Class Union** | +300 | Friendly | "Supported the strike" |
| **Kronole Network** | +100 | Neutral | "Tolerated but didn't engage" |
| **Second Class Bureaucracy** | -200 | Unfriendly | "Disrupted the trial" |
| **Working Spine** | +500 | Allied | "Recruited Kazakov" |
| **First Class Elite** | -100 | Unfriendly | "Rejected Beaumont's deal" |
| **Order of the Engine** | 0 | Neutral | "Reformed the Order" |
| **Thaw Believers** | +200 | Friendly | "Provided Thaw evidence" |
| **The Forgotten** | +150 | Friendly | "Discovered Car 10" |

The "Key Moment" is the single highest-impact reputation event for each faction.

### Faction Summary Stats

| Stat | Value |
|------|-------|
| **Factions Allied** | Count + names |
| **Factions Hostile** | Count + names |
| **Double Agent Active** | Yes/No (maintained opposing faction relationships simultaneously) |
| **Trade Volume** | Total items bought + sold across all merchants |
| **Best Merchant Relationship** | Merchant name + favor tier |
| **Black Market Visits** | Count |

---

## Page 5: Moral Choices & The Hidden Ledger

### The Hidden Ledger (Revealed)

After the first completion, the Hidden Ledger is fully revealed. A pentagonal radar chart shows the player's final position on all five axes:

| Axis | Player Score | Scale | Dominant Lean |
|------|-------------|-------|---------------|
| **Mercy ↔ Pragmatism** | -3 to +3 | Mercy = positive | "Leaned Pragmatic" |
| **Individual ↔ Collective** | -3 to +3 | Individual = positive | "Strongly Collective" |
| **Truth ↔ Stability** | -3 to +3 | Truth = positive | "Balanced" |
| **Force ↔ Cunning** | -3 to +3 | Force = positive | "Leaned Cunning" |
| **Present ↔ Future** | -3 to +3 | Present = positive | "Strongly Future" |

### Moral Choice Log

All 24 moral choices listed in zone order. Each entry shows:

| Field | Details |
|-------|---------|
| **Choice Name** | e.g., "The Ration Dilemma" |
| **Zone / Car** | e.g., "Zone 1 / Car 8" |
| **Option Chosen** | The specific option selected |
| **Immediate Consequence** | What happened right after |
| **Downstream Consequence** | Delayed effects that materialized later (if any triggered) |
| **Ledger Impact** | Which axes were affected and in which direction |

### Zone-Defining Choices Highlighted

The 6 zone-defining choices (Gilliam's Secret, Checkpoint Decision, The Trial, Engine's Children, Dinner with Beaumont, The Order's Fate) are displayed larger with branching diagrams showing all available options and which was chosen.

### Moral Summary Stats

| Stat | Value |
|------|-------|
| **Choices Made** | 24/24 (or fewer if any were skipped/missed) |
| **Most Common Lean** | The Ledger axis with the strongest lean |
| **Wilford's Final Words** | The specific dialogue Wilford delivered based on the player's Ledger configuration |
| **Ending Variations Triggered** | Which specific ending variant was achieved (e.g., "The Revolution — Democratic Transition" vs. "The Revolution — Military Dictatorship") |

---

## Page 6: Exploration & Collection

### Car Completion Grid

A 103-cell grid (styled as a train diagram) where each car is colored:
- **Green**: Fully explored (all loot found, all NPCs spoken to, all events triggered)
- **Yellow**: Partially explored (some items or NPCs missed)
- **Red**: Entered but minimally explored
- **Grey**: Never entered
- **Gold border**: Car completion bonus XP earned

Hovering/selecting a car shows: items found/total, NPCs met/total, events triggered/total.

### Collectible Tracking

| Category | Found / Total | Notes |
|----------|---------------|-------|
| **Journal Pages** | X/Y | Protagonist's pre-Freeze journal entries |
| **Memory Fragments** | X/Y | Flashback vignettes from environmental triggers |
| **Blueprint Collectibles** | X/6 | Legendary crafting blueprints |
| **Legendary Weapons** | X/5 | T5 unique weapons acquired |
| **Permanent Stat Items** | X/5 | One-time stat-boosting consumables used |
| **Lore Documents** | X/Y | Historical records, memos, letters found in the world |
| **Thaw Symbols** | X/Y | Hidden Thaw Believer markings discovered |
| **Revolution Evidence** | X/Y | Physical artifacts from the 4 previous revolutions |
| **Timmy's Requests** | X/Y | Items brought back to Timmy from forward zones |

### Exploration Summary Stats

| Stat | Value |
|------|-------|
| **Cars Fully Explored** | X/103 |
| **Total Items Collected** | Count |
| **Rarest Item Found** | Name (based on global discovery rate if online features exist) |
| **Total Distance Traveled** | Meters (tracked per-car as the train is traversed) |
| **Exterior Traversals** | Count of hull/rooftop sections crossed |
| **Hidden Rooms Discovered** | X/Y |
| **Quests Completed** | Main + side quest count |

---

## Page 7: Economy & Crafting

### Resource Summary

| Resource | Total Collected | Total Used | Total Traded | Peak Held |
|----------|----------------|------------|--------------|-----------|
| Metal Scrap | — | — | — | — |
| Cloth | — | — | — | — |
| Protein Blocks | — | — | — | — |
| Fresh Food | — | — | — | — |
| Medicine | — | — | — | — |
| Kronole | — | — | — | — |
| Electronics | — | — | — | — |
| Chemicals | — | — | — | — |
| Luxury Items | — | — | — | — |
| Ammo | — | — | — | — |
| Engine Parts | — | — | — | — |

### Crafting Summary

| Stat | Value |
|------|-------|
| **Items Crafted** | Total count |
| **Recipes Known** | X/62 standard + X/6 blueprint |
| **Crafting Stations Used** | List of station types visited |
| **Masterwork Items Created** | Count + names (items with unique properties from high-stat crafting) |
| **Failed Crafts** | Count (from low-stat attempts) |
| **Most Crafted Item** | Name + count |

### Trade Summary

| Stat | Value |
|------|-------|
| **Total Trades** | Count |
| **Best Haggle** | Highest discount percentage achieved |
| **Best Arbitrage** | Most profitable single trade (buy zone → sell zone → profit margin) |
| **Merchant Favors Earned** | X across all merchants |
| **Kronole Consumed** | Count (for addiction/perk tracking) |

---

## Page 8: Road Not Taken

This page is the emotional centerpiece of the statistics screen. It shows what the player missed or chose against.

### Paths Not Chosen

For each of the 24 moral choices, a brief description of what would have happened with each unchosen option:
- Displayed as collapsed/expandable entries to avoid overwhelming the player
- Text is written as "What if..." hypotheticals: "What if you had fed the children? Timmy would have survived to Zone 3, where he becomes a crucial messenger for the revolution."
- Information revealed scales with NG+ cycle: first playthrough shows vague hints; NG+ 1 shows moderate detail; NG+ 2+ shows full consequences

### Companions Never Met

For each companion not recruited:
- Brief description of their storyline and what they would have contributed
- Their unique ability description
- A quote from their personal quest that the player will never hear (unless they recruit them next time)

### Endings Not Achieved

For each of the 6 endings not selected:
- A 2–3 sentence description of what would have happened
- The Ledger configuration required to unlock it (if it's a secret ending)
- Whether the player's current Ledger would have qualified for it

### The Counter-Factual

A single generated paragraph describing the most dramatically different version of the player's journey based on the choices they didn't make. Example:

> "In another timeline, you fed the children in Car 8, spared Commander Grey, and trusted Beaumont's deal. The revolution succeeded without bloodshed, but Beaumont's reforms hollowed out equality within a year. Edgar died defending you in Zone 4 because you never invested in his loyalty. The train is warm, fed, and slowly rotting from within."

This paragraph is assembled from a template system using choice/consequence data, not generated by AI at runtime.

---

## Multi-Playthrough Comparison

### Journey Log (Main Menu)

After completing the game, the main menu gains a "Journey Log" option containing:
- A chronological list of all completed playthroughs with summary cards
- Side-by-side comparison mode: select 2–3 playthroughs to compare stats
- A cumulative "Master Statistics" page showing totals across all playthroughs

### Comparison View Fields

| Category | Comparison Points |
|----------|------------------|
| **Character** | Background, final level, stat distribution, build archetype |
| **Combat** | Kill count, playstyle label, boss resolution methods |
| **Companions** | Who survived, loyalty states, romances |
| **Factions** | Final standings per faction |
| **Ledger** | Radar chart overlay showing how Ledger scores differ |
| **Ending** | Which ending, which variant |
| **Completion** | Percentage, cars explored, collectibles found |

### Master Statistics (Cumulative)

| Stat | Details |
|------|---------|
| **Total Playtime** | Sum across all playthroughs |
| **Playthroughs Completed** | Count |
| **Unique Endings Seen** | X/6 |
| **Unique Moral Choices Made** | X/96 (24 choices × 4 options average) |
| **Companions Recruited (Ever)** | X/12 |
| **Companions Lost (Total)** | Count |
| **All Blueprints Found (Ever)** | X/6 |
| **All Legendaries Acquired (Ever)** | X/5 (base game) + X/2 (NG+ exclusives) |
| **Titles Earned** | List |

---

## UI/UX Considerations

- **Presentation pace**: Statistics pages auto-advance with a slow reveal (2–3 seconds per stat), but the player can skip ahead or go back freely.
- **Music**: A quiet, reflective arrangement of the game's main theme plays during the statistics screen. Different musical cues for combat stats (percussive), companion stats (melodic), and moral choices (ambient).
- **Spoiler protection**: The "Road Not Taken" page warns the player before revealing what they missed. Can be hidden entirely in settings.
- **Accessibility**: All statistics are available as plain text in a scrollable list for screen readers. Color coding is paired with icons (not color-dependent).
- **Export**: Players can export their statistics as a shareable image (a stylized "journey card" showing key stats) for social media.
