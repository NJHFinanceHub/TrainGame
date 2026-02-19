# Car 09: The Blockade

**Type:** First Real Combat Encounter
**Difficulty:** Medium (8-10 enemies, environmental combat)
**Estimated Play Time:** 15-25 minutes

## Narrative Context

The first Jackboot checkpoint. A reinforced gate blocks the forward passage, manned by a squad of armed guards. This is where the Tail's world ends and Third Class begins. Every previous revolution attempt was stopped here. The floor is stained with old blood. Commander Grey — the Jackboot officer responsible for Tail security — oversees this gate personally. This fight is the revolution's first test.

## Layout

```
[DOOR ← Car 08]
|                                    |
|  [Approach corridor — narrow]     |  Section A: The Approach (10m)
|  [Overhead pipes — exposed]       |
|  [Steam vents — wall-mounted]     |
|  [Crates for cover — scattered]   |
|                                    |
|===== BARRICADE (waist-high) ======|
|                                    |
|  [Main combat arena]              |  Section B: The Blockade (20m)
|  [Sandbag positions — 3 spots]    |
|  [Overhead catwalk — narrow]      |
|  [Steam pipe valve — left wall]   |
|  [Electrical panel — right wall]  |
|  [Hanging chains from ceiling]    |
|                                    |
|===== THE GATE (reinforced) =======|
|                                    |
|  [Gate mechanism — wheel lock]    |  Section C: The Gate (10m)
|  [Commander Grey's position]      |
|  [Guard booth — armored]          |
|  [Weapon rack — Jackboot gear]    |
|                                    |
|-------- Through the gate ----------|
|                                    |
|  [Aftermath corridor]             |  Section D: Beyond (10m)
|  [Transition to Car 10 door]      |
|                                    |
[DOOR → Car 10]
```

## Environmental Details

### Section A: The Approach
- Narrow corridor — intentional chokepoint. The Jackboots designed this to be indefensible from the Tail side.
- Overhead pipes are exposed and accessible — can be climbed on (stealth approach).
- **Steam vents** on walls — 3 vents that can be triggered to blast steam (environmental weapon).
- Scattered crates provide limited cover. Not enough to hide behind permanently.

### Section B: The Blockade
- The main fighting space. Wider than the approach — ~4m wide, allowing flanking.
- **Sandbag positions** — 3 defensive positions the Jackboots use. Player can also use them as cover.
- **Overhead catwalk** — A narrow maintenance walkway along the left wall, 2m up. Accessible via pipes in Section A. Allows flanking attacks from above.
- **Steam pipe valve** — Left wall. A large red valve. Turning it releases high-pressure steam across the center of the arena. Damages anyone in the steam (friend and foe).
- **Electrical panel** — Right wall. Can be smashed to short-circuit — triggers a brief power outage (1-2 seconds of darkness, disorienting guards).
- **Hanging chains** — From ceiling maintenance points. Can be swung into enemies for stagger.

### Section C: The Gate
- A reinforced steel door with a wheel-lock mechanism. Heavy. Requires clearing all guards.
- Commander Grey's position is behind the gate mechanism — he falls back here.
- Guard booth: an armored box with a slit window. One guard inside with a crossbow.
- Weapon rack: Jackboot-grade equipment (batons, shields, one crossbow). Lootable after combat.

### Section D: Beyond
- A short aftermath corridor. The player catches their breath. Transition to Car 10.
- Blood on the walls (old, from previous revolts). Names scratched into the metal.
- A moment of quiet before the Dark Car.

## Gameplay

### Pre-Combat (Intel-Dependent)

What the player knows before the fight depends on earlier choices:

| Condition | Effect |
|---|---|
| Got guard count from Listening Post | UI shows "8 enemies" before engagement |
| Got shift change timing | Option to attack during reduced guard (6 instead of 8) |
| Spy still active (Bookmaker not caught) | Guards are warned — reinforced (+2 enemies, total 10) |
| Recruited Kez | She fights alongside you. Significant help. |
| Recruited Whisper | She identifies a vent route (stealth bypass for 3 guards) |
| Saved Rem (fighter from Sickbay) | He joins the assault team (+1 allied NPC fighter) |

### Combat Encounter

**Enemy Composition (Base: 8 guards)**

| Enemy | Count | Equipment | Behavior |
|---|---|---|---|
| Jackboot Grunt | 4 | Baton + light armor | Aggressive, charge in pairs |
| Jackboot Shield | 2 | Shield + baton | Defensive, block attacks, push player |
| Jackboot Crossbow | 1 | Crossbow, dagger | Ranged, stays in guard booth |
| **Cmdr. Grey** | 1 | Sword + heavy armor | Commander, rallies troops, falls back to gate |

### Combat Flow

**Phase 1: The Approach**
- 2 Grunts patrol Section A. Can be engaged normally or stealth-killed (if Whisper's vent route: bypass entirely).
- Steam vents can be triggered to damage patrolling guards.

**Phase 2: The Blockade**
- Crossing the barricade triggers the main engagement. 4 guards behind sandbags.
- The crossbow guard fires from the booth (suppressive fire — player must use cover).
- **Environmental opportunities:**
  - Steam valve: clears the center, forces guards to scatter
  - Electrical panel: brief blackout, guards disorient (reduced accuracy for 5 seconds)
  - Catwalk: reach from above, drop attacks on guards below
  - Chains: swing into enemies for stagger + knockback

**Phase 3: The Gate**
- Grey falls back behind the gate when 4+ guards are down.
- He seals himself behind the gate — player must open it (wheel-lock takes 5 seconds while Grey threatens through the slit).
- Once opened: Grey is the last opponent. He's tough — heavy armor, good sword, blocks well.
- **Grey does NOT die here.** He retreats through a maintenance hatch when at 25% health. He reappears at Car 15 as the boss. This is foreshadowed by his dialogue: "This isn't over, Tailie."

### Old Pike's Fate

If Old Pike is still alive and present:
- He fights in the assault (regardless of player choice — he insists).
- **Scripted moment:** During Phase 2, a Jackboot gets behind Pike. If the player intervenes (reaches Pike within 5 seconds): Pike survives. If not: Pike is killed.
- Pike's death is not a fail state — it's a consequence. His death galvanizes the remaining fighters.
- If Pike survives: he remains at Car 5 as an advisor. If he dies: his memorial is added to Car 1's wall.

## Items

| Item | Location | Type |
|---|---|---|
| Jackboot Baton (x2) | Defeated guards | Weapon (good condition) |
| Jackboot Shield | Defeated shield guard | Shield (durable) |
| Crossbow + 8 Bolts | Guard booth | Ranged weapon |
| Light Armor Vest | Commander Grey (dropped) | Armor |
| Gate Key | Grey's retreat spot | Key item — opens shortcuts |
| Protein Blocks (x4) | Guard supply crate | Food |
| Medicine Kit (x1) | Guard booth | Medicine (full heal) |

## NPCs

| Name | Location | Role |
|---|---|---|
| **Cmdr. Grey** | Gate area → retreats | Zone 1 antagonist. Boss preview. |
| 4 Jackboot Grunts | Sections A-B | Enemies |
| 2 Jackboot Shields | Section B | Enemies |
| 1 Jackboot Crossbow | Guard booth | Enemy (ranged) |
| **Old Pike** | With player (if alive) | Allied NPC — can die here |
| **Kez** | With player (if recruited) | Companion fighter |
| **Rem** | With player (if saved) | Allied NPC fighter |

## Lighting

- **Section A:** Flickering. Deliberately unstable — the Jackboots control the power to this section. Creates disorienting light/shadow.
- **Section B:** Bright overhead work lights on the Jackboot side. Dark on the Tail side. The guards see the approach; the player is in shadow until crossing the barricade.
- **If electrical panel smashed:** Full blackout for 1-2 seconds, then emergency red lighting. Dramatic shift.
- **Section C:** Well-lit. The gate is illuminated. Grey is visible.
- **Section D:** Dim. The tension releases.

## Audio

- **Pre-combat:** Dead silence from Section A. The guards don't chat. Professional. Only boot steps and equipment sounds.
- **Combat:** Intense — clashing metal, shouts ("Hold the line!"), the hiss of steam, the twang of the crossbow, the electrical crackle if the panel is hit.
- **Grey's retreat:** His boots on metal, a hatch slamming. Then silence.
- **Post-combat:** Ringing ears effect. Fading combat sounds. The train rhythm returns. The player's breathing.
- **Pike's death (if it happens):** All sound drops for 2 seconds. Just heartbeat. Then slowly returns.

## Design Notes

- This is the first real test. The player should feel underprepared and desperate.
- The Jackboots are better equipped, better fed, and better trained. The Tailies win through numbers, desperation, and environmental creativity.
- The fight should take 3-5 minutes on first playthrough. It should feel hard-earned.
- Environmental kills should be satisfying — the player should feel clever for using the steam valve.
- Grey's retreat is mandatory (scripted). He cannot be killed here. This prevents the Car 15 boss from being skipped.

## Revisit Behavior

- Post-battle: the Blockade becomes a Tailie checkpoint. Friendly guards replace Jackboots.
- The gate remains open. Quick transit.
- Environmental elements (steam, electrical) remain interactive.
- Bodies are gone (Tailies cleared them). Bloodstains remain.
