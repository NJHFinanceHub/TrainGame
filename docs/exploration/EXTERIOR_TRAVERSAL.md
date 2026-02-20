# Exterior Traversal Systems

> *"Up top, the wind hits you like a wall of knives. Below, the wheels scream inches from your skull. Between those two hells — that's where the real paths are."*

## Overview

Exterior traversal encompasses three distinct movement systems that exist outside the train's interior: **Rooftop Traversal**, **Under-Car Crawlspace**, and **Window/Exterior Viewing**. These systems provide alternate routes, exclusive content, and environmental storytelling that contextualizes the train within the frozen world.

---

## I. Rooftop Traversal

### Access Points

Rooftop access is available via:

| Method | Frequency | Requirements |
|--------|-----------|--------------|
| Roof hatches | 1 per 3-4 cars | STR 7 to force, or find release latch |
| External ladders | 1 per 5-6 cars | Must be on exterior already (between-car gap) |
| Between-car climb | Every coupling | AGI check (DC 8), stamina cost 20 |
| Emergency exits | Specific cars | Story-triggered or blueprint knowledge |
| Vent-to-roof paths | Rare (4 total) | Vent navigation + PER 8 to spot exit grate |

### Environmental Hazards

#### Wind Resistance

The train moves at 70-120 km/h depending on terrain. Wind is a constant physics force pushing the player toward the trailing edge of the car.

| Train Speed | Wind Force | Movement Penalty | Standing Risk |
|-------------|-----------|------------------|---------------|
| 70 km/h (uphill) | Moderate | -30% move speed | Stable |
| 90 km/h (flat) | Strong | -50% move speed | Stagger every 15s (AGI save) |
| 120 km/h (downhill) | Severe | -70% move speed | Stagger every 8s, prone risk |
| Tunnel entry | Extreme burst | Knockdown (no save) | Lethal if standing |

**Crouch movement** reduces wind penalties by 50% but slows base speed to 1.5 m/s.
**Prone crawl** eliminates wind knockdown but limits speed to 0.8 m/s and prevents combat.

Wind direction shifts at curves — the train's route includes sweeping bends where wind suddenly comes from the side instead of head-on, catching players off-guard.

#### Cold Exposure

Rooftop exposure activates the **Cold Exposure Timer**:

| Stage | Time | Effect |
|-------|------|--------|
| Chill | 0-60s | Screen frost edges, slight blue tint |
| Biting Cold | 60-120s | -10% max stamina, visible breath, hands shake (aim penalty) |
| Hypothermia | 120-180s | -25% max stamina, -15% move speed, health drain 2/sec |
| Frostbite | 180-240s | -50% max stamina, -30% move speed, health drain 5/sec, blur |
| Lethal | 240s+ | Health drain 15/sec, screen whiteout, unconsciousness in 20s |

**Mitigation:**
- Warm clothing: +60s to each stage transition
- Cold Suit: +120s to each stage, wind resistance +50%
- Cold Suit Mk.II (Blueprint, Zone 4): +180s, wind immunity below 100 km/h
- Cold Suit Mk.III (Blueprint, Prototype Car): +300s, wind immunity, allows extended EVA
- Heated roof sections (near exhaust vents): Reset timer while standing on them, marked by visible steam
- Re-entering the train interior: Timer resets over 30s

**Gameplay Intent:** Rooftop is a high-risk shortcut, not a leisurely path. Players should feel the pressure of cold and wind, making every rooftop segment a calculated gamble. The Cold Suit progression rewards exploration — players who find the blueprints gain dramatically more rooftop access.

#### Tunnel Clearance

The train passes through tunnels at irregular intervals. Tunnels are telegraphed by:
1. Audio cue: Distant rumbling, 10s warning
2. Visual cue: Tunnel mouth visible ahead, 5s warning
3. Companion warning: Active companion shouts "GET DOWN!" 3s warning

**Tunnel behavior:**
- **Low clearance** (most tunnels): Must be prone. Standing or crouching = instant death
- **Medium clearance** (rare): Crouching safe, standing = heavy damage + knockdown
- **Open tunnels** (bridges, overpasses): Safe but wind intensifies

**Player death in tunnels** is gruesome and immediate — a hard consequence that trains respect for the environment. Checkpoint placed before every tunnel section.

### Rooftop Navigation

#### Car Roof Types

| Car Type | Roof Surface | Features |
|----------|-------------|----------|
| Passenger (standard) | Curved metal, slippery | Antenna arrays, vent stacks (partial cover) |
| Freight/Industrial | Flat metal, textured grip | Cargo tie-downs (handholds), access panels |
| Armored (Jackboot) | Flat reinforced, spotlight mounts | Guard posts (occupied), searchlight hazards |
| Luxury (First Class) | Glass sections + metal frame | Glass is fragile — can break through (fall inside) |
| Engine cars | Heavily textured, hot spots | Exhaust vents (heat timer reset), pipe obstacles |

#### Rooftop Traversal Abilities

| Action | Requirement | Speed | Risk |
|--------|------------|-------|------|
| Walk upright | Wind < Strong | 2.5 m/s | Stagger risk |
| Crouch walk | Any wind | 1.5 m/s | Low |
| Prone crawl | Any conditions | 0.8 m/s | Minimal |
| Sprint | Wind Moderate only | 5.0 m/s | High stagger, high stamina |
| Jump (car gap) | 2m gap, AGI 8 | One-time | Fall = severe damage/death |
| Climb antenna | STR 6, stamina | 1.0 m/s vertical | Fall = moderate damage |
| Slide down curve | Curved roofs | 4.0 m/s | Must arrest slide (AGI check) |

#### Rooftop Combat

Combat on rooftops follows modified rules:

- **No heavy weapons**: Two-handed weapons cannot be used (wind + balance)
- **Melee range reduced**: Effective range -0.5m (footing instability)
- **Knockback enhanced**: All knockback effects push 2x distance (wind assist or resist depending on direction)
- **Fall risk**: Any stagger effect near a roof edge triggers a grab check (AGI DC 10) or fall
- **Ranged advantage**: Ranged weapons gain +15% accuracy (open sightlines, no corridor obstructions)
- **Wind-assisted attacks**: Attacks in the wind's direction deal +20% knockback. Against the wind: -20%

**Enemy types on rooftops:**
- **Breachmen scouts** (neutral/hostile depending on faction standing): Patrol rooftops, know the terrain. Fast, lightly armed
- **Jackboot snipers** (hostile): Positioned at guard posts on armored cars. Long-range threat, poor melee
- **Wind Stalkers** (rare encounter): Individuals who live on the rooftop permanently, wrapped in insulation, half-mad. Unpredictable — may trade, may attack, may simply watch

### Rooftop Exclusive Content

| Item | Type | Location | Access |
|------|------|----------|--------|
| Breachman's Logbook | Faction Intel | Car 28 antenna post | Climb antenna, AGI 7 |
| Frozen Climber's Pack | Artifact | Car 44 exhaust vent area | Near heated section |
| Sniper's Scope Blueprint | Blueprint | Car 70 guard post | Defeat/stealth past Jackboot sniper |
| The View South | Audio Recording | Car 85 roof peak | Cold Suit required, specific time of day |
| Wind Map | Faction Intel | Car 52 Breachmen cache | Breachmen faction FRIENDLY+ |
| Rooftop Garden | Artifact (set of 3) | Car 33 heat vent cluster | PER 10+, hidden behind vents |

---

## II. Under-Car Crawlspace System

### Overview

The undercarriage of Snowpiercer is a network of maintenance crawlspaces, cable runs, and mechanical systems. Movement is restricted to prone crawling on metal grating with the frozen ground rushing past below. The system provides stealth bypasses, connects otherwise inaccessible areas, and contains hidden content.

### Access Points

| Method | Frequency | Requirements |
|--------|-----------|--------------|
| Floor hatches (interior) | 1 per 2-3 cars | CUN 5 to open, or find release |
| Between-car drop-down | Every coupling | AGI 6, takes fall damage (minor) |
| Maintenance pits (rare) | 4 total across train | Story access or Breachmen codes |
| Derailed section | Zone 4 specific | Navigate damaged undercarriage |

### Environmental Hazards

#### Mechanical Dangers

| Hazard | Frequency | Effect | Avoidance |
|--------|-----------|--------|-----------|
| Moving drive shafts | Continuous in engine cars | Instant death on contact | Timing-based crawl (3s windows) |
| Steam vents | Periodic (every 8-12s) | 30% health damage, stagger | Listen for hiss (1.5s warning), PER 7 |
| Electrical conduits | Static placement | 20% health if touched, stun 2s | Visual spark indicator, stay center |
| Loose grating | Random placement | Falls through — grab check AGI 8 | Weight indicators (grating sags visually) |
| Ice formation | Exterior-adjacent sections | Slippery surface, slide toward edge | Slow movement, AGI check at speed |

#### Noise Considerations

The crawlspace is directly beneath occupied cars. Sound travels upward:

| Action | Noise Radius (upward) | Detection Risk |
|--------|----------------------|----------------|
| Slow crawl | 2m | Minimal — masked by train noise |
| Normal crawl | 4m | Low — may trigger SUSPICIOUS in quiet cars |
| Fast crawl | 8m | Moderate — will trigger investigation |
| Equipment drop | 12m | High — loud clang on metal grating |
| Combat | 15m+ | Certain detection — NPCs hear and respond |

**Train motion masking:** Background train noise provides -3m to all noise radii. In louder car sections (engine room, kitchen), noise is further masked (-5m).

### Crawlspace Navigation

#### Movement in Crawlspace

| Action | Speed | Stamina Cost | Noise |
|--------|-------|-------------|-------|
| Slow crawl | 0.5 m/s | 2/sec | Very low |
| Normal crawl | 1.0 m/s | 5/sec | Low |
| Fast crawl | 1.8 m/s | 10/sec | Moderate |
| Squeeze through gap | 0.3 m/s | 8/sec | Very low |
| Drag object | 0.4 m/s | 12/sec | High |

#### Crawlspace Features

- **Observation grates**: Look up into cars above. See NPC positions, listen to conversations (eavesdropping mechanic). Cannot interact, but can plan approach
- **Cable junction boxes**: Hackable (CUN 8) to disable specific car systems — lights, locks, intercoms. Creates tactical advantages before entering from above
- **Maintenance caches**: Small supply stashes left by Breachmen. Contain materials, occasionally rare components
- **Graffiti markers**: Scratched symbols left by previous crawlers. Guide toward hidden paths and warn of hazards. Decipherable with PER 7+
- **Rat runs**: Literal — rats use the crawlspace too. Following rat paths sometimes reveals otherwise hidden passages

#### Crawlspace Map

The crawlspace is not contiguous — sections are blocked by:
- **Bulkhead seals** between zones (require Breachmen codes or CUN 12 hack)
- **Damaged sections** where grating has collapsed (jump gap — AGI 9 or turn back)
- **Flooded sections** where coolant or condensation pools (wade through — cold exposure, reduced speed)
- **Occupied maintenance bays** where workers are actively servicing (wait for shift change or find alternate route)

### Crawlspace Exclusive Content

| Item | Type | Location | Access |
|------|------|----------|--------|
| Breachman's Toolkit | Blueprint | Under Car 49 | Maintenance pit access |
| The Underneath People | Faction Intel | Under Car 23 | Follow graffiti markers |
| Frozen Rat King | Artifact | Under Car 61 (zone border) | Navigate flooded section |
| Saboteur's Guide | Faction Intel | Under Car 78 | CUN 10 to find hidden cache |
| Maintenance Manifest | Train Log | Under Car 55 | Near Outside Lab mount point |
| Escape Route Map | Faction Intel | Under Car 15 | Crawlspace from Zone 1 exit |

---

## III. Window & Exterior Viewing System

### Overview

The exterior of the train — the frozen world rushing past — is a persistent environmental storytelling layer. Through windows, observation points, and exterior traversal, players witness the thaw progression and the state of the world beyond the train.

### Window Types

| Window Type | Location | View Quality | Interaction |
|-------------|----------|-------------|-------------|
| Passenger windows | Most cars | Standard — frost-covered, partial view | Wipe frost (temporary clear spot, 30s) |
| Porthole windows | Industrial cars | Limited — small, often steamed | No wiping, fixed view |
| Observation windows | Specific cars (5 total) | Excellent — large, climate-controlled | Telescope interaction available |
| Sealed windows | Jackboot/secured cars | None — permanently shuttered | Cannot open (story-relevant) |
| Broken windows | Damaged cars (rare) | Perfect but dangerous — cold exposure | Creates micro-exterior zone |
| First Class panoramic | Luxury cars | Premium — heated glass, no frost | Interactive zoom, scenic framing |

### Thaw Progression System

The exterior world changes visually as the player progresses through the game. This is THE primary indicator that the world is recovering — visible through windows and during rooftop traversal.

#### Progression Stages

| Game Progress | Season | Exterior Description |
|--------------|--------|---------------------|
| Zone 1-2 (early) | Deep Winter | Solid white. No features visible beyond ice formations. Blizzard conditions. Wind screams. Temperature: -67°C |
| Zone 3 (mid-early) | Winter | Ice formations more structured. Occasional glimpse of buried structures — rooftops, highway signs. Temperature: -58°C |
| Zone 4 (mid) | Late Winter | Color differentiation — dark rock visible under thin ice on mountainsides. Frozen rivers show cracks. Temperature: -45°C |
| Zone 5 (mid-late) | Early Thaw | Brown patches on south-facing slopes. Water visible at geothermal sites. Animal tracks in snow (first life). Temperature: -31°C |
| Zone 6 (late) | Thaw | Green patches. Trees with buds visible on lower elevations. Standing water. Bird silhouettes. Temperature: -18°C |
| Zone 7 (endgame) | Spring | Grass visible. Flowing water. Animals visible (deer, birds). Clouds that aren't just ice. Temperature: -7°C |

**Route-Dependent Variation:** The thaw is not uniform. Equatorial and coastal route segments show more recovery than northern/interior segments. The train's circumnavigation means players see both recovered and still-frozen regions in the same zone.

**Player Discovery:** The thaw progression is NEVER explicitly stated by the game's UI or narrative text until the Outside Lab is discovered. Before that, it's purely environmental — a player paying attention to windows will notice the change. Companions may comment: "Is that... brown?" or "Did you see that? On the mountainside?" These comments are easy to miss if you're not looking.

#### Key Exterior Landmarks

Specific visible landmarks appear at windows throughout the train, creating a sense of geographic reality:

| Landmark | Route Segment | Visible From | Significance |
|----------|--------------|-------------|--------------|
| The Frozen City | European segment | Cars 10-15, 40-45 | A city buried in ice. Skyscraper tips visible. Lights? (PER 12 — no, it's ice refraction) |
| The Bridge | Asian segment | Cars 30-35, 70-75 | A massive bridge the train crosses. View straight down into a frozen gorge |
| The Geothermal Vent | Icelandic segment | Cars 50-55 | Steam rising from the ground. First evidence of heat. Visible from Zone 4+ |
| The Graveyard | Unknown segment | Cars 60-65 | Another train, derailed and frozen. Big Alice? A prototype? |
| The Green Valley | Southern segment | Cars 80-85 | Only visible late-game (Zone 5+). A valley with visible vegetation. The proof |
| The Shelter | Endgame only | Car 95+ | If the player has shelter coordinates, a structure becomes visible on a mountainside |

### Observation Point Interactions

Five dedicated observation points throughout the train offer enhanced exterior viewing:

| Location | Car | Equipment | Unique Feature |
|----------|-----|-----------|----------------|
| Tail Observatory | Car 5 | Scratched window + binoculars | See the track behind the train — where you've been |
| Worker's Lookout | Car 28 | Porthole + hand telescope | Mounted at the highest point in Third Class |
| The Gallery | Car 45 | Floor-to-ceiling windows | Heated, comfortable — a rare Zone 4 luxury for engineers |
| First Class Solarium | Car 72 | Panoramic + powered telescope | Motorized tracking, can follow landmarks |
| Sanctum Viewport | Car 90 | Armored glass + digital overlay | Wilford's personal window. Digital readouts show real-time atmospheric data (if activated) |

**Telescope Mechanic:**
- Free-look mode with zoom (2x-8x depending on equipment)
- During telescope use, time advances at 4x speed (watching the world go by)
- PER checks during telescope use may reveal additional details:
  - PER 6: General landscape features
  - PER 8: Structural details (buildings, vehicles)
  - PER 10: Signs of life (animal tracks, smoke, movement)
  - PER 12: Fine details (text on signs, window details on buildings, bird species)

### Exterior Set Pieces

Scripted exterior moments that occur at specific narrative points:

1. **The First Window** (Zone 1): Player's first clear look outside. Josie wipes frost, they look together. Nothing but white. "This is what we're fighting through."

2. **The Derailed Train** (Zone 3): A sidelined train visible from the rooftop. If the player has binoculars: frozen faces visible in the windows. A different train that didn't make it.

3. **The Warm Spot** (Zone 4): During rooftop traversal near a geothermal area, ice on the train's surface is visibly melting. Water runs down the hull. For a moment, the cold exposure timer pauses. Something is different here.

4. **The Bird** (Zone 5): A bird lands on the rooftop near the player. First living thing seen outside the train. Companions react with wonder/disbelief. If attacked: companion loyalty penalty. If watched: it flies south. There's a south to fly to.

5. **The Green** (Zone 6): From the Sanctum viewport, during a specific route segment, an entire valley is visible with green vegetation. The digital overlay (if activated) shows: Temperature -4°C. Humidity 62%. UV Index: 3. "Habitable" flashes once, then the display is remotely shut off.

6. **The Decision View** (Zone 7): Before the Dead Man's Switch, a long exterior sequence. The player walks the full length of the Engine's roof. In every direction: a world that is waking up. Not healed. Not safe. But alive. The lever is below.

---

## IV. Maintenance Tunnels

### Overview

Connecting the crawlspace, rooftop, and interior systems are the maintenance tunnels — dedicated service corridors built into the train's structure. These are the Breachmen's domain.

### Tunnel Types

| Type | Location | Dimensions | Purpose |
|------|----------|-----------|---------|
| Cable runs | Between floors | 0.8m x 0.8m | Carry electrical/data cables. Prone only |
| Pipe galleries | Along car walls | 1.2m x 1.5m | Water, steam, coolant pipes. Crouch walk |
| Service corridors | Between doubled walls | 1.0m x 2.0m | Access to systems. Walk (single file) |
| Ventilation shafts | Floor-to-ceiling | 0.6m x 0.6m | Air circulation. Prone, tight squeeze |
| Emergency passages | Between zones | 1.5m x 2.0m | Sealed evacuation routes. Walk normally |

### Breachmen Access

Maintenance tunnels are the Breachmen faction's territory. Relationship with them affects access:

| Faction Standing | Access Level |
|-----------------|-------------|
| HOSTILE | Tunnels booby-trapped. Breachmen attack on sight |
| COLD | Basic cable runs accessible. Breachmen ignore you |
| NEUTRAL | Pipe galleries open. Breachmen may share warnings |
| FRIENDLY | Service corridors accessible. Breachmen offer guidance |
| ALLIED | Emergency passages unlocked. Full tunnel network available. Breachmen fight alongside you |

### Tunnel Network Map

The maintenance tunnel network forms a secondary map layer:

```
Zone 1 ←──cable run──→ Zone 2 ←──pipe gallery──→ Zone 3
  ↕                      ↕                         ↕
crawlspace             crawlspace                crawlspace
  ↕                      ↕                         ↕
Zone 3 ←──service corridor──→ Zone 4 ←──emergency──→ Zone 5
  ↕                             ↕                      ↕
rooftop                      rooftop                rooftop
  ↕                             ↕                      ↕
Zone 5 ←──pipe gallery──→ Zone 6 ←──cable run──→ Zone 7
```

**Key Design Principle:** The maintenance network makes the train feel three-dimensional. Players aren't just moving car-to-car in a line — they're moving through layers: roof, interior, sub-floor, and between walls. The train is a living machine with arteries and veins the passengers never see.

### Tunnel Exclusive Content

| Item | Type | Location | Requirements |
|------|------|----------|-------------|
| Breachmen's Code Book | Faction Intel | Service corridor, Zone 4 | Breachmen FRIENDLY |
| Pipe Pressure Map | Blueprint | Pipe gallery, Zone 2-3 | CUN 8 to read gauges |
| The Whispering Wall | Audio Recording | Cable run, Zone 5-6 | PER 9 to hear, investigate |
| Emergency Manifest | Manifest Page | Emergency passage, Zone 4-5 | Sealed door, STR 10 or code |
| Builder's Signature | Artifact | Ventilation shaft, Zone 1 | Tight squeeze, END 8 |
| Tunnel Rat's Diary | Train Log | Multiple locations (3 parts) | Each in different tunnel type |
