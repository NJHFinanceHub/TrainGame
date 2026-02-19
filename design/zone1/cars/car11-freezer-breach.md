# Car 11: The Freezer Breach

**Type:** Environmental Hazard — Cold Damage Timer
**Difficulty:** Medium (time pressure, no combat, navigation puzzle)
**Estimated Play Time:** 8-12 minutes

## Narrative Context

A hull breach. Somewhere in the wall, a seal failed. The outside — minus 80 Celsius — pours in through a jagged crack. Ice has formed on every surface. The temperature drops the closer you get to the breach. This car is a frozen death trap that must be crossed quickly. The Jackboots abandoned it years ago. Nobody comes here by choice.

## Layout

```
[DOOR ← Car 10]
|                                    |
|  [Threshold — cold hits here]     |  Section A: Entry Cold Zone (8m)
|  [Frost on walls, floor icy]      |
|  [Warm pipe — heating element]    |
|  [Frozen NPC — dead, preserved]   |
|                                    |
|-------- Ice wall (partial) --------|
|                                    |
|  [Frozen debris — path blocked]   |  Section B: The Breach Zone (18m)
|  [THE BREACH — wall crack, wind]  |
|  [Ice stalactites from ceiling]   |
|  [3 warm spots — steam pipes]     |
|  [Navigation puzzle — find path]  |
|                                    |
|-------- Frozen partition ----------|
|                                    |
|  [Collapsing ice shelf]           |  Section C: The Crossing (14m)
|  [Overhead pipes — climbable]     |
|  [Floor partially missing — void] |
|  [Wind gusts — knockback risk]    |
|                                    |
|-------- Ice thins ------------------|
|                                    |
|  [Safe zone — sealed bulkhead]    |  Section D: Exit (10m)
|  [Warm vent — recovery spot]      |
|  [Supplies left by previous crew] |
|                                    |
[DOOR → Car 12]
```

## Environmental Details

### The Cold System

This car introduces the **Cold damage mechanic** as a central challenge:

| Zone | Temperature | Cold Damage Rate | Time to Death |
|---|---|---|---|
| Section A | -10C | Slow (1 HP/10s) | ~3 minutes |
| Section B (away from breach) | -30C | Moderate (1 HP/5s) | ~90 seconds |
| Section B (near breach) | -60C | Fast (1 HP/2s) | ~40 seconds |
| Section C | -40C | Moderate-Fast (1 HP/3s) | ~60 seconds |
| Section D | +5C | None (recovery) | Safe |

**Warm spots** — Steam pipes and heating elements provide temporary safe zones where cold damage stops and slowly reverses. The player must hop between warm spots.

**Cold resistance** — Lurker Pelts (from Car 10) reduce cold damage by 30%. Cloth wraps help slightly. These matter here.

### Section A: Entry Cold Zone
- The temperature drop is immediate and visceral — frost on the camera edges, breath visible, the player character's animations slow slightly.
- A warm pipe runs along the left wall — standing near it stops cold damage. First warm spot.
- **Frozen NPC** — A perfectly preserved corpse, mid-stride. Died trying to cross. A warning. Can be looted: warm gloves (cold resistance item), 1 protein block.

### Section B: The Breach Zone
- **THE BREACH** — A jagged crack in the right wall, ~30cm wide. Wind howls through it. Snow/ice crystals blow across the car. The outside world is visible: a white void of frozen wasteland.
- Ice has formed massive stalactites from the ceiling. Some block pathways. Some can be broken (environmental interaction).
- **3 warm spots** — Steam pipes at irregular intervals. The player must reach each one before cold damage becomes lethal.
- **Navigation puzzle:** Direct path is blocked by ice formations and frozen debris. Must find alternate routes through/over/under obstacles.

### Section C: The Crossing
- The floor has partially collapsed — corroded by the cold and ice expansion. Gaps reveal the undercarriage (wind, tracks, death).
- **Overhead pipes** — Must be crossed by climbing/shimmying along pipes above the void.
- **Wind gusts** — Periodic blasts of wind from the breach direction. On the pipes: risk of knockback. Player must time movements between gusts.
- No warm spots in this section. Continuous cold damage. Speed is survival.

### Section D: Exit
- A sealed bulkhead door separates the frozen section from the rest. Emergency heating vent still functions.
- **Recovery zone** — Cold damage stops. HP slowly regenerates.
- Supplies left by a Jackboot maintenance crew (abandoned their post): food, medicine, a note about the breach being "impossible to repair from inside."

## Gameplay

### Core Challenge: Beat the Clock

The Freezer Breach is a navigation puzzle under time pressure:
1. Enter → Cold damage starts
2. Reach Warm Spot 1 (Section A pipe) → Plan route
3. Sprint to Warm Spot 2 (Section B, first pipe) → Navigate debris
4. Sprint to Warm Spot 3 (Section B, far pipe) → Break ice obstacles
5. Cross the void (Section C pipes) → Time wind gusts
6. Reach Section D → Safe

**Total crossing time:** 60-90 seconds of active cold exposure (with warm spot rests between)
**Without warm spots:** ~40 seconds before death. Doable only with cold resistance gear.

### Navigation Puzzle

The direct path through Section B is blocked by a wall of ice. Options:
- **Smash through** (weapon required): Hit ice wall 3 times. Takes ~8 seconds. Cold damage accrues.
- **Climb over debris** (no requirement): Longer route through overturned furniture/bunks. ~15 seconds.
- **Crawl under** (small gap): Fastest route but requires crouch. ~5 seconds. Not obvious.

### Section C Pipe Crossing

A platforming/timing challenge:
- 3 pipe segments to cross, each ~4m long
- Between segments: ~1m gaps (jump)
- Wind gusts every 5 seconds (3-second duration). Must stop and grip during gusts.
- Falling = death (instant, into undercarriage void)
- The crossing takes ~20 seconds with optimal timing

### Cold Resistance Equipment (Affects Difficulty)

| Equipment | Source | Cold Resist | Effect |
|---|---|---|---|
| Lurker Pelt Wrap | Car 10 (crafted) | -30% cold damage | Significant survival boost |
| Warm Gloves | Frozen NPC, Section A | -10% cold damage | Minor but helpful |
| Cloth Layers | Crafted (Workshop) | -15% cold damage | Moderate |
| Kronole (if used) | Car 12 (preview) | Ignore cold 30s | Powerful but costly |

## Items

| Item | Location | Type |
|---|---|---|
| Warm Gloves | Frozen NPC, Section A | Armor (cold resist) |
| Protein Block (x1) | Frozen NPC, Section A | Food |
| Medicine Dose (x1) | Section D supplies | Medicine |
| Maintenance Note | Section D supplies | Lore (breach info) |
| Ice Crystal (x3) | Broken stalactites | Crafting (weapon: ice shard) |
| Jackboot Ration Pack | Section D crate | Food (x3 meals) |

## NPCs

None living. The Freezer Breach is uninhabitable. The frozen corpse tells the story.

## Lighting

- **Section A:** Ice-blue. The frost reflects and scatters what little light exists. LED strips under ice.
- **Section B:** Blinding white near the breach (outside light pouring in). Deep blue shadows elsewhere. The contrast is extreme.
- **Section C:** Dark below (the void). Ice-blue above. The pipes are silhouetted against emergency LEDs.
- **Section D:** Warm yellow. A heater's glow. Safety. The warmest light since Car 5.

## Audio

- **The Wind:** Dominates everything. A constant, moaning howl from the breach. Rises and falls with gusts. Should feel physically cold.
- **Ice:** Cracking, groaning, tinkling. The car is alive with the sound of ice shifting under temperature stress.
- **Player:** Heavy breathing, chattering teeth (character audio). Footsteps crunch on ice.
- **Section C:** Wind is loudest here. The gap below has a void-wind sound — deep, resonant, terrifying.
- **Section D:** The wind cuts to almost nothing as the bulkhead seals behind. The warmth is audible (heating vent hum). Relief.

## Design Notes

- This car should feel physically uncomfortable. Camera frost effects, reduced visibility, the sound design — the player should want to get out as much as the character does.
- The frozen NPC is not horror — it's pragmatic. This is what happens if you're too slow. Motivation.
- The pipe crossing should be tense but fair. Wind gust timing is predictable (5-second cycle). Falling is a clear fail state — reload from Section C entrance (checkpoint).
- The breach itself is the first time the player sees the outside world. It should be simultaneously beautiful (pristine white) and terrifying (that's death out there).

## Revisit Behavior

- The cold remains. This car never becomes comfortable.
- With better cold resistance gear (later zones): crossing is easier but never trivial.
- The breach may be relevant to the Exodus ending (the outside is survivable in late-game?).
- Ice Crystals respawn — renewable crafting resource for cold-damage weapons.
