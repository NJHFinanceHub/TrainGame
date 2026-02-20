# Lower Transport Deck — Design Document

## Overview

A secondary sub-level running beneath Cars 15–82 (Third Class through First Class).
Industrial cargo transport corridor with mini-rail system, used for legitimate supply
distribution and illicit smuggling operations.

## Physical Layout

### Dimensions (per car segment, at 10x scale)
- **Length**: 100m (matches upper car, 10000 UE units)
- **Width**: 30m (narrower than upper deck — hull curvature)
- **Height**: 4m ceiling (cramped, industrial)
- **Vertical offset**: -500 UE units below upper deck floor

### Zone Characteristics

| Zone | Cars | Deck Condition | Security Level | Cargo Traffic |
|------|------|----------------|----------------|---------------|
| Third Class | 16–35 | Deteriorated, jury-rigged | Low | Food, water, basic medical |
| Second Class | 36–55 | Functional, maintained | Medium | Medical, textiles, chemicals |
| Working Spine | 56–70 | Heavy industrial | High (automated) | Engine parts, chemicals, water |
| First Class | 71–82 | Clean, well-lit sections | Very High | Luxury goods, premium food |

### Environmental Design

**Third Class Sub-deck (Cars 16–35)**
- Exposed pipes, dripping condensation, flickering fluorescent strips
- Rat nests in unused alcoves — noise hazard
- Kronole Network stash points behind false panels
- Graffiti from Tailies who've passed through
- Cold breaches near hull seams — ice on metal

**Second Class Sub-deck (Cars 36–55)**
- Better maintained, functional lighting
- Organized storage bays with manifest boards
- Bureaucracy inspection checkpoints (locked hatches)
- Chemical storage with hazard warnings

**Working Spine Sub-deck (Cars 56–70)**
- Heavy machinery vibration, constant noise (good for stealth)
- Drive shaft access — timing hazards
- Automated security drones patrol fixed routes
- Steam vents and electrical arcs

**First Class Sub-deck (Cars 71–82)**
- Pristine compared to lower zones, some sections heated
- Heavy security — cameras, tripwires, pressure plates
- Premium cargo: wine cellars, fabric stores, art crates
- Jackboot patrol presence (NPC guards, not just automated)

## Mini-Rail System

### Track Layout
- **Main line**: Single rail running full length (Car 15 → Car 82)
- **Sidings**: Loading bays every 3–5 cars
- **Junctions**: Player-switchable at zone boundaries and major intersections
- **Turntables**: At zone boundary terminals (Cars 35, 55, 70)

### Cart Specifications
- **Speed**: 0–5 m/s (walking speed ~1.4 m/s, so 3.5x faster)
- **Capacity**: 200 weight units
- **Noise**: Proportional to speed (2m idle → 12m at max)
- **Controls**: Start, brake, emergency stop
- **Player can**: Ride, steer at junctions, load/unload cargo

### Cart Types
1. **Open Flatbed** — Standard cargo, player visible
2. **Covered Hopper** — Enclosed, player hidden but blind
3. **Maintenance Cart** — Lighter, faster, no cargo capacity
4. **Contraband Cart** — False-bottom, hidden compartment for smuggling

## Access Points

### Distribution (one or more per car)
| Type | Frequency | Transition Time | Noise | Lock Status |
|------|-----------|-----------------|-------|-------------|
| Floor Hatch | Every 2–3 cars | 2.0s | Medium | Often locked |
| Freight Elevator | Zone boundaries | 3.0s | High | Key required |
| Maintenance Stairs | Every 5 cars | 1.5s | Low | Usually open |
| Emergency Ladder | Every car | 1.0s | Low | Never locked |
| Vent Shaft | Rare, hidden | 2.5s | Very low | CUN check |

### Key Access Points
- **Car 16**: Maintenance stairs (first deck access from Third Class)
- **Car 25**: Floor hatch near Kronole Network den
- **Car 35**: Freight elevator (Third/Second Class boundary)
- **Car 45**: Hidden vent shaft (stealth route to Bureaucracy offices above)
- **Car 55**: Freight elevator (Second Class/Working Spine boundary)
- **Car 60**: Emergency ladder (near drive shaft — dangerous)
- **Car 70**: Freight elevator (Working Spine/First Class boundary)
- **Car 75**: Heavily guarded freight elevator (First Class)
- **Car 80**: Secret vent shaft (leads to First Class private suites)

## Security Systems

### Automated Security
- **Motion Sensors**: Sweep area, trigger on movement speed > crouch
- **Laser Tripwires**: Static beams across corridors, instant alarm
- **Patrol Drones**: Move along track, have vision cones
- **Pressure Plates**: Floor-mounted, weight-activated
- **Security Cameras**: Fixed rotation, records for Jackboots

### Countermeasures (Player Options)
- **CUN Check**: Hack/disable devices (DC 8 base, +2/+4/+8 at higher alerts)
- **Timing**: Move between sensor sweeps
- **Distraction**: Throw objects to trigger false alarms
- **EMP**: Craftable item, jams devices for 15 seconds
- **Cart Riding**: Some sensors ignore tracked carts (legitimate traffic)

### Alert Escalation
1. **Clear** → Normal operations, carts running
2. **Caution** → Sensors more sensitive (+2 DC), carts slow down
3. **Alert** → Drones deployed, access points monitored
4. **Lockdown** → All carts stop, all access points sealed, Jackboots dispatched

## Gameplay: Stealth Route

The lower deck is the primary **checkpoint bypass** for players preferring stealth:

1. Player discovers a floor hatch or maintenance stairs in current car
2. Descends to lower deck — cramped, dark, industrial
3. Navigates between cars without passing through guarded upper-deck doors
4. Avoids automated security (sensors, tripwires, drones)
5. Boards a mini-rail cart for faster traversal (but louder)
6. Exits via access point past the checkpoint
7. Returns to upper deck behind enemy lines

### Stealth Risk/Reward
- **Low risk**: Walking slowly in Third Class sub-deck (few sensors, machinery noise masking)
- **Medium risk**: Riding cart through Second Class (moderate security, some cameras)
- **High risk**: Infiltrating First Class sub-deck on foot (heavy security, patrols)
- **Very high risk**: Cart through First Class during alert (carts searched at checkpoints)

## Gameplay: Smuggling

The Kronole Network runs contraband through the lower deck:

### Smuggling Runs
1. **Accept contract** from Kronole Network contact above
2. **Pick up cargo** at a stash point on the lower deck
3. **Load onto cart** (or carry manually — slower, quieter)
4. **Navigate** through zones, switching tracks at junctions
5. **Avoid inspection** — Jackboot checkpoints at zone boundaries
6. **Deliver cargo** to destination stash point
7. **Collect payment** — Kronole, scrap, faction reputation

### Contraband Types
- **Kronole**: High value, Kronole Network exclusive
- **Weapons**: Rebellion supply caches, hidden in tool crates
- **Intel**: Encrypted documents, guard rotations, gate codes
- **Luxury goods**: Stolen from First Class, sold to black market

### Consequences
- **Caught by Jackboots**: Confiscation, reputation loss, possible combat
- **Caught by Kronole Network rivals**: Ambush encounter
- **Successful delivery**: Faction reputation, currency, quest progression

## Integration Points

### With Existing Systems
- **Car Streaming Subsystem**: Lower deck sublevels stream in 3-car window alongside upper deck
- **Stealth/Detection System**: Lower deck uses same alert state propagation
- **Inventory System**: Cargo looting adds items via standard AddItem()
- **Faction Manager**: Stealing cargo affects faction standing
- **Economy/Barter**: Cargo trade values integrate with merchant system
- **Crawlspace Component**: Under-car crawlspaces connect to deck access points
- **Quest Manager**: Smuggling runs are quest objectives

### Sublevel Naming Convention
- Upper deck: `L_Car_[CarIndex]`
- Lower deck: `L_Deck_[CarIndex]`
- Both stream together when car is in the active window
