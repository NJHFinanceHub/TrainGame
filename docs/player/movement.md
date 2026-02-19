# Movement & Traversal

## Core Movement

### Walking
- **Default speed:** 3.5 m/s
- **Always available.** No stamina cost.
- **Footstep noise:** Moderate. Enemies in adjacent rooms can hear at PER 8+.

### Sprinting
- **Speed:** 6.0 m/s (1.7× walk speed)
- **Stamina cost:** 15/sec (base 100 stamina = ~6.5 seconds of sprint)
- **Noise:** High. Alerts enemies within 15m through walls.
- **Cannot sprint when:** Critically injured (Tier 4), Freezing cold level (61+), carrying heavy object, crouching
- **Sprint attack:** Sprinting into a melee attack triggers a tackle/charge (STR check, knocks down on success)

### Crouching
- **Speed:** 1.5 m/s (reduced to ~43% of walk)
- **Height:** Player profile reduced by 40% — can pass under obstacles, through low passages
- **Noise:** Low. Base detection radius reduced by 60%.
- **Can crouch-walk indefinitely.** No stamina cost.
- **Crouch + Stealth skills** stack: Soft Steps + Shadow Blend + crouch = near-silent in darkness

### Sliding
- **Triggered by:** Sprint → Crouch (hold)
- **Speed:** 7.0 m/s (decaying over 1.5 seconds)
- **Use:** Pass under low gaps, close distance to enemies, dodge ranged attacks
- **Slide attack:** Can swing melee weapon during slide for a low sweeping hit (trips enemies)
- **Cooldown:** 2 seconds between slides

## Vertical Movement

### Climbing
- **Climbable surfaces:** Pipes, ladder rungs, bunk stacks, cargo netting, cable runs, exposed structural beams
- **Visual indicator:** Climbable surfaces have subtle chalk marks (placed by Tailies) or visible handholds
- **Climb speed:** 1.0 m/s (vertical), reduced with arm injury
- **Stamina cost:** 8/sec while climbing. Falling when stamina runs out.
- **Fall damage:** Below 3m: none. 3-5m: Tier 1 injury. 5-8m: Tier 2-3 injury. 8m+: lethal.
- **Stat interaction:** STR 8 unlocks faster climbing (1.5 m/s). STR 12 unlocks climbing with one arm (hold object while climbing).

### Mantling
- **Triggered by:** Jump toward waist-height obstacles
- **Automatic** — no button press needed once you're close enough
- **Speed:** 0.8 seconds for standard mantle
- **Limits:** Cannot mantle objects above head height without STR 10 (pull-up mantle)

### Jumping
- **Horizontal distance:** 2.5m standing, 3.5m from sprint
- **Vertical reach:** 1.0m from standing jump
- **Not a platformer** — jumps are contextual and forgiving. Gaps that look jumpable are jumpable. No pixel-perfect timing.
- **Stamina cost:** 10 per jump

## Stealth Traversal

### Vents
- **Network:** Each car has 1-3 vent access points connecting to a vent network
- **Vent crawl speed:** 0.8 m/s (slow, cramped)
- **Benefits:**
  - Completely hidden from enemy detection
  - Bypass locked doors and barricades
  - Eavesdrop on rooms below (PER check for details)
  - Access otherwise unreachable areas (loot, shortcuts, story content)
- **Limitations:**
  - Cannot fight in vents (auto-exit if detected from below)
  - Some vents are blocked (need CUN 8 to remove grate, or STR 10 to force)
  - Vent maps are NOT shown on the HUD — must be discovered or found as intel items
  - Limited vents in later zones — First Class has sealed, monitored ventilation
- **Skill interaction:** Vent Crawler (Stealth tree) doubles vent movement speed. Engineer tree lets you reroute vent paths.

### Under-Car Traversal
- **Available at:** Connecting gaps between cars (exterior underbelly)
- **How it works:** Drop through floor access hatches → move along the train's undercarriage between cars
- **Speed:** 1.0 m/s (crawling on maintenance railings)
- **Hazards:**
  - **Cold exposure** — Full exterior cold rate. Must be quick.
  - **Train sway** — Periodic QTE-like moments: grab railing or risk falling off (instant death)
  - **Wind noise** — Cannot hear enemies/allies while under-car
- **Use cases:**
  - Skip a car entirely (bypass locked doors, ambushes, or hostile encounters)
  - Access car from unexpected entry point (flank enemies)
  - Reach hidden maintenance compartments with loot
- **Prerequisites:** Unlocked after finding the first floor hatch (Car 7 Workshop, shown by Forge)
- **Stat checks:** END 6 minimum (cold tolerance), periodic STR 6 grip checks in high-wind sections
- **Skill interaction:** Hardy (Survival) and Thermal Mastery reduce cold risk. STR investment eliminates grip checks.

### Rooftop Access
- **Available from:** Zone 2 onward (Tail cars have no roof access — too damaged)
- **How to reach rooftops:** Exterior ladders, hull breaches, specific car structures, under-car → ladder up
- **Rooftop movement:**
  - **Speed:** Normal walk speed on flat rooftops. Sprint available but risky.
  - **Terrain:** Varies per car — flat industrial roofs, curved passenger car tops, antenna forests, solar panel arrays
  - **Wind:** Strong headwind reduces forward speed by 20%. Gusts can stagger.
- **Hazards:**
  - **Cold exposure** — Full exterior cold rate
  - **Wind knockdown** — Periodic gusts require bracing (hold button) or be knocked prone
  - **Tunnel clearance** — Occasional tunnel sections require going prone or dropping back inside (instant death if standing in tunnel)
  - **Sniper exposure** — Enemies in watchtower cars can spot rooftop movement
- **Use cases:**
  - Skip 1-3 cars at a time (huge shortcut potential)
  - Access cars from above (drop through skylights, roof hatches)
  - Sniper positions for ranged combat
  - Reach otherwise impossible areas
  - Dramatic set-piece sequences (Zone 4-5 major quest involves rooftop traverse)
- **Prerequisites:** First accessed via scripted sequence in Zone 2. Free access afterward.
- **Stat requirements:** END 8 for extended rooftop exposure. PER 8 to spot tunnel warnings early.

## Contextual Movement

### Ladders
- Dedicated climb animation. Up/down at 1.5 m/s. Can slide down at 3.0 m/s (takes minor damage).

### Balancing
- Narrow beams, pipes, railings. Walk speed only (1.5 m/s). Falling off causes Tier 1-2 injury depending on height.

### Swimming
- **Does not exist.** No water areas deep enough to swim. Flooding sections are waded through at 1.0 m/s.

### Carrying Objects
- Some puzzles require carrying items (power cells, explosives, bodies).
- Carry speed: 2.0 m/s (walk only, no sprint). Cannot climb or crouch while carrying.
- Can throw carried objects (3-5m range based on STR).

### Door Interactions
- **Standard doors:** 3-second wheel-lock animation (masks level streaming)
- **Quick doors:** 1-second push/pull (within already-loaded areas)
- **Barricaded doors:** Environmental puzzle — clear debris, find alternate route, or force (STR check)
- **Locked doors:** Lockpick minigame (CUN-based), find key, hack panel (CUN-based), or force (STR 14+ only, loud)

## Movement Feel

### Train Motion
The player is always on a moving train. This manifests as:
- **Constant subtle sway** — 0.5-degree oscillation on a 4-second cycle
- **Periodic jolts** — Every 30-60 seconds, a track bump causes a brief stumble (cosmetic, no gameplay effect normally; during combat/climbing, can be a factor)
- **Curve tilt** — When the train rounds a bend, everything tilts 2-3 degrees for 10-20 seconds
- **Sound:** Persistent thunk-thunk of wheels on track joints. Grows quieter in insulated cars, louder near hull breaches.

### Stamina Management
Stamina governs sprint, climb, jump, and combat actions. Balancing exploration stamina vs. combat stamina is a core survival consideration.

| Action | Stamina Cost |
|---|---|
| Sprint | 15/sec |
| Climb | 8/sec |
| Jump | 10 per jump |
| Melee swing (light) | 8 per swing |
| Melee swing (heavy) | 20 per swing |
| Block (sustain) | 5/sec |
| Dodge/roll | 15 per dodge |
| Slide | 10 per slide |

**Recovery:** 12/sec base (out of combat), 8/sec (in combat). Modified by END stat and stamina perks.

**Empty stamina:** Cannot perform stamina actions. 1-second recovery delay before regen starts. Character visibly gasps/hunches.
