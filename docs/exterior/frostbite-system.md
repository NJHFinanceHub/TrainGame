# Frostbite System

Frostbite is the progressive debuff system governing exterior survival. Exposure to
outside temperatures causes cumulative cold damage that advances through five stages,
ending in death. The system creates time pressure for all exterior activities and scales
with cold suit quality, weather conditions, and player stats.

## Core Mechanic

Every second spent on the exterior accumulates **cold exposure**. When cold exposure
exceeds a stage threshold, the player advances to the next frostbite stage. Each stage
applies increasingly severe debuffs. Stage 5 is lethal.

**Cold exposure** accumulates at a base rate of 1 point per second, modified by:

| Factor | Modifier |
|---|---|
| Ambient temperature (base) | 1.0× at -40°C or warmer |
| Per 10°C below -40°C | +0.25× (e.g., -60°C = 1.5×, -80°C = 2.0×) |
| Wind speed (per 20 km/h above 20) | +0.15× |
| Blizzard weather | +0.5× flat |
| Whiteout weather | +1.0× flat |
| Cold suit Mk.I | -0.3× |
| Cold suit Mk.II (battery active) | -0.6× |
| Cold suit Mk.III | -0.8× |
| Cold suit Mk.IV | -0.9× |
| Endurance stat (per point above 5) | -0.02× |
| Frostblood perk | -0.3× (stacks with suit) |
| Heated vent proximity (2m radius) | Accumulation paused, decays at 0.5×/sec |
| Warm Tunnels (Andean segment) | Accumulation paused, decays at 1.0×/sec |

**Minimum accumulation rate**: 0.1× (cold suits and perks cannot fully negate exposure;
biological limits exist regardless of equipment).

## Frostbite Stages

### Stage 0 — Normal

- **Threshold**: 0-59 cold exposure
- **Debuffs**: None
- **Visual**: None
- **Audio**: Ambient exterior sounds (wind, hull creaking)
- **HUD**: Frostbite meter empty (thin blue bar, bottom of screen)

### Stage 1 — Chill

- **Threshold**: 60 cold exposure
- **Debuffs**:
  - Movement speed: -5%
  - Interaction speed: -10% (hands stiffening)
  - Perception: -1 (effective stat, not permanent)
- **Visual**: Screen edges develop a faint frost vignette. Player character's breath
  becomes visible as white vapor (if not already visible from ambient cold). Hands
  tremble slightly during interactions.
- **Audio**: Breathing becomes audible—slow, controlled, but present. Suit fabric
  rustles as player moves.
- **HUD**: Frostbite meter fills to 20%. Bar turns light blue. Small snowflake icon
  appears.
- **Recovery**: Returns to Stage 0 after 30 seconds in a heated area or interior.

### Stage 2 — Biting Cold

- **Threshold**: 120 cold exposure
- **Debuffs**:
  - Movement speed: -15%
  - Interaction speed: -25% (fingers losing dexterity)
  - Attack speed: -15% (grip weakening)
  - Perception: -2
  - Cunning: -1 (cognitive function declining)
  - Lockpicking/hacking: Fail threshold increased by 2 (fumbling)
- **Visual**: Frost vignette intensifies, covering 20% of screen edges. Skin visible
  through suit gaps shows reddening. Hands shake during interactions (visible tremor).
  Breath vapor thicker, more frequent. Ice crystals form on visor edges (Mk.II+).
- **Audio**: Breathing faster, shallower. Occasional involuntary grunt of discomfort.
  Wind sounds louder (suit insulation less effective). If companion present: "We need
  to get inside."
- **HUD**: Frostbite meter at 40%. Bar turns medium blue. Snowflake icon pulses slowly.
  Temperature display text begins to flicker.
- **Recovery**: Returns to Stage 1 after 45 seconds in heated area. Returns to Stage 0
  after 90 seconds. Interior recovery: 60 seconds to Stage 0.

### Stage 3 — Hypothermia

- **Threshold**: 180 cold exposure
- **Debuffs**:
  - Movement speed: -30%
  - Interaction speed: -50% (hands barely functional)
  - Attack speed: -30%
  - Attack damage: -20% (muscle power declining)
  - Dodge: -25% (reaction time degraded)
  - Perception: -4
  - Cunning: -3
  - Charisma: -2 (speech slurring)
  - Sprint: Disabled (body conserving energy)
  - Complex interactions (lockpicking, hacking, calibration): Disabled
  - Cold suit battery drain: +50% (heating system overwhelmed)
- **Visual**: Frost vignette covers 40% of screen. Color saturation drops 50% (world
  appears grey-blue). Player character moves with visible stiffness—animations shift
  to a hunched, protective posture. Hands visibly blue-white. Breath comes in
  ragged bursts. Ice on visor significant (Mk.II+), partially obscuring view.
  Peripheral vision narrowing (subtle FOV reduction: -10°).
- **Audio**: Breathing labored, irregular. Heartbeat becomes audible—slow, heavy.
  Player character makes involuntary shivering sounds. Wind sounds distant (hearing
  affected). Companion (if present): urgent, pulling at player. "Come ON. NOW."
- **HUD**: Frostbite meter at 60%. Bar turns deep blue with white frost effect.
  Snowflake icon pulses rapidly. Warning text: "HYPOTHERMIA — SEEK SHELTER." HUD
  elements develop frost overlay (partially obscured).
- **Recovery**: Returns to Stage 2 after 60 seconds in heated area. Full recovery:
  3 minutes in interior. Upon recovery, a 60-second "Warming" debuff applies:
  -10% movement speed, hands shake (painful rewarming).
- **Gameplay note**: This is the "turn back" stage. Players who reach Stage 3 should
  feel that they've pushed too far. The debuffs make completing objectives extremely
  difficult. The game is telling you to retreat.

### Stage 4 — Severe Frostbite

- **Threshold**: 240 cold exposure
- **Debuffs**:
  - Movement speed: -50% (stumbling)
  - Interaction speed: -75%
  - Attack speed: -50%
  - Attack damage: -40%
  - Dodge: Disabled (cannot react fast enough)
  - All stats: -5 (effective)
  - Sprint: Disabled
  - Complex interactions: Disabled
  - Simple interactions: Require 2× time
  - Combat: Player can only use basic attacks (no combos, no special moves)
  - Cold suit: Functions at 50% effectiveness (ice penetrating seals)
  - **Permanent damage**: If the player reaches Stage 4, even after full recovery
    they retain a permanent -1 Endurance debuff ("Frostbite Scars"). This debuff
    is visible in the character screen and affects all future cold resistance
    calculations. It stacks (reaching Stage 4 twice = -2 END permanent). Maximum
    -3 END from frostbite scars.
- **Visual**: Frost vignette covers 60% of screen. Color nearly gone—world is
  blue-grey monochrome. Player character staggers, using walls/rails for support.
  Hands are white (tissue damage). Breath comes in shallow gasps. Visor nearly
  opaque with ice (Mk.II+). FOV reduced -20°. Intermittent screen blackouts (0.5
  seconds every 10 seconds—consciousness flickering).
- **Audio**: Heartbeat dominant sound—slow, irregular, skipping beats. Breathing
  barely audible (shallow). Wind sounds muffled as if underwater. Companion: silent
  (pulling player physically toward shelter—companion grab animation).
- **HUD**: Frostbite meter at 80%. Bar flashes white. Screen edges pulse red. Warning:
  "CRITICAL — TISSUE DAMAGE." HUD elements heavily frosted, barely readable.
- **Recovery**: Returns to Stage 3 after 90 seconds in heated area. Full recovery: 5
  minutes interior. "Warming" debuff lasts 2 minutes with -20% movement, shaking
  hands, screen pain flashes (rewarming tissue damage is real and agonizing). The
  player character makes pain sounds during rewarming.
- **Gameplay note**: Stage 4 is punishment. The permanent stat loss is the game
  saying: "You almost died, and your body will never fully recover." This is
  intentionally harsh to discourage treating exterior time limits as soft suggestions.

### Stage 5 — Lethal Exposure

- **Threshold**: 300 cold exposure
- **Effect**: Death.
- **Visual**: Screen whites out over 5 seconds. The player character collapses. The
  camera pulls back, showing the body on the hull/rooftop as the train continues
  forward, leaving the figure behind. The body shrinks to a dot. The train rounds
  a curve. The dot disappears.
- **Audio**: Heartbeat stops. Wind fades. Silence for 3 seconds. Then: the sound of
  the train moving—distant, mechanical, indifferent.
- **Reload**: Player respawns at last interior checkpoint (most recent interior save
  point or car transition). All exterior progress for the current objective is reset.
  Any permanent frostbite scars from Stage 4 during this run are retained.
- **Narrative**: Death by cold exposure is treated as a real consequence, not a
  game-over screen. The camera work emphasizes the train's indifference—it moves on.
  The world moves on. The cold doesn't care.

## Recovery Mechanics

### Heated Vents (Exterior)

Heated exhaust vents are located between specific cars, marked by visible steam plumes.
Standing within 2 meters of a heated vent pauses frostbite accumulation and begins
decay at 0.5× per second.

**Vent locations** (not exhaustive):
- Between Cars 13-14 (Tail/Third Class boundary)
- Between Cars 20-21 (Third Class, near maintenance hatch)
- Between Cars 35-36 (Third/Second Class boundary)
- Between Cars 47-48 (Working Spine, near engineering airlock)
- Between Cars 55-56 (Working Spine/First Class boundary)
- Between Cars 72-73 (First Class, near observation deck)
- Between Cars 90-91 (First Class/Sanctum boundary)

Vents are spaced approximately every 8-12 cars. The player can plan rooftop routes
around vent locations. The heated radius is small (2m)—the player must stand still
in the warm zone, visible and exposed. This creates a risk-reward decision during
stealth sections: warm up and risk detection, or push on and risk frostbite.

### Interior Recovery

Entering any interior space immediately stops frostbite accumulation. Decay rate in
interior spaces is 2× per second (faster than heated vents). Full recovery from Stage
3 takes approximately 3 minutes; from Stage 4, approximately 5 minutes.

During recovery from Stage 3+, the "Warming" debuff simulates the painful process of
rewarming frozen tissue:
- Stage 3 recovery: -10% movement, hand tremor, 60 seconds
- Stage 4 recovery: -20% movement, hand tremor, screen pain flashes, pain audio, 120 seconds

### Medical Treatment

- **Field Medic** skill: Reduces frostbite stage by 1 when applied (costs 1 Medical
  Kit). 60-second cooldown. Cannot be used on yourself during Stage 4+ (hands non-functional).
- **Dr. Volkov** (if alive and allied): Can treat frostbite scars. Removes 1 permanent
  END debuff per treatment. Requires specific medical supplies (Cryo-Gel, found in Cold
  Lab). One treatment available per zone. Maximum 2 treatments per playthrough.
- **Kronole**: A dose of Kronole temporarily negates frostbite pain and debuffs for 120
  seconds (all stages function as Stage 1). Does NOT stop accumulation—the frostbite
  continues advancing beneath the drug's numbing effect. When Kronole wears off, all
  accumulated stages hit at once. Extremely dangerous: players may reach Stage 5 without
  realizing it. The screen briefly shows the real frostbite stage when Kronole is active
  (a flicker of frost, ice, grey vision) as a warning.

## Stat Interactions

| Stat | Frostbite Interaction |
|---|---|
| Endurance | Each point above 5 reduces accumulation by 0.02× AND extends each stage threshold by 8 points. High END characters can stay outside significantly longer. |
| Survival | SRV 8+: Player receives an on-screen warning 15 seconds before each stage transition. SRV 12+: Warning at 30 seconds. SRV 14+: Frostbite HUD shows exact seconds remaining to next stage. |
| Perception | Affects ability to spot heated vents from distance (PER 10+: vent steam visible at 30m instead of 15m). |
| Cunning | CUN 12+: Cold suit upgrade efficiency +10% (modifications provide slightly more benefit). |

## Perk Interactions

| Perk | Effect |
|---|---|
| **Frostblood** | -0.3× accumulation rate. Stacks with cold suit. The single most powerful anti-frostbite perk. Described as a genetic quirk—some people are just resistant to cold. |
| **Iron Stomach** | No interaction. |
| **Cockroach** (Survival T3) | If frostbite reaches Stage 5, instead of dying: player drops to 1 HP, Stage 4, and gains 30 seconds of immunity. Once per exterior excursion. The "second chance" perk. |
| **Hardy** (Survival T1) | Stage thresholds increased by 10 points each (effectively +10 seconds per stage at base rate). |
| **Thermal Mastery** (Survival T2) | Heated vent effective radius doubled (4m instead of 2m). Recovery rate at vents doubled (1.0×/sec instead of 0.5×/sec). |
| **Breachman** (Quest-locked) | External hull access training. Movement on exterior surfaces: +10% speed. Magnetic boot speed: 70% instead of 60%. Hull handholds automatically highlighted. Does not directly affect frostbite but enables faster traversal, reducing total exposure time. |

## Design Intent

The frostbite system serves three purposes:

1. **Time pressure**: Every exterior objective operates under a ticking clock. The
   player must plan, execute efficiently, and know when to retreat. This transforms
   exterior traversal from "walk on the roof" into a resource-management challenge.

2. **Progressive stakes**: The five stages create escalating tension. Stage 1 is a
   gentle reminder. Stage 2 is a warning. Stage 3 is an alarm. Stage 4 is permanent
   consequences. Stage 5 is death. The player always knows where they are on the
   scale and can make informed decisions about risk.

3. **Thematic reinforcement**: The cold is the game's antagonist as much as Wilford is.
   The frostbite system makes the exterior world feel genuinely lethal. When the thaw
   progression eventually makes the outside survivable, the player has a visceral
   understanding of what "survivable" means—because they've experienced the alternative.
   The endgame walk to the Shelter is powerful precisely because the player has spent
   the entire game fearing the outside. Now they're choosing it.
